#ifndef SPACETIMEDB_H
#define SPACETIMEDB_H

/**
 * SpacetimeDB C++ SDK - Unified Header
 * 
 * This header provides the complete C++ SDK for SpacetimeDB modules.
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
// SDK FEATURE INCLUDES
// =============================================================================

#include "sdk/logging.h"        // Enhanced logging system
#include "sdk/exceptions.h"     // Rich error handling
#include "sdk/database.h"       // Database types
#include "sdk/reducer_context.h" // Reducer context
#include "bsatn/bsatn.h"        // BSATN serialization
#include "field_registration.h"  // Field registration system
#include "table_ops.h"          // Table operations
#include "macros.h"             // Core macros
#include "internal/Module.h"    // Module registration

// =============================================================================
// ENHANCED LOGGING MACROS
// =============================================================================

#ifndef LOG_TRACE
#define LOG_TRACE(msg) SpacetimeDB::log_trace(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_DEBUG  
#define LOG_DEBUG(msg) SpacetimeDB::log_debug(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_INFO
#define LOG_INFO(msg) SpacetimeDB::log_info(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_WARN
#define LOG_WARN(msg) SpacetimeDB::log_warn(msg, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(msg) SpacetimeDB::log_error(msg, __func__, __FILE__, __LINE__)
#endif

// =============================================================================
// FFI DECLARATIONS - SpacetimeDB Host Interface
// =============================================================================

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row_ptr, size_t* row_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name_ptr, size_t name_len, uint32_t* out);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint32_t level, uint32_t msg_ptr, uint32_t msg_len, 
                     uint32_t caller1, uint32_t caller2, uint32_t file_ptr, 
                     uint32_t file_len, uint32_t line);
}

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
class ReducerContext;

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
    size_t len = 1;
    bytes_source_read(source, &val, &len);
    return val;
}

inline uint32_t read_u32(uint32_t source) {
    uint8_t buf[4] = {0};
    size_t len = 4;
    bytes_source_read(source, buf, &len);
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
        std::function<void(ReducerContext&, uint32_t)> handler;
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
// Table Handle
// -----------------------------------------------------------------------------

template<typename T>
class TableHandle {
    std::string table_name_;
    
public:
    TableHandle() = default;
    explicit TableHandle(const std::string& name) : table_name_(name) {}
    
    void insert(const T& row) {
        auto& module = spacetimedb::ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) return;
        
        const auto& table = module.tables[it->second];
        uint32_t table_id;
        if (table_id_from_name((const uint8_t*)table.name.c_str(), table.name.length(), &table_id) != 0) {
            return;
        }
        
        std::vector<uint8_t> data;
        table.serialize(data, &row);
        
        size_t len = data.size();
        datastore_insert_bsatn(table_id, data.data(), &len);
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
// Reducer Context
// -----------------------------------------------------------------------------

class ReducerContext {
public:
    ModuleDatabase db;
    ReducerContext() = default;
};

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
        size_t len = 2;
        bytes_source_read(source, buf, &len);
        return buf[0] | (buf[1] << 8);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return read_u32(source);
    } else if constexpr (std::is_same_v<T, std::string>) {
        uint32_t len = read_u32(source);
        std::string result;
        result.resize(len);
        size_t actual_len = len;
        bytes_source_read(source, reinterpret_cast<uint8_t*>(result.data()), &actual_len);
        return result;
    }
    return T{};
}

template<typename... Args>
void spacetimedb_reducer_wrapper(void (*func)(ReducerContext, Args...), 
                                ReducerContext& ctx, uint32_t args_source) {
    if constexpr (sizeof...(Args) == 0) {
        func(ctx);
    } else if constexpr (sizeof...(Args) == 1) {
        auto arg = read_arg<Args...>(args_source);
        func(ctx, arg);
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
void register_reducer_impl(const std::string& name, void (*func)(ReducerContext, Args...)) {
    ModuleDef::Reducer reducer;
    reducer.name = name;
    reducer.handler = [func](ReducerContext& ctx, uint32_t args) {
        spacetimedb_reducer_wrapper(func, ctx, args);
    };
    reducer.write_params = [](std::vector<uint8_t>& buf) {
        write_params_for_types<Args...>(buf);
    };
    ModuleDef::instance().reducers.push_back(std::move(reducer));
}

template<typename FuncType>
struct ReducerRegistrar {
    static void register_func(const char* name, FuncType func) {}
};

template<typename... Args>
struct ReducerRegistrar<void (*)(ReducerContext, Args...)> {
    static void register_func(const char* name, void (*func)(ReducerContext, Args...)) {
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
        
        w.push_back(1);  // lifecycle (None)
    }
    
    // Types (empty)
    write_u32(w, 0);
    
    // MiscExports (empty)
    write_u32(w, 0);
    
    // RowLevelSecurity (empty)
    write_u32(w, 0);
    
    size_t len = w.size();
    bytes_sink_write(sink, w.data(), &len);
}

inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args) {
    auto& module = ModuleDef::instance();
    if (id < module.reducers.size()) {
        ReducerContext ctx;
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

// Table registration macro
#define SPACETIMEDB_TABLE(type_name, table_name, is_public) \
    __attribute__((export_name("__preinit__20_table_" #table_name))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_table_, table_name)() { \
        spacetimedb::register_table_impl<type_name>(#table_name, is_public); \
        spacetimedb::detail::register_table_name(#table_name); \
    }

// Reducer registration macro
#define SPACETIMEDB_REDUCER(func_name, ...) \
    void func_name(__VA_ARGS__); \
    extern "C" __attribute__((export_name("__preinit__30_reducer_" #func_name))) \
    void _preinit_register_reducer_##func_name() { \
        spacetimedb::ReducerRegistrar<decltype(&func_name)>::register_func(#func_name, func_name); \
    } \
    void func_name(__VA_ARGS__)

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
        return spacetimedb::spacetimedb_call_reducer(id, args_source);
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