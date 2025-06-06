#ifndef SPACETIMEDB_MACROS_SIMPLE_H
#define SPACETIMEDB_MACROS_SIMPLE_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

// =============================================================================
// SPACETIMEDB FFI IMPORTS
// =============================================================================

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, const uint8_t* row, size_t row_len);
}

// =============================================================================
// MODULE DEFINITION BUILDER
// =============================================================================

namespace spacetimedb {

// Simple BSATN writer
class ModuleBuilder {
private:
    std::vector<uint8_t> buffer;
    
    struct TableInfo {
        std::string name;
        std::vector<uint8_t> type_def;
    };
    
    struct ReducerInfo {
        std::string name;
        std::vector<uint8_t> params_def;
    };
    
    std::vector<TableInfo> tables;
    std::vector<ReducerInfo> reducers;
    
public:
    static ModuleBuilder& instance() {
        static ModuleBuilder inst;
        return inst;
    }
    
    void add_table(const std::string& name, const std::vector<uint8_t>& type_def) {
        tables.push_back({name, type_def});
    }
    
    void add_reducer(const std::string& name, const std::vector<uint8_t>& params_def) {
        reducers.push_back({name, params_def});
    }
    
    size_t get_table_count() const { return tables.size(); }
    size_t get_reducer_count() const { return reducers.size(); }
    
    void write_u8(uint8_t val) {
        buffer.push_back(val);
    }
    
    void write_u32_le(uint32_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back((val >> 16) & 0xFF);
        buffer.push_back((val >> 24) & 0xFF);
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.size());
        buffer.insert(buffer.end(), str.begin(), str.end());
    }
    
    void write_bytes(const std::vector<uint8_t>& data) {
        buffer.insert(buffer.end(), data.begin(), data.end());
    }
    
    std::vector<uint8_t> build_module_def() {
        buffer.clear();
        
        // Write RawModuleDef::V9 (variant 1)
        write_u8(1);
        
        // Typespace
        write_u32_le(tables.size());
        
        // Write types for each table
        for (const auto& table : tables) {
            write_bytes(table.type_def);
        }
        
        // Tables
        write_u32_le(tables.size());
        
        for (size_t i = 0; i < tables.size(); i++) {
            write_string(tables[i].name);
            write_u32_le(i); // product_type_ref
            write_u32_le(0); // primary_key (empty)
            write_u32_le(0); // indexes (empty)
            write_u32_le(0); // constraints (empty)
            write_u32_le(0); // sequences (empty)
            write_u8(1);     // schedule: None
            write_u8(1);     // table_type: User
            write_u8(0);     // table_access: Public
        }
        
        // Reducers
        write_u32_le(reducers.size());
        
        for (const auto& reducer : reducers) {
            write_string(reducer.name);
            write_bytes(reducer.params_def);
            write_u8(1); // lifecycle: None
        }
        
        // Empty arrays for the rest
        write_u32_le(0); // types
        write_u32_le(0); // misc_exports
        write_u32_le(0); // row_level_security
        
        return buffer;
    }
};

// Global reducer handlers storage
using ReducerHandler = void(*)(const uint8_t*, size_t);
std::vector<ReducerHandler> g_reducer_handlers;

// Helper to write to sink
inline void write_to_sink(uint32_t sink, const std::vector<uint8_t>& data) {
    size_t written = data.size();
    bytes_sink_write(sink, data.data(), &written);
}

// Helper to log messages
inline void log_info(const std::string& message) {
    console_log(2, // INFO
               (const uint8_t*)"module", 6,
               (const uint8_t*)__FILE__, strlen(__FILE__),
               __LINE__,
               (const uint8_t*)message.c_str(), message.size());
}

// BSATN type writer helper
class TypeWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t val) {
        buffer.push_back(val);
    }
    
    void write_u32_le(uint32_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back((val >> 16) & 0xFF);
        buffer.push_back((val >> 24) & 0xFF);
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.size());
        buffer.insert(buffer.end(), str.begin(), str.end());
    }
    
    std::vector<uint8_t> get_buffer() { return buffer; }
};

// BSATN value writer
class ValueWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t val) {
        buffer.push_back(val);
    }
    
    void write_u32_le(uint32_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back((val >> 16) & 0xFF);
        buffer.push_back((val >> 24) & 0xFF);
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.size());
        buffer.insert(buffer.end(), str.begin(), str.end());
    }
    
    std::vector<uint8_t> get_buffer() const { return buffer; }
};

// BSATN reader
class ValueReader {
    const uint8_t* data;
    size_t size;
    size_t pos;
public:
    ValueReader(const uint8_t* d, size_t s) : data(d), size(s), pos(0) {}
    
    uint8_t read_u8() {
        if (pos >= size) return 0;
        return data[pos++];
    }
    
    uint32_t read_u32_le() {
        if (pos + 4 > size) return 0;
        uint32_t val = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24);
        pos += 4;
        return val;
    }
    
    std::string read_string() {
        uint32_t len = read_u32_le();
        if (pos + len > size) return "";
        std::string str((const char*)(data + pos), len);
        pos += len;
        return str;
    }
};

} // namespace spacetimedb

// =============================================================================
// TABLE MACRO
// =============================================================================

