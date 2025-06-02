// Simplified SpacetimeDB module using sdk_test types
#include "sdk_test.h"
#include <cstdint>
#include <cstring>

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(
        uint8_t level,
        const uint8_t *target_ptr, uint32_t target_len,
        const uint8_t *filename_ptr, uint32_t filename_len, 
        uint32_t line_number,
        const uint8_t *message_ptr, uint32_t message_len
    );
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // Global counter for demonstration
    uint32_t test_counter = 0;
    
    // Required: Module description using working pattern from demo_module
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Minimal valid RawModuleDef::V9 that works
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            0, 0, 0, 0,     // tables (empty vector)  
            2, 0, 0, 0,     // reducers (2 reducers)
            
            // Reducer 0: init
            4, 0, 0, 0,     // name length
            'i', 'n', 'i', 't', // name
            0, 0, 0, 0,     // params (empty vector)
            0,              // lifecycle flags
            
            // Reducer 1: test_sdk_types
            14, 0, 0, 0,    // name length
            't', 'e', 's', 't', '_', 's', 'd', 'k', '_', 't', 'y', 'p', 'e', 's', // name
            0, 0, 0, 0,     // params (empty vector)
            0,              // lifecycle flags
            
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
        const char* filename = "sdk_test_simple.cpp";
        
        if (id == 0) { // init reducer
            const char* message = "SDK Test module with complex types initialized!";
            console_log(
                1, // info level
                (const uint8_t*)"", 0,
                (const uint8_t*)filename, strlen(filename),
                50, // line number
                (const uint8_t*)message, strlen(message)
            );
        } else if (id == 1) { // test_sdk_types reducer
            // Demonstrate that sdk_test types compile and link correctly
            test_counter++;
            
            // Create instances of various sdk_test types
            sdk_test_cpp::SimpleEnum simple_enum = sdk_test_cpp::SimpleEnum::Two;
            sdk_test_cpp::UnitStruct unit_struct;
            sdk_test_cpp::ByteStruct byte_struct;
            byte_struct.b = 123;
            
            sdk_test_cpp::OneU8Row u8_row;
            u8_row.n = 42;
            
            sdk_test_cpp::OneStringRow string_row;
            string_row.n = "Hello SDK Test!";
            
            // Create more complex types
            sdk_test_cpp::EveryPrimitiveStruct eps;
            eps.a = 255;
            eps.b = 65535;
            eps.c = 4294967295U;
            eps.d = 18446744073709551615ULL;
            eps.m = true;
            eps.n = 3.14f;
            eps.o = 2.718281828;
            eps.p = "Complex struct test";
            
            // Test EnumWithPayload with manual construction
            sdk_test_cpp::EnumWithPayload enum_payload;
            enum_payload.tag = sdk_test_cpp::EnumWithPayloadTag::TagU32;
            enum_payload.value = static_cast<uint32_t>(12345);
            
            // Create success message with counter
            char message[100];
            const char* base_msg = "SDK types test #";
            int i = 0;
            while (base_msg[i] != '\0' && i < 50) {
                message[i] = base_msg[i];
                i++;
            }
            
            // Add counter value
            uint32_t counter = test_counter;
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
            
            const char* suffix = " completed successfully!";
            int k = 0;
            while (suffix[k] != '\0' && i < 95) {
                message[i++] = suffix[k++];
            }
            message[i] = '\0';
            
            console_log(
                1, // info level
                (const uint8_t*)"", 0,
                (const uint8_t*)filename, strlen(filename),
                95, // line number
                (const uint8_t*)message, i
            );
        }
        
        return 0; // success
    }
}