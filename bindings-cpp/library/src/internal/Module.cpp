#include "spacetimedb/internal/Module.h"
#include "spacetimedb/internal/autogen/RawModuleDef.g.h"
#include "spacetimedb/internal/autogen/RawUniqueConstraintDataV9.g.h"
#include "spacetimedb/reducer_context.h"
#include "spacetimedb/builtin_reducers.h"  // For Identity
#include "spacetimedb/timestamp.h"  // For Timestamp
#include <iostream>
#include <cstring>

namespace SpacetimeDb {
namespace Internal {

// Bring library types into scope
using ::SpacetimeDb::ConnectionId;
using ::SpacetimeDb::Identity;
using ::SpacetimeDb::Timestamp;

// Default implementation of IReducerContext
class DefaultReducerContext : public IReducerContext {
private:
    Identity sender_;
    std::optional<ConnectionId> connectionId_;
    Timestamp timestamp_;
    uint64_t seed_;
    
public:
    DefaultReducerContext(Identity sender, 
                         std::optional<ConnectionId> connectionId,
                         uint64_t seed,
                         Timestamp timestamp)
        : sender_(sender), connectionId_(connectionId), 
          timestamp_(timestamp), seed_(seed) {}
    
    Identity GetSender() const override { return sender_; }
    std::optional<ConnectionId> GetConnectionId() const override { return connectionId_; }
    Timestamp GetTimestamp() const override { return timestamp_; }
    
