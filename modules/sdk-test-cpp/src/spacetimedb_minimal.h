// Minimal SpacetimeDB C++ SDK - No WASI dependencies
// This provides just enough functionality to build modules without pulling in the C++ standard library

#ifndef SPACETIMEDB_MINIMAL_H
#define SPACETIMEDB_MINIMAL_H

#include <cstdint>
#include <cstddef>

// Import/Export macros
#define STDB_IMPORT(name) \
    __attribute__((import_module("spacetime_10.0"), import_name(#name))) extern

#define STDB_EXPORT(name) __attribute__((export_name(#name)))

// Basic SpacetimeDB imports we need
extern "C" {

STDB_IMPORT(bytes_sink_write)
uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);

STDB_IMPORT(bytes_source_read)
int16_t bytes_source_read(uint32_t source, uint8_t* buffer_ptr, size_t* buffer_len_ptr);

STDB_IMPORT(datastore_insert_bsatn)
uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row_ptr, size_t* row_len_ptr);

STDB_IMPORT(table_id_from_name)
uint16_t table_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* id);

STDB_IMPORT(console_log)
void console_log(
    uint8_t level, const uint8_t* target_ptr, uint32_t target_len,
    const uint8_t* filename_ptr, uint32_t filename_len, uint32_t line_number,
    const uint8_t* message_ptr, uint32_t message_len);

}

// Simple BSATN writer without exceptions or vectors
class SimpleBsatnWriter {
private:
    uint8_t* buffer;
    size_t capacity;
    size_t position;
    
public:
    SimpleBsatnWriter(uint8_t* buf, size_t cap) : buffer(buf), capacity(cap), position(0) {}
    
    bool write_u8(uint8_t value) {
        if (position + 1 > capacity) return false;
        buffer[position++] = value;
        return true;
    }
    
    bool write_u32_le(uint32_t value) {
        if (position + 4 > capacity) return false;
        buffer[position++] = value & 0xFF;
        buffer[position++] = (value >> 8) & 0xFF;
        buffer[position++] = (value >> 16) & 0xFF;
        buffer[position++] = (value >> 24) & 0xFF;
        return true;
    }
    
    bool write_bytes(const uint8_t* data, size_t len) {
        if (position + len > capacity) return false;
        for (size_t i = 0; i < len; i++) {
            buffer[position++] = data[i];
        }
        return true;
    }
    
    bool write_string(const char* str) {
        size_t len = 0;
        while (str[len]) len++;
        if (!write_u32_le(len)) return false;
        return write_bytes((const uint8_t*)str, len);
    }
    
    size_t get_position() const { return position; }
    const uint8_t* get_buffer() const { return buffer; }
};

// Helper to write module description
inline void write_minimal_module_desc(uint32_t sink) {
    uint8_t buffer[1024];
    SimpleBsatnWriter writer(buffer, sizeof(buffer));
    
    // RawModuleDef::V9
    writer.write_u8(1);
    
    // Empty arrays for all fields
    writer.write_u32_le(0); // typespace
    writer.write_u32_le(0); // tables
    writer.write_u32_le(0); // reducers
    writer.write_u32_le(0); // types
    writer.write_u32_le(0); // misc_exports
    writer.write_u32_le(0); // row_level_security
    
    size_t len = writer.get_position();
    bytes_sink_write(sink, buffer, &len);
}

// Simple logging without iostream
inline void log_info(const char* message) {
    const char* target = "module";
    const char* filename = __FILE__;
    size_t msg_len = 0;
    while (message[msg_len]) msg_len++;
    size_t target_len = 6;
    size_t filename_len = 0;
    while (filename[filename_len]) filename_len++;
    
    console_log(2, // INFO level
        (const uint8_t*)target, target_len,
        (const uint8_t*)filename, filename_len,
        __LINE__,
        (const uint8_t*)message, msg_len);
}

// Minimal table registration
struct TableInfo {
    const char* name;
    uint32_t type_ref;
    bool is_public;
};

// Minimal reducer context
class MinimalReducerContext {
private:
    uint32_t args_source;
    uint32_t error_sink;
    
public:
    MinimalReducerContext(uint32_t args, uint32_t err) : args_source(args), error_sink(err) {}
    
    // Simple insert for a table (would need table ID lookup)
    template<typename T>
    bool insert(const char* table_name, const T& row) {
        // Get table ID
        uint32_t table_id;
        size_t name_len = 0;
        while (table_name[name_len]) name_len++;
        
        if (table_id_from_name((const uint8_t*)table_name, name_len, &table_id) != 0) {
            return false;
        }
        
        // Serialize row (simplified - assumes T has a serialize method)
        uint8_t buffer[1024];
        SimpleBsatnWriter writer(buffer, sizeof(buffer));
        if (!row.serialize(writer)) return false;
        
        // Insert
        size_t len = writer.get_position();
        return datastore_insert_bsatn(table_id, buffer, &len) == 0;
    }
    
    bool read_args(uint8_t* buffer, size_t* len) {
        return bytes_source_read(args_source, buffer, len) >= 0;
    }
    
    void write_error(const char* msg) {
        size_t len = 0;
        while (msg[len]) len++;
        bytes_sink_write(error_sink, (const uint8_t*)msg, &len);
    }
};

#endif // SPACETIMEDB_MINIMAL_H