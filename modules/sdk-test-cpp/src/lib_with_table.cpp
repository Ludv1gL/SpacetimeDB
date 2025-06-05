// SpacetimeDB C++ Module with Table
#include <cstdint>
#include <vector>
#include <string>

// Import functions from SpacetimeDB
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
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

// Module exports
extern "C" {

// Describe module - module with one table and one reducer
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    // RawModuleDef::V9 (variant 1)
    write_u8(sink, 1);
    
    // Typespace with 1 type (for our table)
    write_u32_le(sink, 1);
    
    // Type 0: ProductType for User table
    // AlgebraicType::Product (variant 2)
    write_u8(sink, 2);
    
    // ProductType with 2 elements
    write_u32_le(sink, 2);
    
    // Element 0: id (u32)
    // name: Some("id")
    write_u8(sink, 0); // Some
    write_string(sink, "id");
    // type: AlgebraicType::U32 (variant 9)
    write_u8(sink, 9);
    
    // Element 1: name (String)
    // name: Some("name")
    write_u8(sink, 0); // Some
    write_string(sink, "name");
    // type: AlgebraicType::String (variant 17)
    write_u8(sink, 17);
    
    // 1 table
    write_u32_le(sink, 1);
    
    // Table: User
    // name
    write_string(sink, "User");
    // product_type_ref: type 0
    write_u32_le(sink, 0);
    // primary_key: [0] (column 0 is primary key)
    write_u32_le(sink, 1); // 1 column in primary key
    write_u32_le(sink, 0); // column 0
    // indexes: empty
    write_u32_le(sink, 0);
    // constraints: empty
    write_u32_le(sink, 0);
    // sequences: empty
    write_u32_le(sink, 0);
    // schedule: None
    write_u8(sink, 1); // None
    // table_type: User (1)
    write_u8(sink, 1);
    // table_access: Public (0)
    write_u8(sink, 0);
    
    // 1 reducer
    write_u32_le(sink, 1);
    
    // Reducer: "add_user"
    write_string(sink, "add_user");
    
    // params: ProductType with 2 elements
    write_u32_le(sink, 2);
    // Element 0: id (u32)
    write_u8(sink, 0); // Some
    write_string(sink, "id");
    write_u8(sink, 9); // U32
    // Element 1: name (String)
    write_u8(sink, 0); // Some
    write_string(sink, "name");
    write_u8(sink, 17); // String
    
    // lifecycle: None
    write_u8(sink, 1); // None
    
    // Empty arrays for the rest
    write_u32_le(sink, 0); // types
    write_u32_le(sink, 0); // misc_exports
    write_u32_le(sink, 0); // row_level_security
}

// Handle reducer calls
__attribute__((export_name("__call_reducer__")))
int32_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    if (id == 0) { // add_user reducer
        std::string message = "add_user reducer called!";
        console_log(2, // INFO level
                   reinterpret_cast<const uint8_t*>("module"), 6,
                   reinterpret_cast<const uint8_t*>(__FILE__), sizeof(__FILE__) - 1,
                   __LINE__,
                   reinterpret_cast<const uint8_t*>(message.c_str()), message.length());
        
        // TODO: Actually insert into the table
        return 0; // Success
    }
    
    return -999; // No such reducer
}

} // extern "C"