#ifndef SPACETIMEDB_H
#define SPACETIMEDB_H

/**
 * SpacetimeDB C++ Module Library - Unified Header
 * 
 * This header provides the complete C++ Module Library for SpacetimeDB modules.
 * It combines all functionality into a single, well-organized file.
 * 
 * Features:
 * - Table registration and operations
 * - Reducer definitions
 * - Enhanced logging system
 * - BSATN serialization support
 * - Error handling
 * - Advanced query and index management
 */

// =============================================================================
// SYSTEM INCLUDES
// =============================================================================

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <typeinfo>
#include <tuple>
#include <stdexcept>
#include <memory>
#include <optional>

// =============================================================================
// MODULE LIBRARY FEATURE INCLUDES
// =============================================================================

#include "abi/spacetimedb_abi.h"  // SpacetimeDB ABI
#include "library/logging.h"        // Enhanced logging system
#include "library/exceptions.h"     // Rich error handling
#include "library/database.h"       // Database types
#include "bsatn/bsatn.h"        // BSATN serialization (includes AlgebraicType)
#include "field_registration.h"  // Field registration system
#include "table_ops.h"          // Table operations
#include "macros.h"             // Core macros
#include "builtin_reducers.h"   // Built-in reducer support (defines Identity)
#include "credentials.h"        // Credential management
// Note: reducer_context_enhanced.h defines ReducerContext, so we don't include the basic version
#include "library/reducer_context_enhanced.h" // Enhanced reducer context (uses Identity)
#include "internal/Module.h"    // Module registration
#include "timestamp.h"          // Timestamp type for scheduled reducers
#include "time_duration.h"      // TimeDuration type for scheduled reducers
#include "schedule_reducer.h"   // Scheduled reducer support
// Advanced features - to be integrated
// #include "library/query_operations.h"  // Advanced query operations
// #include "library/index_management.h"  // Index management
// #include "library/schema_management.h" // Schema management

// =============================================================================
// ENHANCED LOGGING MACROS
// =============================================================================

#ifndef LOG_TRACE
#define LOG_TRACE(msg) spacetimedb::log_trace(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_DEBUG  
#define LOG_DEBUG(msg) spacetimedb::log_debug(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_INFO
#define LOG_INFO(msg) spacetimedb::log_info(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_WARN
#define LOG_WARN(msg) spacetimedb::log_warn(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(msg) spacetimedb::log_error(msg, __func__, __FILE__, __LINE__)
#endif

// FFI declarations are provided by abi/spacetimedb_abi.h and internal/FFI.h

// =============================================================================
// UTILITY MACROS
// =============================================================================

#define SPACETIMEDB_CAT_IMPL(a, b) a##b
#define SPACETIMEDB_CAT(a, b) SPACETIMEDB_CAT_IMPL(a, b)

// =============================================================================
// CORE SPACETIMEDB NAMESPACE
// =============================================================================

namespace spacetimedb {

// -----------------------------------------------------------------------------
// Type Aliases and Forward Declarations
// -----------------------------------------------------------------------------

using byte = uint8_t;
class Database;

// -----------------------------------------------------------------------------
// Table Name Registry
// -----------------------------------------------------------------------------

namespace detail {
    constexpr size_t MAX_TABLES = 64;
    inline const char* table_names[MAX_TABLES] = {};
    inline size_t table_count = 0;
    
