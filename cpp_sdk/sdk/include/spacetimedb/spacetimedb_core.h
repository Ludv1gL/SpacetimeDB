#pragma once

/**
 * SpacetimeDB C++ SDK - Core Header
 * 
 * This header provides the minimal, conflict-free core functionality for
 * the SpacetimeDB C++ SDK. It includes only the essential types and macros
 * needed to write working modules.
 */

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <optional>
#include <cstring>

// =============================================================================
// FFI DECLARATIONS (from spacetimedb_abi.h)
// =============================================================================

extern "C" {
    // Table operations
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row_bsatn_ptr, size_t* row_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name_ptr, size_t name_len, uint32_t* out_table_id_ptr);
    
    // Logging
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    // BytesSink operations
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // Module exports (these will be implemented by user modules)
    void __describe_module__(uint32_t description);
    int16_t __call_reducer__(uint32_t id, 
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1, 
                            uint64_t timestamp,
                            uint32_t args, uint32_t error);
}

// =============================================================================
// CORE TYPES
// =============================================================================

namespace spacetimedb {

// Forward declarations
struct ReducerContext;
class ModuleDatabase;

// Log levels matching SpacetimeDB
enum class LogLevel : uint32_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4
};

// Simple logging functions
inline void log(LogLevel level, const std::string& message) {
    const char* target = "module";
    const char* filename = "unknown";
    console_log(static_cast<uint8_t>(level), 
                reinterpret_cast<const uint8_t*>(target), strlen(target),
                reinterpret_cast<const uint8_t*>(filename), strlen(filename),
                0, // line number
                reinterpret_cast<const uint8_t*>(message.c_str()), 
                message.length());
}

#define LOG_TRACE(msg) spacetimedb::log(spacetimedb::LogLevel::Trace, msg)
#define LOG_DEBUG(msg) spacetimedb::log(spacetimedb::LogLevel::Debug, msg)  
#define LOG_INFO(msg) spacetimedb::log(spacetimedb::LogLevel::Info, msg)
#define LOG_WARN(msg) spacetimedb::log(spacetimedb::LogLevel::Warn, msg)
#define LOG_ERROR(msg) spacetimedb::log(spacetimedb::LogLevel::Error, msg)

// TODO: Add Identity type once we have the FFI functions

// Basic table handle
template<typename T>
class TableHandle {
    std::string table_name_;
    mutable std::optional<uint32_t> table_id_;
    
public:
    explicit TableHandle(const std::string& name) : table_name_(name) {}
    
    uint32_t get_table_id() const {
        if (!table_id_) {
            uint32_t id;
            uint16_t result = table_id_from_name(
                reinterpret_cast<const uint8_t*>(table_name_.c_str()),
                table_name_.length(),
                &id
            );
            if (result == 0) {
                table_id_ = id;
            } else {
                // Table not found or error
                table_id_ = 0;
            }
        }
        return *table_id_;
    }
    
    bool insert(const T& row) {
        // Serialize row to BSATN
        std::vector<uint8_t> buffer;
        serialize_row(buffer, row);
        
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(get_table_id(), buffer.data(), &buffer_size);
        return result == 0;
    }
    
private:
    void serialize_row(std::vector<uint8_t>& buffer, const T& row) const {
        // This will be implemented by the field registration system
        T::spacetimedb_serialize(buffer, row);
    }
};

// Basic reducer context
struct ReducerContext {
    ModuleDatabase* db;
    
    ReducerContext(ModuleDatabase* database) : db(database) {}
    
    // TODO: Add timestamp() and sender() methods once we have the FFI functions
};

// Module database interface
class ModuleDatabase {
    std::unordered_map<std::string, std::function<void*()>> table_creators_;
    
public:
    template<typename T>
    TableHandle<T> table(const std::string& name) {
        return TableHandle<T>(name);
    }
    
    template<typename T>
    void register_table(const std::string& name) {
        table_creators_[name] = [name]() -> void* {
            return new TableHandle<T>(name);
        };
    }
};

