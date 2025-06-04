#include "spacetimedb/internal/Module.h"
#include "spacetimedb/internal/autogen/RawModuleDef.g.h"
#include "spacetimedb/sdk/reducer_context.h"
#include <iostream>
#include <cstring>

namespace SpacetimeDb {
namespace Internal {

// Default implementation of IReducerContext
class DefaultReducerContext : public IReducerContext {
private:
    sdk::Identity sender_;
    std::optional<sdk::ConnectionId> connectionId_;
    sdk::Timestamp timestamp_;
    uint64_t seed_;
    
public:
    DefaultReducerContext(sdk::Identity sender, 
                         std::optional<sdk::ConnectionId> connectionId,
                         uint64_t seed,
                         sdk::Timestamp timestamp)
        : sender_(sender), connectionId_(connectionId), 
          timestamp_(timestamp), seed_(seed) {}
    
    sdk::Identity GetSender() const override { return sender_; }
    std::optional<sdk::ConnectionId> GetConnectionId() const override { return connectionId_; }
    sdk::Timestamp GetTimestamp() const override { return timestamp_; }
    
    sdk::ReducerContext ToSdkContext() override {
        // Create SDK context - implementation depends on sdk::ReducerContext structure
        sdk::ReducerContext ctx;
        // TODO: Initialize ctx with sender, connectionId, timestamp
        return ctx;
    }
};

// FFI constants
const FFI::RowIter FFI::RowIter::INVALID = {0xFFFFFFFF};
const FFI::BytesSource FFI::BytesSource::INVALID = {0xFFFFFFFF};

// Module implementation
Module& Module::Instance() {
    static Module instance;
    return instance;
}

Module::Module() : typeRegistrar(std::make_unique<TypeRegistrar>(*this)) {
    // Initialize empty module definition
    moduleDef = RawModuleDefV9{};
    
    // Set default context constructor
    newContext = [](sdk::Identity sender, std::optional<sdk::ConnectionId> connectionId, 
                    uint64_t seed, sdk::Timestamp timestamp) {
        return std::make_unique<DefaultReducerContext>(sender, connectionId, seed, timestamp);
    };
}

void Module::SetReducerContextConstructor(
    std::function<std::unique_ptr<IReducerContext>(
        sdk::Identity, std::optional<sdk::ConnectionId>, uint64_t, sdk::Timestamp
    )> ctor
) {
    Instance().newContext = std::move(ctor);
}

void Module::RegisterReducerImpl(std::unique_ptr<IReducer> reducer) {
    // Get reducer definition
    auto reducerDef = reducer->MakeReducerDef(*typeRegistrar);
    
    // Add to module definition
    moduleDef.reducers.push_back(std::move(reducerDef));
    
    // Store the reducer instance
    reducers.push_back(std::move(reducer));
}

void Module::RegisterTableImpl(const RawTableDefV9& table) {
    moduleDef.tables.push_back(table);
}

void Module::RegisterReducerDirectImpl(const std::string& name, ReducerFn fn) {
    // Create reducer definition
    RawReducerDefV9 reducerDef;
    reducerDef.name = name;
    
    // Generate function type
    auto funcTypeRef = typeRegistrar->RegisterType<void()>(
        [](AlgebraicTypeRef) -> std::vector<uint8_t> {
            // Simple function type with no args for now
            bsatn::Writer writer;
            writer.write_u8(2); // Product type
            writer.write_u32_le(0); // 0 args
            return writer.take_buffer();
        }
    );
    reducerDef.func_type_ref = funcTypeRef.idx;
    reducerDef.lifecycle = std::nullopt;
    
    // Add to module definition
    moduleDef.reducers.push_back(std::move(reducerDef));
    
    // Store the function
    reducerFns.push_back(fn);
    reducerNames.push_back(name);
}

AlgebraicTypeRef Module::RegisterTypeGeneric(const std::string& typeName,
                                     std::function<std::vector<uint8_t>(AlgebraicTypeRef)> makeType) {
    auto& types = moduleDef.typespace.types;
    AlgebraicTypeRef typeRef(static_cast<uint32_t>(types.size()));
    
    // Reserve slot with empty type
    types.push_back(AlgebraicType{});
    
    // Generate the actual type
    auto typeBytes = makeType(typeRef);
    types[typeRef.idx] = AlgebraicType(std::move(typeBytes));
    
    // Register type name
    RawScopedTypeNameV9 scopedName;
    scopedName.name = typeName; // Use provided type name
    
    RawTypeDefV9 typeDef;
    typeDef.name = scopedName;
    typeDef.ty = typeRef.idx;
    typeDef.custom_ordering = true;
    
    moduleDef.types.push_back(std::move(typeDef));
    
    return typeRef;
}

void Module::RegisterTableDirectImpl(const std::string& name, TableAccess access, std::function<std::vector<uint8_t>()> typeGen) {
    RawTableDefV9 table;
    table.name = name;
    table.table_access = access;
    table.table_type = TableType::User;
    
    // Generate and register the type
    auto typeBytes = typeGen();
    auto& types = moduleDef.typespace.types;
    table.product_type_ref = static_cast<uint32_t>(types.size());
    types.push_back(AlgebraicType(std::move(typeBytes)));
    
    // Initialize other fields
    table.primary_key = std::nullopt;
    table.indexes = {};
    table.constraints = {};
    table.sequences = {};
    table.schedule = std::nullopt;
    
    moduleDef.tables.push_back(std::move(table));
}

void Module::RegisterClientVisibilityFilter(const std::string& sql) {
    RawRowLevelSecurityDefV9 rls;
    rls.sql = sql;
    Instance().moduleDef.row_level_security.push_back(std::move(rls));
}

// FFI exports
void Module::__describe_module__(FFI::BytesSink description) {
    try {
        auto& instance = Instance();
        
        // Create versioned module definition
        RawModuleDef versioned;
        versioned.tag = RawModuleDef::Tag::V9;
        versioned.v9 = instance.moduleDef;
        
        // Serialize to BSATN
        bsatn::Writer writer;
        versioned.bsatn_serialize(writer);
        
        auto bytes = writer.take_buffer();
        WriteBytes(description, bytes);
    }
    catch (const std::exception& e) {
        std::cerr << "Error while describing the module: " << e.what() << std::endl;
    }
}

FFI::Errno Module::__call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    Timestamp timestamp,
    FFI::BytesSource args,
    FFI::BytesSink error
) {
    try {
        auto& instance = Instance();
        
        // Reconstruct identity from parts
        sdk::Identity sender;
        std::memcpy(&sender.data[0], &sender_0, 8);
        std::memcpy(&sender.data[8], &sender_1, 8);
        std::memcpy(&sender.data[16], &sender_2, 8);
        std::memcpy(&sender.data[24], &sender_3, 8);
        
        // Reconstruct connection ID
        std::optional<sdk::ConnectionId> connectionId;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            sdk::ConnectionId connId;
            std::memcpy(&connId.data[0], &conn_id_0, 8);
            std::memcpy(&connId.data[8], &conn_id_1, 8);
            connectionId = connId;
        }
        
        // Create SDK reducer context directly (Rust-like)
        sdk::ReducerContext ctx;
        ctx.sender = sender;
        ctx.connection_id = connectionId;
        ctx.timestamp = timestamp;
        // TODO: Initialize db field
        
        // Check which dispatch method to use
        if (!instance.reducerFns.empty() && id < instance.reducerFns.size()) {
            // Direct dispatch (new Rust-like pattern)
            auto argBytes = ConsumeBytes(args);
            return instance.reducerFns[id](ctx, argBytes.data(), argBytes.size());
        } else if (id < instance.reducers.size()) {
            // Legacy IReducer dispatch
            auto internalCtx = instance.newContext(sender, connectionId, timestamp.microseconds_since_epoch, timestamp);
            auto argBytes = ConsumeBytes(args);
            bsatn::Reader reader(argBytes);
            instance.reducers[id]->Invoke(reader, *internalCtx);
            
            if (!reader.is_eos()) {
                throw std::runtime_error("Unrecognised extra bytes in the reducer arguments");
            }
            
            return FFI::Errno::OK;
        } else {
            WriteBytes(error, std::vector<uint8_t>{'N', 'o', ' ', 's', 'u', 'c', 'h', ' ', 'r', 'e', 'd', 'u', 'c', 'e', 'r'});
            return FFI::Errno::NO_SUCH_REDUCER;
        }
    }
    catch (const std::exception& e) {
        auto errorStr = std::string(e.what());
        WriteBytes(error, std::vector<uint8_t>(errorStr.begin(), errorStr.end()));
        return FFI::Errno::HOST_CALL_FAILURE;
    }
}

