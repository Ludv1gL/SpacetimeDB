#include <cstdint>
#include <cstddef>

// Minimal SDK test without WASI dependencies
// This tests basic module functionality and ABI compatibility

extern "C" {
    // Import the minimal required functions from spacetime_10.0
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(
        uint8_t level,
        const uint8_t *target_ptr, uint32_t target_len,
        const uint8_t *filename_ptr, uint32_t filename_len, 
        uint32_t line_number,
        const uint8_t *message_ptr, uint32_t message_len
    );
    
    // Required module exports
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Create minimal RawModuleDef::V9 with basic table definition
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            
            // Tables (1 table)
            1, 0, 0, 0,     // tables vector length = 1
            
            // Table definition starts here
            8, 0, 0, 0,     // table name length = 8
            't', 'e', 's', 't', '_', 't', 'b', 'l',  // table name "test_tbl"
            
            // Product type with 1 field  
            1,              // ProductType tag = 1
            1, 0, 0, 0,     // elements length = 1
            
            // Field definition
            2, 0, 0, 0,     // field name length = 2  
            'i', 'd',       // field name "id"
            8,              // AlgebraicType::U32 = 8
            
            // Table properties
            1,              // is_public = true
            0,              // unique_id = 0
            0, 0, 0, 0,     // indexes (empty vector)
            0, 0, 0, 0,     // constraints (empty vector)
            0, 0, 0, 0,     // sequences (empty vector)
            
            0, 0, 0, 0,     // reducers (empty vector)
            0, 0, 0, 0,     // types (empty vector)
            0, 0, 0, 0,     // misc_exports (empty vector)
            0, 0, 0, 0      // row_level_security (empty vector)
        };
        
        size_t len = sizeof(data);
        bytes_sink_write(sink, data, &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        // Log a message to demonstrate functionality
        const char* target = "";
        const char* filename = "minimal_sdk_test.cpp";
        const char* message = "Reducer called successfully";
        
        console_log(
            1, // info level
            (const uint8_t*)target, 0,
            (const uint8_t*)filename, 21, // strlen("minimal_sdk_test.cpp") 
            42, // line number
            (const uint8_t*)message, 27 // strlen("Reducer called successfully")
        );
        
        return 0; // success
    }
}