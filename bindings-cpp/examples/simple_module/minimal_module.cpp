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
    
    __attribute__((import_module("spacetime_10.0"), import_name("_console_log")))
    void _console_log(uint8_t level, const uint8_t* target, size_t target_len,
                      const uint8_t* filename, size_t filename_len, uint32_t line_number,
                      const uint8_t* text, size_t text_len);
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        MinimalWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Empty typespace
        w.write_u32_le(0);
        
        // Empty tables
        w.write_u32_le(0);
        
        // Empty reducers
        w.write_u32_le(0);
        
        // Empty types
        w.write_u32_le(0);
        
        // Empty misc exports
        w.write_u32_le(0);
        
        // Empty row level security
        w.write_u32_le(0);
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        bytes_sink_write(sink, buffer.data(), &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id, 
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        // No reducers, so always return error
        return -1;
    }
}