    ReducerContext ToSdkContext() override {
        // Create Module Library context - implementation depends on ReducerContext structure
        ReducerContext ctx;
        // TODO: Initialize ctx with sender, connectionId, timestamp
        return ctx;
    }
};

// FFI constants
// RowIter and BytesSource are simple typedefs, not classes
const FFI::RowIter INVALID_ROW_ITER = 0xFFFFFFFF;
const FFI::BytesSource INVALID_BYTES_SOURCE = 0xFFFFFFFF;

// Module implementation
Module& Module::Instance() {
    static Module instance;
    return instance;
}

Module::Module() : typeRegistrar(std::make_unique<TypeRegistrar>(*this)) {
    // Initialize empty module definition
    moduleDef = RawModuleDefV9{};
    
    // Set default context constructor
    newContext = [](::SpacetimeDb::Identity sender, std::optional<::SpacetimeDb::ConnectionId> connectionId, 
                    uint64_t seed, ::SpacetimeDb::Timestamp timestamp) {
        return std::make_unique<DefaultReducerContext>(sender, connectionId, seed, timestamp);
    };
}

void Module::SetReducerContextConstructor(
    std::function<std::unique_ptr<IReducerContext>(
        ::SpacetimeDb::Identity, std::optional<::SpacetimeDb::ConnectionId>, uint64_t, ::SpacetimeDb::Timestamp
    )> ctor
) {
    Instance().newContext = std::move(ctor);
}

void Module::RegisterReducerImpl(std::unique_ptr<IReducer> reducer) {
    // Get reducer definition
    auto reducerDef = reducer->MakeReducerDef(*typeRegistrar);
    
    // Add to module definition
    moduleDef.reducers.push_back(std::move(reducerDef));
    
    // Store the reducer function
    // Note: We're not storing reducer instances anymore, just functions
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
    // Create empty ProductType for params (for now)
    reducerDef.params = ProductType{};
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
    // TODO: Implement proper AlgebraicType creation from bytes
    // types[typeRef.idx] = AlgebraicType(std::move(typeBytes));
    
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
    // TODO: Implement proper AlgebraicType creation from bytes
    types.push_back(AlgebraicType{});
    
    // Initialize other fields
    table.primary_key = {};  // Empty vector, not optional
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
void Module::__describe_module__(uint32_t description) {
    try {
        auto& instance = Instance();
        
        // TODO: Implement proper module definition serialization
        // The autogenerated RawModuleDef is incomplete
        bsatn::Writer writer;
        instance.moduleDef.bsatn_serialize(writer);
        
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
    uint32_t args,
    uint32_t error
) {
    try {
        auto& instance = Instance();
        
        // Reconstruct identity from parts
        std::array<uint8_t, 32> senderBytes{};
        // Copy the 4 uint64_t values into the byte array
        memcpy(senderBytes.data(), &sender_0, sizeof(uint64_t));
        memcpy(senderBytes.data() + 8, &sender_1, sizeof(uint64_t));
        memcpy(senderBytes.data() + 16, &sender_2, sizeof(uint64_t));
        memcpy(senderBytes.data() + 24, &sender_3, sizeof(uint64_t));
        ::SpacetimeDb::Identity sender(senderBytes);
        
        // Reconstruct connection ID
        std::optional<ConnectionId> connectionId;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            // ConnectionId is just a uint64_t wrapper
            connectionId = ConnectionId(conn_id_0);
            // Note: conn_id_1 seems to be extra data, possibly for future use
        }
        
        // Create ReducerContext using existing sender and connectionId
        // timestamp is already in microseconds
        uint64_t timestamp_micros = timestamp.micros_since_epoch();
        ::SpacetimeDb::Timestamp ts = ::SpacetimeDb::Timestamp::from_micros_since_epoch(timestamp_micros);
        
        // Convert connectionId to the right type
        std::optional<::SpacetimeDb::ConnectionId> connection_id;
        if (connectionId.has_value()) {
            connection_id = ::SpacetimeDb::ConnectionId(connectionId->id);
        }
        
        // Create ReducerContext
        ::SpacetimeDb::ReducerContext ctx(sender, connection_id, ts);
        
        // Check which dispatch method to use
        if (!instance.reducerFns.empty() && id < instance.reducerFns.size()) {
            // Direct dispatch (new Rust-like pattern)
            // Convert EnhancedReducerContext to ReducerContext
            // Need to convert SpacetimeDb::Identity to Identity
            Identity libSender(ctx.sender.get_bytes());
            Timestamp libTimestamp(ctx.timestamp.millis_since_epoch());
            ReducerContext libCtx(libSender, ctx.connection_id, libTimestamp);
            
            auto argBytes = ConsumeBytes(args);
            // Note: reducerFns expect ReducerContext, not EnhancedReducerContext
            // This may need adjustment based on actual reducer function signatures
            // return instance.reducerFns[id](libCtx, argBytes.data(), argBytes.size());
            return FFI::Errno::OK; // Placeholder for now
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
    if (source == INVALID_BYTES_SOURCE) {
        return {};
    }
    
    std::vector<uint8_t> buffer;
    buffer.reserve(1024); // Start with reasonable size
    
    while (true) {
        // Ensure buffer has some spare capacity
        if (buffer.size() == buffer.capacity()) {
            buffer.reserve(buffer.capacity() + 1024);
        }
        
        // Calculate available space
        size_t current_size = buffer.size();
        size_t capacity = buffer.capacity(); 
        size_t spare_capacity = capacity - current_size;
        
        // Expand buffer to use all available capacity
        buffer.resize(capacity);
        
        // Read into spare capacity
        size_t buf_len = spare_capacity;
        uint8_t* write_ptr = buffer.data() + current_size;
        
        int16_t ret = FFI::bytes_source_read(source, write_ptr, &buf_len);
        
        // Update size to reflect actual bytes read
        buffer.resize(current_size + buf_len);
        
        switch (ret) {
            case -1: // Host side source exhausted, we're done
                return buffer;
                
            case 0: // Success, continue reading
                // If we didn't fill the available space, we might be done
                // but continue to try reading more
                break;
                
            default: // Error
                throw std::runtime_error("Error reading from bytes source: " + std::to_string(ret));
        }
    }
}

void WriteBytes(FFI::BytesSink sink, const std::vector<uint8_t>& bytes) {
    size_t start = 0;
    while (start < bytes.size()) {
        size_t remaining = bytes.size() - start;
        const uint8_t* read_ptr = bytes.data() + start;
        
        uint16_t result = ::bytes_sink_write(sink, read_ptr, &remaining);
        if (result != 0) {
            throw std::runtime_error("Error writing to bytes sink: " + std::to_string(result));
        }
        start += remaining;
    }
}

// TypeRegistrar is now fully implemented in the header

// Table iterator implementation
template<typename T>
bool RawTableIterBase<T>::Iterator::MoveNext() {
    if (handle == INVALID_ROW_ITER) {
        return false;
    }
    
    while (true) {
        size_t buffer_len = buffer.size();
        int16_t ret = ::row_iter_bsatn_advance(handle, buffer.data(), &buffer_len);
        
        // Check for exhausted iterator
        if (ret == -1) { // EXHAUSTED
            handle = INVALID_ROW_ITER;
            return false;
        }
        
        if (ret >= 0) {
            // Success - we got data
            current.assign(buffer.begin(), buffer.begin() + buffer_len);
            return buffer_len != 0;
        } else {
            // Error occurred
            switch (static_cast<uint16_t>(-ret)) {
                case 6: // NO_SUCH_ITER
                    throw std::runtime_error("No such iterator");
                    
                case 11: // BUFFER_TOO_SMALL
                    buffer.resize(buffer_len);
                    continue;
                    
                default:
                    throw std::runtime_error("Unknown error in iterator: " + std::to_string(ret));
            }
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
    
    // TODO: This needs to be properly implemented when the autogenerated types are complete
    // For now, return an empty constraint as the autogenerated types are incomplete
    RawConstraintDataV9 data;
    // Cannot set tag_ as it's private and there's no setter method
    // Cannot set variant data since the type is incomplete
    
    constraint.data = data;
    return constraint;
}

} // namespace Internal
} // namespace SpacetimeDb