#ifndef SPACETIMEDB_MACROS_STANDALONE_H
#define SPACETIMEDB_MACROS_STANDALONE_H

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

// =============================================================================
// CORE SPACETIMEDB IMPORTS
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
// BSATN WRITER
// =============================================================================

namespace spacetimedb {

class BsatnWriter {
private:
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
    
    void write_bytes(const uint8_t* data, size_t len) {
        buffer.insert(buffer.end(), data, data + len);
    }
    
    const std::vector<uint8_t>& get_buffer() const { return buffer; }
    size_t size() const { return buffer.size(); }
    const uint8_t* data() const { return buffer.data(); }
};

// BSATN reader
class BsatnReader {
private:
    const uint8_t* data;
    size_t size;
    size_t pos;
public:
    BsatnReader(const uint8_t* d, size_t s) : data(d), size(s), pos(0) {}
    
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

// Helper functions
inline void write_to_sink(uint32_t sink, const uint8_t* data, size_t len) {
    size_t written = len;
    bytes_sink_write(sink, data, &written);
}

inline void log_info(const std::string& message) {
    console_log(2, // INFO
               (const uint8_t*)"module", 6,
               (const uint8_t*)__FILE__, strlen(__FILE__),
               __LINE__,
               (const uint8_t*)message.c_str(), message.size());
}

} // namespace spacetimedb

// =============================================================================
// MODULE DEFINITION STORAGE
// =============================================================================

namespace spacetimedb_module_internal {

// Storage for table definitions
struct TableDef {
    std::string name;
    uint32_t field_count;
    void (*write_fields)(spacetimedb::BsatnWriter&);
};

// Storage for reducer definitions  
struct ReducerDef {
    std::string name;
    uint32_t param_count;
    void (*write_params)(spacetimedb::BsatnWriter&);
    void (*handler)(const uint8_t*, size_t);
};

// Global storage
std::vector<TableDef> g_tables;
std::vector<ReducerDef> g_reducers;
std::vector<uint32_t> g_table_ids;

} // namespace spacetimedb_module_internal

// =============================================================================
// TABLE MACROS
// =============================================================================

#define SPACETIMEDB_TABLE(TableType) \
    /* Forward declare serialization function */ \
    void TableType##_write_fields(spacetimedb::BsatnWriter& writer); \
    void TableType##_serialize(const TableType& obj, spacetimedb::BsatnWriter& writer); \
    /* Table registration */ \
    namespace { \
        struct Register##TableType { \
            Register##TableType() { \
                spacetimedb_module_internal::TableDef def; \
                def.name = #TableType; \
                def.field_count = TableType##_field_count(); \
                def.write_fields = TableType##_write_fields; \
                spacetimedb_module_internal::g_tables.push_back(def); \
                spacetimedb_module_internal::g_table_ids.push_back(0); \
            } \
        }; \
        static Register##TableType register_##TableType##_instance; \
    } \
    /* Table operations */ \
    namespace TableType##_ops { \
        inline uint32_t get_table_id() { \
            size_t idx = 0; \
            for (size_t i = 0; i < spacetimedb_module_internal::g_tables.size(); i++) { \
                if (spacetimedb_module_internal::g_tables[i].name == #TableType) { \
                    idx = i; \
                    break; \
                } \
            } \
            if (spacetimedb_module_internal::g_table_ids[idx] == 0) { \
                const char* name = #TableType; \
                table_id_from_name((const uint8_t*)name, strlen(name), \
                                 &spacetimedb_module_internal::g_table_ids[idx]); \
            } \
            return spacetimedb_module_internal::g_table_ids[idx]; \
        } \
        inline void insert(const TableType& row) { \
            spacetimedb::BsatnWriter writer; \
            TableType##_serialize(row, writer); \
            datastore_insert_bsatn(get_table_id(), writer.data(), writer.size()); \
        } \
    }

// Define table fields - for 3 fields
#define SPACETIMEDB_TABLE_FIELDS_3(TableType, type1, field1, type2, field2, type3, field3) \
    constexpr uint32_t TableType##_field_count() { return 3; } \
    void TableType##_write_fields(spacetimedb::BsatnWriter& writer) { \
        /* Field 1 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#field1); \
        writer.write_u8(type1##_type_id()); \
        /* Field 2 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#field2); \
        writer.write_u8(type2##_type_id()); \
        /* Field 3 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#field3); \
        writer.write_u8(type3##_type_id()); \
    } \
    void TableType##_serialize(const TableType& obj, spacetimedb::BsatnWriter& writer) { \
        type1##_serialize(obj.field1, writer); \
        type2##_serialize(obj.field2, writer); \
        type3##_serialize(obj.field3, writer); \
    }

// =============================================================================
// TYPE HELPERS
// =============================================================================

// Type IDs
constexpr uint8_t uint32_t_type_id() { return 11; } // U32
constexpr uint8_t uint8_t_type_id() { return 7; }   // U8
constexpr uint8_t string_type_id() { return 4; }    // String

// Serialization helpers
inline void uint32_t_serialize(uint32_t val, spacetimedb::BsatnWriter& writer) {
    writer.write_u32_le(val);
}

inline void uint8_t_serialize(uint8_t val, spacetimedb::BsatnWriter& writer) {
    writer.write_u8(val);
}

inline void string_serialize(const std::string& val, spacetimedb::BsatnWriter& writer) {
    writer.write_string(val);
}

// Type aliases for cleaner syntax
using string = std::string;

// =============================================================================
// REDUCER MACROS
// =============================================================================

#define SPACETIMEDB_REDUCER(name) \
    /* Forward declare */ \
    void name##_write_params(spacetimedb::BsatnWriter& writer); \
    void name##_handler(const uint8_t* args, size_t args_len); \
    /* Registration */ \
    namespace { \
        struct Register##name##Reducer { \
            Register##name##Reducer() { \
                spacetimedb_module_internal::ReducerDef def; \
                def.name = #name; \
                def.param_count = name##_param_count(); \
                def.write_params = name##_write_params; \
                def.handler = name##_handler; \
                spacetimedb_module_internal::g_reducers.push_back(def); \
            } \
        }; \
        static Register##name##Reducer register_##name##_reducer_instance; \
    }