// Global module database instance
inline ModuleDatabase* get_module_db() {
    static ModuleDatabase db;
    return &db;
}

} // namespace spacetimedb

// =============================================================================
// MODULE EXPORTS
// =============================================================================

namespace spacetimedb {

// BSATN Writer (simplified)
class BsatnWriter {
    std::vector<uint8_t>& buffer_;
    
public:
    explicit BsatnWriter(std::vector<uint8_t>& buffer) : buffer_(buffer) {}
    
    void write_u8(uint8_t value) {
        buffer_.push_back(value);
    }
    
    void write_u32(uint32_t value) {
        for (int i = 0; i < 4; ++i) {
            buffer_.push_back(static_cast<uint8_t>(value >> (i * 8)));
        }
    }
    
    void write_string(const std::string& str) {
        write_u32(static_cast<uint32_t>(str.length()));
        for (char c : str) {
            buffer_.push_back(static_cast<uint8_t>(c));
        }
    }
    
    void write_vec_len(size_t len) {
        write_u32(static_cast<uint32_t>(len));
    }
};

// Table definition info
struct TableInfo {
    std::string name;
    bool is_public;
    uint32_t product_type_ref = 0; // Will be set during registration
};

// Reducer definition info  
struct ReducerInfo {
    std::string name;
    std::vector<std::string> param_types;
    std::optional<uint8_t> lifecycle; // 0=Init, 1=OnConnect, 2=OnDisconnect
};

// Module definition builder
class ModuleDefBuilder {
    std::vector<TableInfo> tables_;
    std::vector<ReducerInfo> reducers_;
    
public:
    void add_table(const std::string& name, bool is_public) {
        tables_.push_back({name, is_public, 0});
    }
    
    void add_reducer(const std::string& name, const std::vector<std::string>& param_types,
                     std::optional<uint8_t> lifecycle = std::nullopt) {
        reducers_.push_back({name, param_types, lifecycle});
    }
    
    std::vector<uint8_t> build() {
        std::vector<uint8_t> result;
        BsatnWriter writer(result);
        
        // RawModuleDef::V9 tag
        writer.write_u8(1);
        
        // RawModuleDefV9 structure:
        // - typespace: Typespace (empty for now)
        write_empty_typespace(writer);
        
        // - tables: Vec<RawTableDefV9>
        write_tables(writer);
        
        // - reducers: Vec<RawReducerDefV9>  
        write_reducers(writer);
        
        // - types: Vec<RawTypeDefV9> (empty for now)
        writer.write_vec_len(0);
        
        // - misc_exports: Vec<RawMiscModuleExportV9> (empty for now)
        writer.write_vec_len(0);
        
        // - row_level_security: Vec<RawRowLevelSecurityDefV9> (empty for now) 
        writer.write_vec_len(0);
        
        return result;
    }
    
private:
    void write_empty_typespace(BsatnWriter& writer) {
        // Typespace { types: Vec<AlgebraicType>, names: Vec<ScopedTypeName> }
        writer.write_vec_len(0); // types
        writer.write_vec_len(0); // names
    }
    
    void write_tables(BsatnWriter& writer) {
        writer.write_vec_len(tables_.size());
        
        for (const auto& table : tables_) {
            // RawTableDefV9 structure
            writer.write_string(table.name);           // name
            writer.write_u32(table.product_type_ref);  // product_type_ref
            writer.write_vec_len(0);                   // primary_key (empty)
            writer.write_vec_len(0);                   // indexes (empty)
            writer.write_vec_len(0);                   // constraints (empty)
            writer.write_vec_len(0);                   // sequences (empty)
            writer.write_u8(0);                        // schedule (None)
            writer.write_u8(0);                        // table_type (User)
            writer.write_u8(table.is_public ? 0 : 1);  // table_access (Public=0, Private=1)
        }
    }
    