    inline void register_table_name(const char* name) {
        if (table_count < MAX_TABLES) {
            table_names[table_count++] = name;
        }
    }
}

// -----------------------------------------------------------------------------
// Binary I/O Utilities
// -----------------------------------------------------------------------------

#ifndef SPACETIMEDB_WRITE_UTILS_DEFINED
#define SPACETIMEDB_WRITE_UTILS_DEFINED

inline void write_u32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

inline void write_string(std::vector<uint8_t>& buf, const std::string& str) {
    write_u32(buf, static_cast<uint32_t>(str.length()));
    for (char c : str) buf.push_back(static_cast<uint8_t>(c));
}

#endif // SPACETIMEDB_WRITE_UTILS_DEFINED

inline uint8_t read_u8(uint32_t source) {
    uint8_t val = 0;
    ::BytesSource src{static_cast<uint16_t>(source)};
    _bytes_source_read(src, &val, 1);
    return val;
}

inline uint32_t read_u32(uint32_t source) {
    uint8_t buf[4] = {0};
    ::BytesSource src{static_cast<uint16_t>(source)};
    _bytes_source_read(src, buf, 4);
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

// -----------------------------------------------------------------------------
// Type System (Legacy compatibility)
// -----------------------------------------------------------------------------

template<typename T> struct type_id { static constexpr uint8_t value = 0; };
template<> struct type_id<bool> { static constexpr uint8_t value = 1; };
template<> struct type_id<uint8_t> { static constexpr uint8_t value = 7; };
template<> struct type_id<uint16_t> { static constexpr uint8_t value = 8; };
template<> struct type_id<uint32_t> { static constexpr uint8_t value = 9; };
template<> struct type_id<uint64_t> { static constexpr uint8_t value = 10; };
template<> struct type_id<int8_t> { static constexpr uint8_t value = 11; };
template<> struct type_id<int16_t> { static constexpr uint8_t value = 12; };
template<> struct type_id<int32_t> { static constexpr uint8_t value = 13; };
template<> struct type_id<int64_t> { static constexpr uint8_t value = 14; };
template<> struct type_id<float> { static constexpr uint8_t value = 15; };
template<> struct type_id<double> { static constexpr uint8_t value = 16; };
template<> struct type_id<std::string> { static constexpr uint8_t value = 3; };
// For complex types, we'll use 0 which will need proper handling
template<typename T> struct type_id<std::vector<T>> { static constexpr uint8_t value = 0; };
template<typename T> struct type_id<std::optional<T>> { static constexpr uint8_t value = 0; };

// Serialization helper
template<typename T>
void write_value(std::vector<uint8_t>& buf, const T& val) {
    if constexpr (std::is_same_v<T, std::string>) {
        write_string(buf, val);
    } else if constexpr (sizeof(T) == 1) {
        buf.push_back(static_cast<uint8_t>(val));
    } else if constexpr (sizeof(T) == 2) {
        buf.push_back(val & 0xFF);
        buf.push_back((val >> 8) & 0xFF);
    } else if constexpr (sizeof(T) == 4) {
        write_u32(buf, static_cast<uint32_t>(val));
    }
}

// -----------------------------------------------------------------------------
// Module Definition System
// -----------------------------------------------------------------------------

struct FieldInfo {
    const char* name;
    uint8_t type_id;
    size_t offset;
    size_t size;
    std::function<void(std::vector<uint8_t>&, const void*)> serialize;
};

struct ModuleDef {
    struct Table {
        std::string name;
        bool is_public;
        const std::type_info* type;
        std::vector<FieldInfo> fields;
        std::function<void(std::vector<uint8_t>&)> write_schema;
        std::function<void(std::vector<uint8_t>&, const void*)> serialize;
    };
    
    struct Reducer {
        std::string name;
        std::function<void(std::vector<uint8_t>&)> write_params;
        std::function<void(spacetimedb::ReducerContext&, uint32_t)> handler;
        std::optional<Lifecycle> lifecycle;
    };
    
    std::vector<Table> tables;
    std::vector<Reducer> reducers;
    std::map<const std::type_info*, size_t> table_indices;
    
    static ModuleDef& instance() {
        static ModuleDef inst;
        return inst;
    }
    
    void add_table(Table table) {
        table_indices[table.type] = tables.size();
        tables.push_back(std::move(table));
    }
};

} // namespace spacetimedb

// -----------------------------------------------------------------------------
// Table Iterator
// -----------------------------------------------------------------------------

template<typename T>
class TableIterator {
private:
    uint32_t handle_ = 0xFFFFFFFF;
    std::vector<uint8_t> buffer_;
    std::vector<T> current_batch_;
    size_t current_index_ = 0;
    bool done_ = false;
    
    void fetch_next_batch() {
        if (done_) return;
        
        buffer_.resize(0x20000); // 128KB buffer
        ::Buffer buf_handle;
        
        auto ret = _iter_next(handle_, &buf_handle);
        
        if (ret == 2) { // EXHAUSTED
            done_ = true;
            return;
        }
        
        if (ret != 0) {
            done_ = true;
            return;
        }
        
        // Get buffer length and read data
        size_t buf_len = _buffer_len(buf_handle);
        buffer_.resize(buf_len);
        _buffer_consume(buf_handle, buffer_.data(), buf_len);
        
        // Parse the batch - the buffer contains concatenated BSATN-encoded rows
        current_batch_.clear();
        size_t pos = 0;
        
        auto& module = spacetimedb::ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) {
            done_ = true;
            return;
        }
        
