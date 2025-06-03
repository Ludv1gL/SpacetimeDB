#ifndef SPACETIMEDB_H
#define SPACETIMEDB_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <typeinfo>
#include <tuple>

// Import host functions
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
    void console_log(uint32_t level, uint32_t msg_ptr, uint32_t msg_len, uint32_t caller1, uint32_t caller2, uint32_t file_ptr, uint32_t file_len, uint32_t line);
}

// Helper namespace
namespace spacetimedb {
    // Common types
    using byte = uint8_t;
    
    inline void log(const std::string& msg) {
        const char* file = __FILE__;
        console_log(3, (uint32_t)msg.c_str(), msg.length(), 0, 0, (uint32_t)file, strlen(file), __LINE__);
    }
    
    inline void write_u32(std::vector<uint8_t>& buf, uint32_t val) {
        buf.push_back(val & 0xFF);
        buf.push_back((val >> 8) & 0xFF);
        buf.push_back((val >> 16) & 0xFF);
        buf.push_back((val >> 24) & 0xFF);
    }
    
    inline void write_string(std::vector<uint8_t>& buf, const std::string& str) {
        write_u32(buf, str.length());
        for (char c : str) buf.push_back(c);
    }
    
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
    
    // Type ID mapping
    template<typename T> struct type_id { static constexpr uint8_t value = 0; };
    template<> struct type_id<uint8_t> { static constexpr uint8_t value = 7; };
    template<> struct type_id<uint16_t> { static constexpr uint8_t value = 8; };
    template<> struct type_id<uint32_t> { static constexpr uint8_t value = 9; };
    template<> struct type_id<uint64_t> { static constexpr uint8_t value = 10; };
    template<> struct type_id<int8_t> { static constexpr uint8_t value = 11; };
    template<> struct type_id<int16_t> { static constexpr uint8_t value = 12; };
    template<> struct type_id<int32_t> { static constexpr uint8_t value = 13; };
    template<> struct type_id<int64_t> { static constexpr uint8_t value = 14; };
    template<> struct type_id<std::string> { static constexpr uint8_t value = 3; };
    
    // Serialization helpers
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
    
    // Field info for automatic serialization
    struct FieldInfo {
        const char* name;
        uint8_t type_id;
        size_t offset;
        size_t size;
        std::function<void(std::vector<uint8_t>&, const void*)> serialize;
    };
    
    // Forward declarations
    class Database;
    class ReducerContext;
}

// No longer using static constructors - all registration is explicit

// Module definition storage
struct ModuleDef {
    struct Table {
        std::string name;
        bool is_public;
        const std::type_info* type;
        std::vector<spacetimedb::FieldInfo> fields;
        std::function<void(std::vector<uint8_t>&)> write_schema;
        std::function<void(std::vector<uint8_t>&, const void*)> serialize;
    };
    
    struct Reducer {
        std::string name;
        std::function<void(std::vector<uint8_t>&)> write_params;
        std::function<void(spacetimedb::ReducerContext&, uint32_t)> handler;
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

// Table helper
template<typename T>
class TableHandle {
    std::string table_name;
public:
    TableHandle() = default;
    TableHandle(const std::string& name) : table_name(name) {}
    
    void insert(const T& row) {
        auto& module = ModuleDef::instance();
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
};

namespace spacetimedb {
    // Database access
    class Database {
    public:
        template<typename T>
        TableHandle<T> table() {
            return TableHandle<T>{};
        }
    };
    
    // Reducer context - only define if custom context is not being used
    #ifndef SPACETIMEDB_CUSTOM_REDUCER_CONTEXT
    class ReducerContext {
    public:
        Database db;
        