#define SPACETIMEDB_TABLE(TableType) \
    /* Table ID cache */ \
    static uint32_t TableType##_table_id = 0; \
    /* Get table ID */ \
    uint32_t get_##TableType##_table_id() { \
        if (TableType##_table_id == 0) { \
            const char* name = #TableType; \
            table_id_from_name((const uint8_t*)name, strlen(name), &TableType##_table_id); \
        } \
        return TableType##_table_id; \
    } \
    /* Insert function */ \
    void insert_##TableType(const TableType& row) { \
        spacetimedb::ValueWriter writer; \
        TableType##_serialize(row, writer); \
        auto buffer = writer.get_buffer(); \
        datastore_insert_bsatn(get_##TableType##_table_id(), buffer.data(), buffer.size()); \
    } \
    /* Registration */ \
    namespace { \
        struct Register##TableType { \
            Register##TableType() { \
                spacetimedb::TypeWriter type_writer; \
                type_writer.write_u8(2); /* Product type */ \
                TableType##_write_type(type_writer); \
                spacetimedb::ModuleBuilder::instance().add_table(#TableType, type_writer.get_buffer()); \
            } \
        }; \
        static Register##TableType register_##TableType##_instance; \
    }

// Define table with 3 fields
#define SPACETIMEDB_TABLE_3(TableType, type1, field1, type2, field2, type3, field3) \
    void TableType##_write_type(spacetimedb::TypeWriter& writer) { \
        writer.write_u32_le(3); /* 3 fields */ \
        /* Field 1 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#field1); \
        writer.write_u8(type1##_TYPE_ID); \
        /* Field 2 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#field2); \
        writer.write_u8(type2##_TYPE_ID); \
        /* Field 3 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#field3); \
        writer.write_u8(type3##_TYPE_ID); \
    } \
    void TableType##_serialize(const TableType& obj, spacetimedb::ValueWriter& writer) { \
        writer.write_u32_le(obj.field1); \
        writer.write_string(obj.field2); \
        writer.write_u8(obj.field3); \
    }

// Type constants
#define uint32_t_TYPE_ID 11  // U32
#define uint8_t_TYPE_ID 7    // U8
#define std_string_TYPE_ID 4 // String

// =============================================================================
// REDUCER MACRO
// =============================================================================

#define SPACETIMEDB_REDUCER(name) \
    /* Forward declare handler */ \
    void name##_handler(const uint8_t* args, size_t args_len); \
    /* Registration */ \
    namespace { \
        struct Register##name##Reducer { \
            Register##name##Reducer() { \
                spacetimedb::TypeWriter params_writer; \
                name##_write_params(params_writer); \
                spacetimedb::ModuleBuilder::instance().add_reducer(#name, params_writer.get_buffer()); \
                spacetimedb::g_reducer_handlers.push_back(name##_handler); \
            } \
        }; \
        static Register##name##Reducer register_##name##_reducer_instance; \
    }

// Reducer with no parameters
#define SPACETIMEDB_REDUCER_0(name) \
    void name##_write_params(spacetimedb::TypeWriter& writer) { \
        writer.write_u32_le(0); /* 0 params */ \
    } \
    void name##_handler(const uint8_t* args, size_t args_len) { \
        name##_impl(); \
    } \
    void name##_impl()

// Reducer with 3 parameters
#define SPACETIMEDB_REDUCER_3(name, type1, param1, type2, param2, type3, param3) \
    void name##_write_params(spacetimedb::TypeWriter& writer) { \
        writer.write_u32_le(3); /* 3 params */ \
        /* Param 1 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#param1); \
        writer.write_u8(type1##_TYPE_ID); \
        /* Param 2 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#param2); \
        writer.write_u8(type2##_TYPE_ID); \
        /* Param 3 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#param3); \
        writer.write_u8(type3##_TYPE_ID); \
    } \
    void name##_handler(const uint8_t* args, size_t args_len) { \
        spacetimedb::ValueReader reader(args, args_len); \
        type1 param1 = reader.read_u32_le(); \
        type2 param2 = reader.read_string(); \
        type3 param3 = reader.read_u8(); \
        name##_impl(param1, param2, param3); \
    } \
    void name##_impl(type1 param1, type2 param2, type3 param3)

// =============================================================================
// MODULE EXPORTS - Auto-generated only if not using library
// =============================================================================

#ifndef SPACETIMEDB_MODULE_LIBRARY_EXPORTS

extern "C" {

__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    auto module_def = spacetimedb::ModuleBuilder::instance().build_module_def();
    spacetimedb::write_to_sink(sink, module_def);
}

__attribute__((export_name("__call_reducer__")))
int32_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    if (id < spacetimedb::g_reducer_handlers.size()) {
        // Read arguments
        size_t args_len = 1024;
        uint8_t args_buffer[1024];
        bytes_source_read(args, args_buffer, &args_len);
        
        // Call handler
        spacetimedb::g_reducer_handlers[id](args_buffer, args_len);
        
        return 0;
    }
    
    return -999; // Unknown reducer
}

} // extern "C"

#endif // SPACETIMEDB_MODULE_LIBRARY_EXPORTS

#endif // SPACETIMEDB_MACROS_SIMPLE_H