// Reducer with no parameters
#define SPACETIMEDB_REDUCER_IMPL_0(name) \
    constexpr uint32_t name##_param_count() { return 0; } \
    void name##_write_params(spacetimedb::BsatnWriter& writer) {} \
    void name##_handler(const uint8_t* args, size_t args_len) { \
        name##_impl(); \
    } \
    void name##_impl()

// Reducer with 3 parameters
#define SPACETIMEDB_REDUCER_IMPL_3(name, type1, param1, type2, param2, type3, param3) \
    constexpr uint32_t name##_param_count() { return 3; } \
    void name##_write_params(spacetimedb::BsatnWriter& writer) { \
        /* Param 1 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#param1); \
        writer.write_u8(type1##_type_id()); \
        /* Param 2 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#param2); \
        writer.write_u8(type2##_type_id()); \
        /* Param 3 */ \
        writer.write_u8(0); /* Some */ \
        writer.write_string(#param3); \
        writer.write_u8(type3##_type_id()); \
    } \
    void name##_handler(const uint8_t* args, size_t args_len) { \
        spacetimedb::BsatnReader reader(args, args_len); \
        type1 param1 = type1##_deserialize(reader); \
        type2 param2 = type2##_deserialize(reader); \
        type3 param3 = type3##_deserialize(reader); \
        name##_impl(param1, param2, param3); \
    } \
    void name##_impl(type1 param1, type2 param2, type3 param3)

// Deserialization helpers
inline uint32_t uint32_t_deserialize(spacetimedb::BsatnReader& reader) {
    return reader.read_u32_le();
}

inline uint8_t uint8_t_deserialize(spacetimedb::BsatnReader& reader) {
    return reader.read_u8();
}

inline std::string string_deserialize(spacetimedb::BsatnReader& reader) {
    return reader.read_string();
}

// =============================================================================
// MODULE EXPORTS
// =============================================================================

// Auto-generate module exports
extern "C" {

__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    spacetimedb::BsatnWriter writer;
    
    // Write RawModuleDef::V9 (variant 1)
    writer.write_u8(1);
    
    // Typespace
    writer.write_u32_le(spacetimedb_module_internal::g_tables.size());
    
    // Write types for each table
    for (size_t i = 0; i < spacetimedb_module_internal::g_tables.size(); i++) {
        // Type i: ProductType (variant 2)
        writer.write_u8(2);
        
        // ProductType with N elements
        writer.write_u32_le(spacetimedb_module_internal::g_tables[i].field_count);
        
        // Write fields
        spacetimedb_module_internal::g_tables[i].write_fields(writer);
    }
    
    // Tables
    writer.write_u32_le(spacetimedb_module_internal::g_tables.size());
    
    for (size_t i = 0; i < spacetimedb_module_internal::g_tables.size(); i++) {
        // Table name
        writer.write_string(spacetimedb_module_internal::g_tables[i].name);
        writer.write_u32_le(i); // product_type_ref
        writer.write_u32_le(0); // primary_key (empty)
        writer.write_u32_le(0); // indexes (empty)
        writer.write_u32_le(0); // constraints (empty)
        writer.write_u32_le(0); // sequences (empty)
        writer.write_u8(1);     // schedule: None
        writer.write_u8(1);     // table_type: User
        writer.write_u8(0);     // table_access: Public
    }
    
    // Reducers
    writer.write_u32_le(spacetimedb_module_internal::g_reducers.size());
    
    for (const auto& reducer : spacetimedb_module_internal::g_reducers) {
        writer.write_string(reducer.name);
        writer.write_u32_le(reducer.param_count);
        reducer.write_params(writer);
        writer.write_u8(1); // lifecycle: None
    }
    
    // Empty arrays for the rest
    writer.write_u32_le(0); // types
    writer.write_u32_le(0); // misc_exports
    writer.write_u32_le(0); // row_level_security
    
    // Write to sink
    spacetimedb::write_to_sink(sink, writer.data(), writer.size());
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
    if (id < spacetimedb_module_internal::g_reducers.size()) {
        // Read arguments
        size_t args_len = 1024;
        uint8_t args_buffer[1024];
        bytes_source_read(args, args_buffer, &args_len);
        
        // Call handler
        spacetimedb_module_internal::g_reducers[id].handler(args_buffer, args_len);
        
        spacetimedb::log_info("Reducer " + spacetimedb_module_internal::g_reducers[id].name + " called");
        return 0;
    }
    
    return -999; // Unknown reducer
}

} // extern "C"

#endif // SPACETIMEDB_MACROS_STANDALONE_H