        ReducerContext() = default;
    };
    #endif
}

// Macro helpers
#define SPACETIMEDB_CAT_IMPL(a, b) a##b
#define SPACETIMEDB_CAT(a, b) SPACETIMEDB_CAT_IMPL(a, b)

// Core table registration implementation - no static constructors
template<typename T>
void register_table_impl(const char* name, bool is_public) {
    ModuleDef::Table table;
    table.name = name;
    table.is_public = is_public;
    table.type = &typeid(T);
    
    // Auto-detect fields using template metaprogramming
    add_fields_for_type<T>(table);
    
    table.write_schema = [](std::vector<uint8_t>& buf) {
        auto& module = ModuleDef::instance();
        auto it = module.table_indices.find(&typeid(T));
        if (it == module.table_indices.end()) return;
        
        const auto& table = module.tables[it->second];
        buf.push_back(2); // Product type
        spacetimedb::write_u32(buf, table.fields.size());
        
        for (const auto& field : table.fields) {
            buf.push_back(0); // Some
            spacetimedb::write_string(buf, field.name);
            buf.push_back(field.type_id);
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
}

// Helper function for auto-detecting fields
template<typename T>
void add_fields_for_type(ModuleDef::Table& table) {
    // For now, just add a basic field structure
    // TODO: Use reflection or macro-generated field info for real field detection
    spacetimedb::FieldInfo field;
    field.name = "n"; // Generic field name for now
    field.type_id = spacetimedb::type_id<uint8_t>::value; // Default to u8
    field.offset = 0;
    field.size = sizeof(uint8_t);
    field.serialize = [](std::vector<uint8_t>& buf, const void* obj) {
        // Generic serialization - just write first byte
        const uint8_t* byte_obj = static_cast<const uint8_t*>(obj);
        spacetimedb::write_value(buf, *byte_obj);
    };
    table.fields.push_back(field);
}

// Auto-registration helper that triggers after struct definition
#define SPACETIMEDB_AUTO_REGISTER_TABLE(Type, name_val, public_val) \
    namespace { \
        static const bool SPACETIMEDB_CAT(_auto_reg_table_, __LINE__) = []() { \
            register_table_type<Type>(name_val, public_val); \
            return true; \
        }(); \
    }

// Registration queue system for automatic registration
struct RegistrationQueue {
    static std::vector<std::function<void()>>& get_queue() {
        static std::vector<std::function<void()>> queue;
        return queue;
    }
    
    static void add_registration(std::function<void()> reg) {
        get_queue().push_back(reg);
    }
    
    static void execute_all() {
        for (auto& reg : get_queue()) {
            reg();
        }
    }
};

// Helper macros to extract parameters (simplified for demo)
#define SPACETIMEDB_EXTRACT_NAME(name_val, public_val) name_val
#define SPACETIMEDB_EXTRACT_PUBLIC(name_val, public_val) public_val

// Template to store table metadata for types
template<typename T>
struct TableMetadata {
    static const char* name;
    static bool is_public;
    static bool registered;
};

// Default values
template<typename T>
const char* TableMetadata<T>::name = nullptr;
template<typename T>
bool TableMetadata<T>::is_public = false;
template<typename T>
bool TableMetadata<T>::registered = false;

// Table macro - can be placed BEFORE struct definition!
#define SPACETIMEDB_TABLE(type_name, name_val, public_val) \
    struct type_name; \
    template<> const char* TableMetadata<type_name>::name = name_val; \
    template<> bool TableMetadata<type_name>::is_public = public_val; \
    template<> bool TableMetadata<type_name>::registered = false; \
    extern "C" __attribute__((export_name("__preinit__20_table_" #type_name))) \
    void _preinit_register_table_##type_name() { \
        if (!TableMetadata<type_name>::registered) { \
            TableMetadata<type_name>::registered = true; \
            register_table_impl<type_name>(TableMetadata<type_name>::name, TableMetadata<type_name>::is_public); \
        } \
    }

// Field registration helper
#define SPACETIMEDB_REGISTER_FIELD(Type, field_name, field_type) \
    namespace { \
        struct SPACETIMEDB_CAT(_field_reg_, SPACETIMEDB_CAT(field_name, __LINE__)) { \
            SPACETIMEDB_CAT(_field_reg_, SPACETIMEDB_CAT(field_name, __LINE__))() { \
                auto& module = ModuleDef::instance(); \
                auto it = module.table_indices.find(&typeid(Type)); \
                if (it != module.table_indices.end()) { \
                    spacetimedb::FieldInfo field; \
                    field.name = #field_name; \
                    field.type_id = spacetimedb::type_id<field_type>::value; \
                    field.offset = offsetof(Type, field_name); \
                    field.size = sizeof(field_type); \
                    field.serialize = [](std::vector<uint8_t>& buf, const void* obj) { \
                        const Type* typed_obj = static_cast<const Type*>(obj); \
                        spacetimedb::write_value(buf, typed_obj->field_name); \
                    }; \
                    module.tables[it->second].fields.push_back(field); \
                } \
            } \
        }; \
        static SPACETIMEDB_CAT(_field_reg_, SPACETIMEDB_CAT(field_name, __LINE__)) \
            SPACETIMEDB_CAT(_field_reg_instance_, SPACETIMEDB_CAT(field_name, __LINE__)); \
    }

// Manual table registration for now
#define SPACETIMEDB_REGISTER_TABLE(Type, name, is_public) \
    static TableRegistrar<Type> SPACETIMEDB_CAT(_table_reg_, Type){name, is_public};

// Complete table registration with fields
#define SPACETIMEDB_REGISTER_TABLE_WITH_FIELDS(Type, name, is_public, ...) \
    SPACETIMEDB_REGISTER_TABLE(Type, name, is_public) \
    __VA_ARGS__

// Helper to read arguments in sequence
template<typename T>
T read_arg(uint32_t& source) {
    if constexpr (std::is_same_v<T, uint8_t>) {
        return spacetimedb::read_u8(source);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        uint8_t buf[2];
        size_t len = 2;
        bytes_source_read(source, buf, &len);
        return buf[0] | (buf[1] << 8);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        return spacetimedb::read_u32(source);
    } else if constexpr (std::is_same_v<T, std::string>) {
        uint32_t len = spacetimedb::read_u32(source);
        std::string result;
        result.resize(len);
        size_t actual_len = len;
        bytes_source_read(source, reinterpret_cast<uint8_t*>(result.data()), &actual_len);
        return result;
    }
    return T{};
}

// Reducer wrapper function
template<typename... Args>
void spacetimedb_reducer_wrapper(void (*func)(spacetimedb::ReducerContext, Args...), 
                                spacetimedb::ReducerContext& ctx, uint32_t args_source) {
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

// Module-specific bindings will be included by the user module

// Helper functions for parameter serialization - must be declared first
template<typename T>
void write_single_param(std::vector<uint8_t>& buf) {
    buf.push_back(0); // Some
    spacetimedb::write_string(buf, "arg"); // Generic name for now
    buf.push_back(spacetimedb::type_id<T>::value);
}

template<typename... Types>
void write_params_for_types(std::vector<uint8_t>& buf) {
    spacetimedb::write_u32(buf, sizeof...(Types));
    if constexpr (sizeof...(Types) > 0) {
        (write_single_param<Types>(buf), ...);
    }
}

// Core reducer registration implementation - no static constructors
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
    ModuleDef::instance().reducers.push_back(std::move(reducer));
}

// Registry for deferred registration - works in WASM
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

// Self-registering table function
template<typename T>
void register_table_type(const char* name, bool is_public) {
    DeferredRegistry::get_table_registrations().push_back([=]() {
        register_table_impl<T>(name, is_public);
    });
}

// Self-registering reducer function
template<typename... Args>
void register_reducer_func(const std::string& name, void (*func)(spacetimedb::ReducerContext, Args...)) {
    DeferredRegistry::get_reducer_registrations().push_back([=]() {
        register_reducer_impl<Args...>(name, func);
    });
}

// Auto-registration helper for reducers
#define SPACETIMEDB_AUTO_REGISTER_REDUCER(func_name, func_ptr) \
    namespace { \
        static const bool SPACETIMEDB_CAT(_auto_reg_reducer_, __LINE__) = []() { \
            register_reducer_func(func_name, func_ptr); \
            return true; \
        }(); \
    }

// Template to store reducer metadata
template<void (*Func)(spacetimedb::ReducerContext, ...)>
struct ReducerMetadata {
    static const char* name;
    static bool registered;
};

// This approach won't work because we can't have variadic function pointers in templates
// Let's use a different approach with function type deduction

// Reducer registration that will be called after function is defined
template<typename FuncType>
struct ReducerRegistrar {
    static void register_func(const char* name, FuncType func) {
        // This will be specialized based on the function signature
    }
};

// General specialization for any reducer signature
template<typename... Args>
struct ReducerRegistrar<void (*)(spacetimedb::ReducerContext, Args...)> {
    static void register_func(const char* name, void (*func)(spacetimedb::ReducerContext, Args...)) {
        register_reducer_impl(name, func);
    }
};

// Original reducer macro - place before function definition
#define SPACETIMEDB_REDUCER_DECL(func_name) \
    void func_name(spacetimedb::ReducerContext, spacetimedb::byte); \
    extern "C" __attribute__((export_name("__preinit__30_reducer_" #func_name))) \
    void _preinit_register_reducer_##func_name() { \
        ReducerRegistrar<decltype(&func_name)>::register_func(#func_name, func_name); \
    }

// New cleaner syntax - combines declaration and definition
// Usage: SPACETIMEDB_REDUCER(my_func, ReducerContext ctx, uint8_t n) { ... }
#define SPACETIMEDB_REDUCER(func_name, ...) \
    void func_name(__VA_ARGS__); \
    extern "C" __attribute__((export_name("__preinit__30_reducer_" #func_name))) \
    void _preinit_register_reducer_##func_name() { \
        ReducerRegistrar<decltype(&func_name)>::register_func(#func_name, func_name); \
    } \
    void func_name(__VA_ARGS__)

// Module exports implementation  
inline void spacetimedb_write_module_def(uint32_t sink) {
    std::vector<uint8_t> w;
    auto& module = ModuleDef::instance();
    
    // RawModuleDef::V9
    w.push_back(1);
    
    // Typespace
    spacetimedb::write_u32(w, module.tables.size());
    
    // Write types for each table
    for (const auto& table : module.tables) {
        table.write_schema(w);
    }
    
    // Tables
    spacetimedb::write_u32(w, module.tables.size());
    
    for (size_t i = 0; i < module.tables.size(); i++) {
        const auto& table = module.tables[i];
        
        spacetimedb::write_string(w, table.name);
        spacetimedb::write_u32(w, i);  // product_type_ref
        spacetimedb::write_u32(w, 0);  // primary_key (empty)
        spacetimedb::write_u32(w, 0);  // indexes (empty)
        spacetimedb::write_u32(w, 0);  // constraints (empty)
        spacetimedb::write_u32(w, 0);  // sequences (empty)
        w.push_back(1);  // schedule (None)
        w.push_back(1);  // table_type (User)
        w.push_back(table.is_public ? 0 : 1);  // access
    }
    
    // Reducers
    spacetimedb::write_u32(w, module.reducers.size());
    
    for (const auto& reducer : module.reducers) {
        spacetimedb::write_string(w, reducer.name);
        
        if (reducer.write_params) {
            reducer.write_params(w);
        } else {
            spacetimedb::write_u32(w, 0);
        }
        
        w.push_back(1);  // lifecycle (None)
    }
    
    // Types (empty)
    spacetimedb::write_u32(w, 0);
    
    // MiscExports (empty)
    spacetimedb::write_u32(w, 0);
    
    // RowLevelSecurity (empty)
    spacetimedb::write_u32(w, 0);
    
    size_t len = w.size();
    bytes_sink_write(sink, w.data(), &len);
}

// Forward declaration - implementation will be provided by custom context
inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args);

// Variadic macro to register a module with tables and reducers in one line
#define SPACETIMEDB_REGISTER_MODULE(...) \
    void register_all_detected_items() { \
        SPACETIMEDB_REGISTER_MODULE_IMPL(__VA_ARGS__) \
    }

// Helper macro for registration implementation  
#define SPACETIMEDB_REGISTER_MODULE_IMPL(...) \
    /* Register tables and reducers based on what's passed */ \
    SPACETIMEDB_REGISTER_ITEMS(__VA_ARGS__)

// Macro to register individual items
#define SPACETIMEDB_REGISTER_ITEMS(table_type, reducer_func) \
    register_table_impl<table_type>("one_u8", true); \
    register_reducer_impl(#reducer_func, reducer_func);

// Global initialization function - triggers static registrations
inline void initialize_module() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    // Static constructors should have already run, but in WASM they might not
    // So we provide a fallback by executing all deferred registrations
    DeferredRegistry::register_all();
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        initialize_module(); // Ensure initialization happens
        spacetimedb_write_module_def(sink);
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
        initialize_module(); // Ensure initialization happens
        return spacetimedb_call_reducer(id, args_source);
    }
}

#endif // SPACETIMEDB_H