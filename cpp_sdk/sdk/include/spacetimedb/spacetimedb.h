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
    void console_log(uint32_t level, uint32_t msg_ptr, uint32_t msg_len, uint64_t caller1, uint64_t caller2, uint32_t file_ptr, uint32_t file_len, uint32_t line);
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
        if constexpr (sizeof(T) == 1) {
            buf.push_back(static_cast<uint8_t>(val));
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
public:
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
    
    // Reducer context
    class ReducerContext {
    public:
        Database db;
        
        ReducerContext() = default;
    };
}

// Macro helpers
#define SPACETIMEDB_CAT_IMPL(a, b) a##b
#define SPACETIMEDB_CAT(a, b) SPACETIMEDB_CAT_IMPL(a, b)

// Table registration helper
template<typename T>
struct TableRegistrar {
    TableRegistrar(const char* name, bool is_public) {
        ModuleDef::Table table;
        table.name = name;
        table.is_public = is_public;
        table.type = &typeid(T);
        
        // Fields will be added by SPACETIMEDB_REGISTER_FIELD
        
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
};

// Simplified table macro - just defines a marker
#define SPACETIMEDB_TABLE(...) \
    static void SPACETIMEDB_CAT(_spacetimedb_table_marker_, __LINE__)();

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

// Reducer wrapper function
template<typename... Args>
void spacetimedb_reducer_wrapper(void (*func)(spacetimedb::ReducerContext, Args...), 
                                spacetimedb::ReducerContext& ctx, uint32_t args_source) {
    // For now, handle single byte argument
    if constexpr (sizeof...(Args) == 1) {
        spacetimedb::byte arg = spacetimedb::read_u8(args_source);
        func(ctx, arg);
    }
}

// Module-specific bindings will be included by the user module

// Clean reducer macro - just marks the function as a reducer
#define SPACETIMEDB_REDUCER()

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

inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args) {
    auto& module = ModuleDef::instance();
    if (id < module.reducers.size()) {
        spacetimedb::ReducerContext ctx;
        module.reducers[id].handler(ctx, args);
        return 0;
    }
    return -1;
}

// Forward declaration of initialization function
void initialize_module();

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