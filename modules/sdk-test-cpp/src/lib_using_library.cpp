// SpacetimeDB C++ Module using Module Library
#include <cstdint>
#include <string>

// Define a simple User struct
struct User {
    uint32_t id;
    std::string name;
};

// Module exports using manual registration
extern "C" {
    // Import functions from SpacetimeDB
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_row_insert")))
    uint16_t table_row_insert(uint32_t table_id, const uint8_t* row_ptr, size_t row_len);
    
    // Module description - manual implementation
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink);
    
    // Reducer calls
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args, 
        uint32_t error
    );
}

// Helper to write to bytes sink
void write_to_sink(uint32_t sink, const uint8_t* data, size_t len) {
    size_t written = len;
    bytes_sink_write(sink, data, &written);
}

// Helper to write a u8
void write_u8(uint32_t sink, uint8_t value) {
    write_to_sink(sink, &value, 1);
}

// Helper to write a u32 in little-endian
void write_u32_le(uint32_t sink, uint32_t value) {
    uint8_t bytes[4] = {
        static_cast<uint8_t>(value & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 24) & 0xFF)
    };
    write_to_sink(sink, bytes, 4);
}

// Helper to write a string
void write_string(uint32_t sink, const std::string& str) {
    write_u32_le(sink, static_cast<uint32_t>(str.length()));
    write_to_sink(sink, reinterpret_cast<const uint8_t*>(str.data()), str.length());
}

// Module description implementation
void __describe_module__(uint32_t sink) {
    // Let me copy the exact format from the working minimal_module.cpp
    
    // RawModuleDef::V9 tag
    write_u8(sink, 1);
    
    // Empty typespace
    write_u32_le(sink, 0);
    
    // Empty tables
    write_u32_le(sink, 0);
    
    // 1 reducer
    write_u32_le(sink, 1);
    
    // Reducer: "test_library"
    write_string(sink, "test_library");
    
    // params: empty ProductType (0 elements)
    write_u32_le(sink, 0);
    
    // lifecycle: None
    write_u8(sink, 1); // None
    
    // Empty arrays for the rest
    write_u32_le(sink, 0); // types
    write_u32_le(sink, 0); // misc_exports
    write_u32_le(sink, 0); // row_level_security
}

// Reducer implementation
int32_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    if (id == 0) { // test_library reducer
        std::string message = "Testing module library integration!";
        console_log(2, // INFO level
                   reinterpret_cast<const uint8_t*>("module"), 6,
                   reinterpret_cast<const uint8_t*>(__FILE__), sizeof(__FILE__) - 1,
                   __LINE__,
                   reinterpret_cast<const uint8_t*>(message.c_str()), message.length());
        return 0; // Success
    }
    
    return -999; // No such reducer
}