// Helper implementations
std::vector<uint8_t> ConsumeBytes(FFI::BytesSource source) {
    if (source.handle == FFI::BytesSource::INVALID.handle) {
        return {};
    }
    
    std::vector<uint8_t> buffer(0x20000);
    uint32_t written = 0;
    
    while (true) {
        uint32_t buf_len = static_cast<uint32_t>(buffer.size() - written);
        uint8_t* write_ptr = buffer.data() + written;
        
        auto ret = FFI::bytes_source_read(source, write_ptr, &buf_len);
        written += buf_len;
        
        switch (ret) {
            case FFI::Errno::EXHAUSTED:
                buffer.resize(written);
                return buffer;
                
            case FFI::Errno::OK:
                if (written == buffer.size()) {
                    // Need more space
                    buffer.resize(buffer.size() + 1024);
                }
                break;
                
            case FFI::Errno::NO_SUCH_BYTES:
                throw std::runtime_error("No such bytes");
                
            default:
                throw std::runtime_error("Unknown error reading bytes");
        }
    }
}

void WriteBytes(FFI::BytesSink sink, const std::vector<uint8_t>& bytes) {
    uint32_t start = 0;
    while (start < bytes.size()) {
        uint32_t written = static_cast<uint32_t>(bytes.size() - start);
        const uint8_t* read_ptr = bytes.data() + start;
        
        FFI::bytes_sink_write(sink, read_ptr, &written);
        start += written;
    }
}