        // For now, assume each row is serialized as a fixed-size struct
        // In a real implementation, we'd need to parse BSATN properly
        const auto& table = module.tables[it->second];
        
        // Simple approach: assume we know the size of each row
        // This is a placeholder - proper BSATN parsing needed
        size_t estimated_row_size = sizeof(T);
        
        while (pos + estimated_row_size <= buf_len) {
            T row{};
            // This is simplified - in reality we'd deserialize field by field
            std::memcpy(&row, buffer_.data() + pos, sizeof(T));
            current_batch_.push_back(row);
            pos += estimated_row_size;
        }
        
        current_index_ = 0;
    }
    
public:
    explicit TableIterator(uint32_t table_id) {
        ::BufferIter iter;
        if (_iter_start(table_id, &iter) == 0) {
            handle_ = iter;
            fetch_next_batch();
        } else {
            done_ = true;
        }
    }
    
    ~TableIterator() {
        if (handle_ != 0xFFFFFFFF) {
            _iter_drop(handle_);
        }
    }
    
    // Iterator interface
    bool has_next() const {
        return !done_ || current_index_ < current_batch_.size();
    }
    
    T next() {
        if (current_index_ >= current_batch_.size()) {
            fetch_next_batch();
        }
        
        if (done_ && current_index_ >= current_batch_.size()) {
            throw std::runtime_error("Iterator exhausted");
        }
        
        return current_batch_[current_index_++];
    }
    
    // Range-for support
    class iterator {
        TableIterator* parent_;
        std::optional<T> current_;
        
    public:
        explicit iterator(TableIterator* parent) : parent_(parent) {
            if (parent && parent->has_next()) {
                current_ = parent->next();
            }
        }
        
        iterator& operator++() {
            if (parent_ && parent_->has_next()) {
                current_ = parent_->next();
            } else {
                current_.reset();
            }
            return *this;
        }
        
        bool operator!=(const iterator& other) const {
            return current_.has_value() != other.current_.has_value();
        }
        
        const T& operator*() const { return *current_; }
    };
    
    iterator begin() { return iterator(this); }
    iterator end() { return iterator(nullptr); }
};

// -----------------------------------------------------------------------------
// Table Handle
// -----------------------------------------------------------------------------

template<typename T>
class TableHandle {
    std::string table_name_;
    mutable uint32_t table_id_ = 0;
    mutable bool id_resolved_ = false;
    
    void resolve_table_id() const {
        if (id_resolved_) return;
        
        auto& module = spacetimedb::ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) return;
        
        const auto& table = module.tables[it->second];
        if (_get_table_id((const uint8_t*)table.name.c_str(), table.name.length(), &table_id_) == 0) {
            id_resolved_ = true;
        }
    }
    
public:
    TableHandle() = default;
    explicit TableHandle(const std::string& name) : table_name_(name) {}
    
    // Insert a row into the table
    T insert(const T& row) {
        resolve_table_id();
        
        auto& module = spacetimedb::ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) return row;
        
        const auto& table = module.tables[it->second];
        
        std::vector<uint8_t> data;
        table.serialize(data, &row);
        
        size_t len = data.size();
        auto err = _insert(table_id_, data.data(), len);
        
        if (err == 0 && len > 0) {
            // Parse returned row (may have auto-inc fields)
            // TODO: Implement proper deserialization
            return row;
        }
        
        return row;
    }
    
    // Count rows in the table
    uint64_t count() const {
        resolve_table_id();
        
        // TODO: Once we have a proper count FFI function
        // For now, iterate and count
        uint64_t count = 0;
        auto iter = this->iter();
        while (iter.has_next()) {
            iter.next();
            count++;
        }
        return count;
    }
    
    // Iterate over all rows
    TableIterator<T> iter() const {
        resolve_table_id();
        return TableIterator<T>(table_id_);
    }
    
    // Delete rows matching a value
    bool delete_by_value(const T& value) {
        resolve_table_id();
        
        auto& module = spacetimedb::ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) return false;
        
        const auto& table = module.tables[it->second];
        
        std::vector<uint8_t> data;
        table.serialize(data, &value);
        
        // For now, we'll use delete_by_col_eq with col_id 0 (first column)
        // In a full implementation, this would be more sophisticated
        uint32_t deleted_count = 0;
        auto err = _delete_by_col_eq(table_id_, 0, data.data(), data.size(), &deleted_count);
        
        return err == 0 && deleted_count > 0;
    }
    
    // Update a row (delete old, insert new)
    bool update(const T& old_value, const T& new_value) {
        if (delete_by_value(old_value)) {
            insert(new_value);
            return true;
        }
        return false;
    }
    
    std::string get_table_name() const { return table_name_; }
};