    void write_reducers(BsatnWriter& writer) {
        writer.write_vec_len(reducers_.size());
        
        for (const auto& reducer : reducers_) {
            // RawReducerDefV9 structure
            writer.write_string(reducer.name);  // name
            
            // params: ProductType (simplified - empty for now)
            writer.write_vec_len(0); // ProductType elements
            
            // lifecycle: Option<Lifecycle>
            if (reducer.lifecycle.has_value()) {
                writer.write_u8(0); // Some tag
                writer.write_u8(reducer.lifecycle.value());
            } else {
                writer.write_u8(1); // None tag
            }
        }
    }
};

// Global module definition registry
class ModuleRegistry {
    ModuleDefBuilder builder_;
    
public:
    static ModuleRegistry& instance() {
        static ModuleRegistry registry;
        return registry;
    }
    
    void register_table(const std::string& name, bool is_public) {
        builder_.add_table(name, is_public);
    }
    
    void register_reducer(const std::string& name, 
                         const std::vector<std::string>& param_types = {},
                         std::optional<uint8_t> lifecycle = std::nullopt) {
        builder_.add_reducer(name, param_types, lifecycle);
    }
    
    std::vector<uint8_t> build_module_def() {
        return builder_.build();
    }
};

} // namespace spacetimedb

// =============================================================================
// REGISTRATION MACROS
// =============================================================================

// Table registration
#define SPACETIMEDB_TABLE(type, name, is_public) \
    namespace { \
        struct type##_table_registrar { \
            type##_table_registrar() { \
                spacetimedb::get_module_db()->register_table<type>(#name); \
                spacetimedb::ModuleRegistry::instance().register_table(#name, is_public); \
            } \
        }; \
        static type##_table_registrar type##_table_reg_; \
    }

// Reducer registration  
#define SPACETIMEDB_REDUCER(name, ...) \
    namespace { \
        struct reducer_##name##_registrar { \
            reducer_##name##_registrar() { \
                spacetimedb::ModuleRegistry::instance().register_reducer(#name); \
            } \
        }; \
        static reducer_##name##_registrar reducer_##name##_reg_; \
    } \
    extern "C" void spacetimedb_reducer_##name(__VA_ARGS__); \
    void spacetimedb_reducer_##name(__VA_ARGS__)

// Built-in reducer registration
#define SPACETIMEDB_INIT(name) \
    namespace { \
        struct reducer_##name##_registrar { \
            reducer_##name##_registrar() { \
                spacetimedb::ModuleRegistry::instance().register_reducer(#name, {}, 0); \
            } \
        }; \
        static reducer_##name##_registrar reducer_##name##_reg_; \
    } \
    extern "C" void spacetimedb_reducer_##name(spacetimedb::ReducerContext ctx); \
    void spacetimedb_reducer_##name(spacetimedb::ReducerContext ctx)

#define SPACETIMEDB_CLIENT_CONNECTED(name) \
    namespace { \
        struct reducer_##name##_registrar { \
            reducer_##name##_registrar() { \
                spacetimedb::ModuleRegistry::instance().register_reducer(#name, {}, 1); \
            } \
        }; \
        static reducer_##name##_registrar reducer_##name##_reg_; \
    } \
    extern "C" void spacetimedb_reducer_##name(spacetimedb::ReducerContext ctx); \
    void spacetimedb_reducer_##name(spacetimedb::ReducerContext ctx)

#define SPACETIMEDB_CLIENT_DISCONNECTED(name) \
    namespace { \
        struct reducer_##name##_registrar { \
            reducer_##name##_registrar() { \
                spacetimedb::ModuleRegistry::instance().register_reducer(#name, {}, 2); \
            } \
        }; \
        static reducer_##name##_registrar reducer_##name##_reg_; \
    } \
    extern "C" void spacetimedb_reducer_##name(spacetimedb::ReducerContext ctx); \
    void spacetimedb_reducer_##name(spacetimedb::ReducerContext ctx)

// Field serialization - to be implemented by user
#define SPACETIMEDB_FIELD_SERIALIZATION(type) \
    namespace type { \
        static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const type& value); \
    }