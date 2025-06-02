#include <cstdint>
#include <cstddef>

// Working enhanced C++ SDK test that successfully compiles and publishes
extern "C" {
    // Import core SpacetimeDB functions from spacetime_10.0 that we know work
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
    
    __attribute__((import_module("spacetime_10.0"), import_name("identity")))
    void identity(uint8_t* out_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* out);
    
    // Module exports
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Create minimal but correctly formatted RawModuleDef::V9
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            0, 0, 0, 0,     // tables (empty vector)  
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
        // Demonstrate working C++ SDK functionality
        const char* filename = "working_enhanced_test.cpp";
        const char* message = "Enhanced C++ reducer called successfully!";
        
        // Log the successful call
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, 25, // strlen("working_enhanced_test.cpp") 
            80, // line number
            (const uint8_t*)message, 41 // strlen("Enhanced C++ reducer called successfully!")
        );
        
        // Demonstrate identity function
        uint8_t identity_buffer[32];
        identity(identity_buffer);
        
        // Try to get a table ID (this will fail since no tables exist, but demonstrates the API)
        const char* table_name = "NonExistentTable";
        uint32_t table_id = 0;
        uint16_t status = table_id_from_name(
            (const uint8_t*)table_name, 
            16, // strlen("NonExistentTable")
            &table_id
        );
        
        // Log the result
        if (status == 0) {
            console_log(1, (const uint8_t*)"", 0, (const uint8_t*)filename, 25, 95,
                       (const uint8_t*)"Table found unexpectedly", 24);
        } else {
            console_log(1, (const uint8_t*)"", 0, (const uint8_t*)filename, 25, 97,
                       (const uint8_t*)"Table not found as expected", 27);
        }
        
        return 0; // success
    }
}