// Minimal SpacetimeDB C++ Module with one table
#include <cstdint>
#include <cstring>

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

// Module exports
extern "C" {

// Describe module - minimal module with one table
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    // Let me write it byte by byte to ensure correctness
    uint8_t module_def[] = {
        // RawModuleDef::V9 (variant 1)
        1,
        
        // Typespace with 1 type
        1, 0, 0, 0,  // u32: 1
        
        // Type 0: ProductType (variant 2)
        2,
        
        // ProductType with 1 element
        1, 0, 0, 0,  // u32: 1
        
        // Element 0: n (u8)
        0,  // Some
        // name length and name
        1, 0, 0, 0,  // u32: 1
        'n',
        // type: U8 (variant 7)
        7,
        
        // Tables: 1 table
        1, 0, 0, 0,  // u32: 1
        
        // Table: SimpleTable
        // name length and name
        11, 0, 0, 0,  // u32: 11
        'S', 'i', 'm', 'p', 'l', 'e', 'T', 'a', 'b', 'l', 'e',
        // product_type_ref
        0, 0, 0, 0,  // u32: 0
        // primary_key (empty)
        0, 0, 0, 0,  // u32: 0
        // indexes (empty)
        0, 0, 0, 0,  // u32: 0
        // constraints (empty)
        0, 0, 0, 0,  // u32: 0
        // sequences (empty)
        0, 0, 0, 0,  // u32: 0
        // schedule: None
        1,
        // table_type: User (1)
        1,
        // table_access: Public (0)
        0,
        
        // Reducers: 1 reducer
        1, 0, 0, 0,  // u32: 1
        
        // Reducer: test_table
        // name length and name
        10, 0, 0, 0,  // u32: 10
        't', 'e', 's', 't', '_', 't', 'a', 'b', 'l', 'e',
        // params: empty ProductType
        0, 0, 0, 0,  // u32: 0
        // lifecycle: None
        1,
        
        // Empty arrays for the rest
        0, 0, 0, 0,  // types
        0, 0, 0, 0,  // misc_exports
        0, 0, 0, 0   // row_level_security
    };
    
    write_to_sink(sink, module_def, sizeof(module_def));
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
    if (id == 0) { // test_table reducer
        const char* message = "Table test reducer called!";
        console_log(2, // INFO level
                   (const uint8_t*)"module", 6,
                   (const uint8_t*)__FILE__, strlen(__FILE__),
                   __LINE__,
                   (const uint8_t*)message, strlen(message));
        return 0;
    }
    
    return -999;
}

} // extern "C"