// -----------------------------------------------------------------------------
// Database Classes
// -----------------------------------------------------------------------------

class ModuleDatabaseBase {
public:
    template<typename T>
    TableHandle<T> table(const char* name) {
        return TableHandle<T>(name);
    }
    
    template<typename T>
    TableHandle<T> get(const char* table_name) {
        return table<T>(table_name);
    }
    
    bool has_table(const char* name) const {
        using namespace spacetimedb::detail;
        for (size_t i = 0; i < table_count; ++i) {
            if (table_names[i] && std::strcmp(table_names[i], name) == 0) {
                return true;
            }
        }
        return false;
    }
    
    size_t get_table_count() const {
        return spacetimedb::detail::table_count;
    }
};

// X-Macro pattern support
#ifndef SPACETIMEDB_TABLES_LIST
#define SPACETIMEDB_TABLES_LIST
#endif

// Forward declare types from X-macro list
#define X(TypeName, table_name, is_public) \
    struct TypeName;
SPACETIMEDB_TABLES_LIST
#undef X

class ModuleDatabase : public ModuleDatabaseBase {
public:
    // Generate accessor methods from X-macro list
    #define X(TypeName, table_name, is_public) \
        TableHandle<TypeName> table_name() { \
            return table<TypeName>(#table_name); \
        }
    SPACETIMEDB_TABLES_LIST
    #undef X
};

namespace spacetimedb {

// -----------------------------------------------------------------------------
// Table Registration
// -----------------------------------------------------------------------------

template<typename T>
void add_fields_for_type(ModuleDef::Table& table) {
    // Check if fields have been registered for this type
    auto& descriptors = get_table_descriptors();
    auto it = descriptors.find(&typeid(T));
    
    if (it != descriptors.end()) {
        // Use registered fields
        for (const auto& field_desc : it->second.fields) {
            FieldInfo field;
            field.name = field_desc.name.c_str();
            field.type_id = 0;  // Not used with new system
            field.offset = field_desc.offset;
            field.size = field_desc.size;
            field.serialize = field_desc.serialize;
            table.fields.push_back(field);
        }
    } else {
        // Fallback for unregistered types (single uint8_t field)
        FieldInfo field;
        field.name = "n";
        field.type_id = type_id<uint8_t>::value;
        field.offset = 0;
        field.size = sizeof(uint8_t);
        field.serialize = [](std::vector<uint8_t>& buf, const void* obj) {
            const uint8_t* byte_obj = static_cast<const uint8_t*>(obj);
            write_value(buf, *byte_obj);
        };
        table.fields.push_back(field);
    }
}

template<typename T>
void register_table_impl(const char* name, bool is_public) {
    ModuleDef::Table table;
    table.name = name;
    table.is_public = is_public;
    table.type = &typeid(T);
    
    add_fields_for_type<T>(table);
    
    table.write_schema = [](std::vector<uint8_t>& buf) {
        auto& module = ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) return;
        
        const auto& table = module.tables[it->second];
        
        // Check if we have registered field descriptors
        auto& descriptors = get_table_descriptors();
        auto desc_it = descriptors.find(&typeid(T));
        
        if (desc_it != descriptors.end()) {
            // Use the simplified working format that matches the fallback
            buf.push_back(2); // Product type
            write_u32(buf, desc_it->second.fields.size());
            
            for (const auto& field_desc : desc_it->second.fields) {
                // Write in the same format as the fallback case but with registered info
                buf.push_back(0); // Some (field name present)
                write_string(buf, field_desc.name);
                
                // Write the correct type for this field
                field_desc.write_type(buf);
            }
        } else {
            // Fallback to simple format
            buf.push_back(2); // Product type
            write_u32(buf, table.fields.size());
            
            for (const auto& field : table.fields) {
                buf.push_back(0); // Some - BSATN Option::Some = 0
                write_string(buf, field.name);
                buf.push_back(field.type_id);
            }
        }
    };
    
