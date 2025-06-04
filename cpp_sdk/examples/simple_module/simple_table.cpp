#include <cstdint>
#include <vector>
#include <string>

// Minimal BSATN writer
class MinimalWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t v) { buffer.push_back(v); }
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
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
};

// FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        MinimalWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace with 1 type - OneU8 struct
        w.write_u32_le(1);
        
        // Type 0: OneU8 struct
        w.write_u8(2);  // Product type
        w.write_u32_le(1);  // 1 field
        // Field: n (u8)
        w.write_u8(0);  // Some (field name present)
        w.write_string("n");
        w.write_u8(7); // U8 type
        
        // Tables: 1 table
        w.write_u32_le(1);
        
        // Table: one_u8
        w.write_string("one_u8");
        w.write_u32_le(0);  // product_type_ref = 0
        w.write_u32_le(0);  // primary_key = empty vec (not None)
        w.write_u32_le(0);  // indexes = empty vec
        w.write_u32_le(0);  // constraints = empty vec  
        w.write_u32_le(0);  // sequences = empty vec
        w.write_u8(1);      // schedule = None
        w.write_u8(1);      // table_type = User
        w.write_u8(0);      // table_access = Public
        
        // Empty reducers, types, misc exports, row level security
        w.write_u32_le(0);  // reducers
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
        return -1;
    }
}