#include <cstdint>
#include <cstddef>

// Demo C++ SpacetimeDB module showcasing successful ABI integration
extern "C" {
    // Core SpacetimeDB ABI functions - all using correct import names
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
    
    // Demo: Multiple reducer functionality
    uint32_t demo_counter = 0;
    
    // Required: Module description
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Minimal valid RawModuleDef::V9 
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
    
    // Required: Reducer dispatcher
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        const char* filename = "demo_module.cpp";
        
        // Increment our counter
        demo_counter++;
        
        // Create a message with the counter value
        char message[64];
        const char* base_msg = "Demo reducer called #";
        
        // Simple string concatenation (avoiding sprintf to stay WASI-free)
        int i = 0;
        while (base_msg[i] != '\0' && i < 40) {
            message[i] = base_msg[i];
            i++;
        }
        
        // Add counter value (simple decimal conversion)
        uint32_t counter = demo_counter;
        char digits[10];
        int digit_count = 0;
        
        if (counter == 0) {
            digits[0] = '0';
            digit_count = 1;
        } else {
            while (counter > 0) {
                digits[digit_count] = '0' + (counter % 10);
                counter /= 10;
                digit_count++;
            }
        }
        
        // Reverse digits and add to message
        for (int j = digit_count - 1; j >= 0; j--) {
            message[i++] = digits[j];
        }
        message[i] = '\0';
        
        // Log the call
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, 15, // strlen("demo_module.cpp")
            60, // line number
            (const uint8_t*)message, i
        );
        
        // Demonstrate identity function usage
        uint8_t identity_buffer[32];
        identity(identity_buffer);
        
        // Log identity result (first 4 bytes as hex-like)
        char id_msg[40] = "Identity bytes: ";
        int msg_len = 16;
        for (int j = 0; j < 4; j++) {
            uint8_t byte = identity_buffer[j];
            uint8_t high = (byte >> 4) & 0xF;
            uint8_t low = byte & 0xF;
            
            id_msg[msg_len++] = (high < 10) ? ('0' + high) : ('A' + high - 10);
            id_msg[msg_len++] = (low < 10) ? ('0' + low) : ('A' + low - 10);
            if (j < 3) id_msg[msg_len++] = ' ';
        }
        id_msg[msg_len] = '\0';
        
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, 15,
            85, // line number
            (const uint8_t*)id_msg, msg_len
        );
        
        return 0; // success
    }
}