    table.serialize = [](std::vector<uint8_t>& buf, const void* obj) {
        auto& module = ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) return;
        
        const auto& table = module.tables[it->second];
        for (const auto& field : table.fields) {
            field.serialize(buf, obj);
        }
    };
    
    ModuleDef::instance().add_table(std::move(table));
    detail::register_table_name(name);
}

// -----------------------------------------------------------------------------
// Reducer Registration
// -----------------------------------------------------------------------------

template<typename T>
T read_arg(uint32_t& source) {
    if constexpr (std::is_same_v<T, uint8_t>) {
        return read_u8(source);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        uint8_t buf[2];
        ::BytesSource src{static_cast<uint16_t>(source)};
        _bytes_source_read(src, buf, 2);
        return buf[0] | (buf[1] << 8);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return read_u32(source);
    } else if constexpr (std::is_same_v<T, std::string>) {
        uint32_t len = read_u32(source);
        std::string result;
        result.resize(len);
        ::BytesSource src{static_cast<uint16_t>(source)};
        _bytes_source_read(src, reinterpret_cast<uint8_t*>(result.data()), len);
        return result;
    }
    return T{};
}

template<typename... Args>
void spacetimedb_reducer_wrapper(void (*func)(spacetimedb::ReducerContext, Args...), 
                                spacetimedb::ReducerContext& ctx, uint32_t args_source) {
    if constexpr (sizeof...(Args) == 0) {
        func(ctx);
    } else if constexpr (sizeof...(Args) == 1) {
        using ArgType = std::tuple_element_t<0, std::tuple<Args...>>;
        if constexpr (std::is_same_v<ArgType, Identity>) {
            // For built-in reducers with Identity, this is handled specially
            // The Identity is passed through the context
            func(ctx, Identity{});
        } else {
            auto arg = read_arg<ArgType>(args_source);
            func(ctx, arg);
        }
    } else if constexpr (sizeof...(Args) == 2) {
        auto arg1 = read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(args_source);
        auto arg2 = read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(args_source);
        func(ctx, arg1, arg2);
    } else if constexpr (sizeof...(Args) == 3) {
        auto arg1 = read_arg<std::tuple_element_t<0, std::tuple<Args...>>>(args_source);
        auto arg2 = read_arg<std::tuple_element_t<1, std::tuple<Args...>>>(args_source);
        auto arg3 = read_arg<std::tuple_element_t<2, std::tuple<Args...>>>(args_source);
        func(ctx, arg1, arg2, arg3);
    }
}

template<typename T>
void write_single_param(std::vector<uint8_t>& buf) {
    buf.push_back(0); // Some
    write_string(buf, "arg");
    buf.push_back(type_id<T>::value);
}

template<typename... Types>
void write_params_for_types(std::vector<uint8_t>& buf) {
    write_u32(buf, sizeof...(Types));
    if constexpr (sizeof...(Types) > 0) {
        (write_single_param<Types>(buf), ...);
    }
}

template<typename... Args>
void register_reducer_impl(const std::string& name, void (*func)(spacetimedb::ReducerContext, Args...)) {
    ModuleDef::Reducer reducer;
    reducer.name = name;
    reducer.handler = [func](spacetimedb::ReducerContext& ctx, uint32_t args) {
        spacetimedb_reducer_wrapper(func, ctx, args);
    };
    reducer.write_params = [](std::vector<uint8_t>& buf) {
        write_params_for_types<Args...>(buf);
    };
    // Check if this is a lifecycle reducer
    reducer.lifecycle = detail::get_lifecycle_for_name(name);
    ModuleDef::instance().reducers.push_back(std::move(reducer));
}

// Specialized registration for init reducer
inline void register_init_reducer(void (*func)(spacetimedb::ReducerContext)) {
    ModuleDef::Reducer reducer;
    reducer.name = "init";
    reducer.lifecycle = Lifecycle::Init;
    reducer.handler = [func](spacetimedb::ReducerContext& ctx, uint32_t) {
        func(ctx);
    };
    reducer.write_params = [](std::vector<uint8_t>& buf) {
        write_u32(buf, 0);  // No parameters
    };
    ModuleDef::instance().reducers.push_back(std::move(reducer));
}

