#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

// Minimal BSATN writer
class MinimalWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t v) { buffer.push_back(v); }
    void write_u16_le(uint16_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
    }
    void write_u32_le(uint32_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
        buffer.push_back((v >> 16) & 0xFF);
        buffer.push_back((v >> 24) & 0xFF);
    }
    void write_string(const std::string& s) {
        write_u32_le(s.length());
        for (char c : s) buffer.push_back(c);
    }
    void write_bytes(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            buffer.push_back(data[i]);
        }
    }
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
};

// FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    int16_t bytes_source_read(uint32_t source, uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row, size_t* row_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
}

// Simple logging
void log_info(const std::string& msg) {
    const char* filename = "working_module.cpp";
    console_log(2, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg.c_str(), msg.length());
}

// Module data
std::vector<void(*)(uint32_t, uint32_t)> g_reducers;

// Simple table: OneU8
void insert_one_u8(uint32_t args, uint32_t error) {
    log_info("insert_one_u8 called");
    
    // Read argument - expecting single u8
    uint8_t value = 0;
    size_t len = 1;
    bytes_source_read(args, &value, &len);
    
    // Get table ID
    uint32_t table_id = 0;
    const char* table_name = "one_u8";
    if (table_id_from_name((const uint8_t*)table_name, strlen(table_name), &table_id) != 0) {
        log_info("Failed to get table ID");
        return;
    }
    
    // Serialize row
    MinimalWriter w;
    w.write_u8(value);  // The n field
    
    auto buffer = w.take_buffer();
    size_t insert_len = buffer.size();
    
    // Insert into table
    uint16_t err = datastore_insert_bsatn(table_id, buffer.data(), &insert_len);
    if (err == 0) {
        log_info("Successfully inserted value");
    } else {
        log_info("Failed to insert value");
    }
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        MinimalWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace with 1 type
        w.write_u32_le(1);
        
        // Type 0: OneU8 struct
        w.write_u8(2);  // Product type
        w.write_u32_le(1);  // 1 field
        // Field: n
        w.write_u8(0);  // Some (field name present)
        w.write_string("n");
        w.write_u8(7); // U8 type
        
        // Tables: 1 table
        w.write_u32_le(1);
        
        // Table: one_u8
        w.write_string("one_u8");
        w.write_u32_le(0);  // product_type_ref = 0 (OneU8)
        
        // primary_key = empty vec
        w.write_u32_le(0);  // empty vec
        
        // indexes = empty
        w.write_u32_le(0);
        
        // constraints = empty
        w.write_u32_le(0);
        
        // sequences = empty
        w.write_u32_le(0);
        
        // schedule = None
        w.write_u8(1);  // None
        
        // table_type = User
        w.write_u8(1);
        
        // table_access = Public
        w.write_u8(0);
        
        // Reducers: 1 reducer
        w.write_u32_le(1);
        
        // Reducer: insert_one_u8
        w.write_string("insert_one_u8");
        
        // params: 1 param
        w.write_u32_le(1);
        // param 0: n (u8)
        w.write_u8(0);  // Some
        w.write_string("n");
        w.write_u8(7); // U8
        
        // lifecycle = None
        w.write_u8(1);
        
        // Empty types, misc exports, row level security
        w.write_u32_le(0);  // types
        w.write_u32_le(0);  // misc_exports  
        w.write_u32_le(0);  // row_level_security
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        bytes_sink_write(sink, buffer.data(), &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id, 
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        if (id == 0) {
            insert_one_u8(args, error);
            return 0;
        }
        return -1;
    }
}