// TypeRegistrar is now fully implemented in the header

// Table iterator implementation
template<typename T>
bool RawTableIterBase<T>::Iterator::MoveNext() {
    if (handle == FFI::RowIter::INVALID) {
        return false;
    }
    
    while (true) {
        uint32_t buffer_len = static_cast<uint32_t>(buffer.size());
        auto ret = FFI::row_iter_bsatn_advance(handle, buffer.data(), &buffer_len);
        
        if (ret == FFI::Errno::EXHAUSTED) {
            handle = FFI::RowIter::INVALID;
        }
        
        switch (ret) {
            case FFI::Errno::EXHAUSTED:
            case FFI::Errno::OK:
                current.assign(buffer.begin(), buffer.begin() + buffer_len);
                return buffer_len != 0;
                
            case FFI::Errno::NO_SUCH_ITER:
                throw std::runtime_error("No such iterator");
                
            case FFI::Errno::BUFFER_TOO_SMALL:
                buffer.resize(buffer_len);
                continue;
                
            default:
                throw std::runtime_error("Unknown error in iterator");
        }
    }
}

template<typename T>
std::vector<T> RawTableIterBase<T>::Parse() {
    std::vector<T> results;
    
    for (const auto& chunk : *this) {
        bsatn::Reader reader(chunk);
        while (!reader.is_eos()) {
            T row;
            row.bsatn_deserialize(reader);
            results.push_back(std::move(row));
        }
    }
    
    return results;
}

// Table view implementation
template<typename View, typename T>
std::vector<T> ITableView<View, T>::DoIter() {
    RawTableIter<T> iter(GetTableId());
    return iter.Parse();
}

template<typename View, typename T>
T ITableView<View, T>::DoInsert(const T& row) {
    // Serialize row
    bsatn::Writer writer;
    row.bsatn_serialize(writer);
    auto bytes = writer.take_buffer();
    uint32_t bytes_len = static_cast<uint32_t>(bytes.size());
    
    // Insert into datastore
    FFI::datastore_insert_bsatn(GetTableId(), bytes.data(), &bytes_len);
    
    // Read back generated columns
    bsatn::Reader reader(bytes.data(), bytes_len);
    T result = row;
    return View::ReadGenFields(reader, result);
}

template<typename View, typename T>
bool ITableView<View, T>::DoDelete(const T& row) {
    // Serialize as array-like BSATN with single element
    bsatn::Writer writer;
    writer.write_u32_le(1); // Array length of 1
    row.bsatn_serialize(writer);
    
    auto bytes = writer.take_buffer();
    uint32_t count = 0;
    
    FFI::datastore_delete_all_by_eq_bsatn(
        GetTableId(),
        bytes.data(),
        static_cast<uint32_t>(bytes.size()),
        &count
    );
    
    return count > 0;
}

template<typename View, typename T>
RawConstraintDefV9 ITableView<View, T>::MakeUniqueConstraint(uint16_t colIndex) {
    RawConstraintDefV9 constraint;
    constraint.name = std::nullopt;
    
    RawUniqueConstraintDataV9 uniqueData;
    uniqueData.columns.push_back(colIndex);
    
    RawConstraintDataV9 data;
    data.tag = RawConstraintDataV9::Tag::Unique;
    data.unique = uniqueData;
    
    constraint.data = data;
    return constraint;
}

} // namespace Internal
} // namespace SpacetimeDb