// Specialized registration for client_connected reducer
inline void register_client_connected_reducer(void (*func)(spacetimedb::ReducerContext, Identity)) {
    ModuleDef::Reducer reducer;
    reducer.name = "client_connected";
    reducer.lifecycle = Lifecycle::OnConnect;
    reducer.handler = [func](spacetimedb::ReducerContext& ctx, uint32_t) {
        Identity sender(g_sender_parts[0], g_sender_parts[1], g_sender_parts[2], g_sender_parts[3]);
        func(ctx, sender);
    };
    reducer.write_params = [](std::vector<uint8_t>& buf) {
        write_u32(buf, 0);  // No parameters from args
    };
    ModuleDef::instance().reducers.push_back(std::move(reducer));
}

// Specialized registration for client_disconnected reducer
inline void register_client_disconnected_reducer(void (*func)(spacetimedb::ReducerContext, Identity)) {
    ModuleDef::Reducer reducer;
    reducer.name = "client_disconnected";
    reducer.lifecycle = Lifecycle::OnDisconnect;
    reducer.handler = [func](spacetimedb::ReducerContext& ctx, uint32_t) {
        Identity sender(g_sender_parts[0], g_sender_parts[1], g_sender_parts[2], g_sender_parts[3]);
        func(ctx, sender);
    };
    reducer.write_params = [](std::vector<uint8_t>& buf) {
        write_u32(buf, 0);  // No parameters from args
    };
    ModuleDef::instance().reducers.push_back(std::move(reducer));
}

template<typename FuncType>
struct ReducerRegistrar {
    static void register_func(const char* name, FuncType func) {}
};

template<typename... Args>
struct ReducerRegistrar<void (*)(spacetimedb::ReducerContext, Args...)> {
    static void register_func(const char* name, void (*func)(spacetimedb::ReducerContext, Args...)) {
        register_reducer_impl(name, func);
    }
};

// -----------------------------------------------------------------------------
// Module Exports
// -----------------------------------------------------------------------------

inline void spacetimedb_write_module_def(uint32_t sink) {
    std::vector<uint8_t> w;
    auto& module = ModuleDef::instance();
    
    // RawModuleDef::V9
    w.push_back(1);
    
    // Typespace
    write_u32(w, module.tables.size());
    
    // Write types for each table
    for (const auto& table : module.tables) {
        table.write_schema(w);
    }
    
    // Tables
    write_u32(w, module.tables.size());
    
    for (size_t i = 0; i < module.tables.size(); i++) {
        const auto& table = module.tables[i];
        
        write_string(w, table.name);
        write_u32(w, i);  // product_type_ref
        write_u32(w, 0);  // primary_key (empty)
        write_u32(w, 0);  // indexes (empty)
        write_u32(w, 0);  // constraints (empty)
        write_u32(w, 0);  // sequences (empty)
        w.push_back(1);  // schedule (Option::None = tag 1)
        w.push_back(1);  // table_type (TableType::User = 1)
        w.push_back(table.is_public ? 0 : 1);  // table_access (TableAccess::Public=0, Private=1)
    }
    
    // Reducers
    write_u32(w, module.reducers.size());
    
    for (const auto& reducer : module.reducers) {
        write_string(w, reducer.name);
        
        if (reducer.write_params) {
            reducer.write_params(w);
        } else {
            write_u32(w, 0);
        }
        
        // Write lifecycle information
        if (reducer.lifecycle.has_value()) {
            w.push_back(0);  // Some
            w.push_back(static_cast<uint8_t>(reducer.lifecycle.value()));
        } else {
            w.push_back(1);  // None
        }
    }
    
    // Types (empty)
    write_u32(w, 0);
    
    // MiscExports (empty)
    write_u32(w, 0);
    
    // RowLevelSecurity (empty)
    write_u32(w, 0);
    
    size_t len = w.size();
    ::BytesSink snk{static_cast<uint16_t>(sink)};
    ::bytes_sink_write(snk, w.data(), &len);
}

// Global variables to pass sender identity to lifecycle reducers
thread_local uint64_t g_sender_parts[4] = {0, 0, 0, 0};

inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args, 
                                       uint64_t sender_0, uint64_t sender_1, 
                                       uint64_t sender_2, uint64_t sender_3) {
    auto& module = ModuleDef::instance();
    if (id < module.reducers.size()) {
        // Store sender parts for lifecycle reducers
        g_sender_parts[0] = sender_0;
        g_sender_parts[1] = sender_1;
        g_sender_parts[2] = sender_2;
        g_sender_parts[3] = sender_3;
        
        spacetimedb::ReducerContext ctx;
        module.reducers[id].handler(ctx, args);
        return 0;
    }
    return -1;
}

// -----------------------------------------------------------------------------
// Deferred Registration System
// -----------------------------------------------------------------------------

struct DeferredRegistry {
    static std::vector<std::function<void()>>& get_table_registrations() {
        static std::vector<std::function<void()>> table_regs;
        return table_regs;
    }
    
    static std::vector<std::function<void()>>& get_reducer_registrations() {
        static std::vector<std::function<void()>> reducer_regs;
        return reducer_regs;
    }
    
    static void register_all() {
        for (auto& reg : get_table_registrations()) {
            reg();
        }
        for (auto& reg : get_reducer_registrations()) {
            reg();
        }
    }
};

template<typename T>
void register_table_type(const char* name, bool is_public) {
    DeferredRegistry::get_table_registrations().push_back([=]() {
        register_table_impl<T>(name, is_public);
    });
}

template<typename... Args>
void register_reducer_func(const std::string& name, void (*func)(ReducerContext, Args...)) {
    DeferredRegistry::get_reducer_registrations().push_back([=]() {
        register_reducer_impl<Args...>(name, func);
    });
}

// -----------------------------------------------------------------------------
// Module Initialization
// -----------------------------------------------------------------------------

inline void initialize_module() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    DeferredRegistry::register_all();
}

} // namespace spacetimedb

// =============================================================================
// REGISTRATION MACROS
// =============================================================================

// Generate table registration functions from X-macro list
#define X(TypeName, table_name, is_public) \
    __attribute__((export_name("__preinit__20_table_" #table_name))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_table_, table_name)() { \
        spacetimedb::register_table_impl<TypeName>(#table_name, is_public); \
        spacetimedb::detail::register_table_name(#table_name); \
    }
SPACETIMEDB_TABLES_LIST
#undef X

// Macros are already defined in macros.h

// =============================================================================
// MODULE EXPORTS
// =============================================================================

extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        spacetimedb::initialize_module();
        spacetimedb::spacetimedb_write_module_def(sink);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        spacetimedb::initialize_module();
        return spacetimedb::spacetimedb_call_reducer(id, args_source, sender_0, sender_1, sender_2, sender_3);
    }
}

// =============================================================================
// CONVENIENCE ALIASES
// =============================================================================

namespace spacetimedb {
    using Context = ReducerContext;
    using DB = ModuleDatabase;
}

// =============================================================================
// AUTOMATIC FIELD REGISTRATION MACROS
// =============================================================================

// Helper macro to register a single field
#define SPACETIMEDB_AUTO_FIELD(struct_type, field_name, field_type) \
    { \
        spacetimedb::FieldDescriptor desc; \
        desc.name = #field_name; \
        desc.offset = offsetof(struct_type, field_name); \
        desc.size = sizeof(field_type); \
        desc.write_type = [](std::vector<uint8_t>& buf) { \
            spacetimedb::write_field_type<field_type>(buf); \
        }; \
        desc.serialize = [](std::vector<uint8_t>& buf, const void* obj) { \
            const struct_type* typed_obj = static_cast<const struct_type*>(obj); \
            spacetimedb::serialize_value(buf, typed_obj->field_name); \
        }; \
        spacetimedb::get_table_descriptors()[&typeid(struct_type)].fields.push_back(desc); \
    }

// Macro to define a struct and automatically register its fields
#define SPACETIMEDB_STRUCT(struct_name, ...) \
    struct struct_name { __VA_ARGS__ }; \
    namespace { \
        struct struct_name##_field_registrar { \
            struct_name##_field_registrar(); \
        }; \
        static struct_name##_field_registrar struct_name##_field_registrar_instance; \
    }

// Short-form macros for convenience
#define STDB_TABLE SPACETIMEDB_TABLE
#define STDB_REDUCER SPACETIMEDB_REDUCER 
#define STDB_STRUCT SPACETIMEDB_STRUCT

#endif // SPACETIMEDB_H