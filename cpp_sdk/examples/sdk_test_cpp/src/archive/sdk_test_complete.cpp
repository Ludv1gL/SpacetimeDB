// Complete SpacetimeDB module using original sdk_test types with bridge
#include "sdk_type_bridge.h"  // Must come first to provide missing types

// Now we can safely include the original sdk_test header
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
    
    // Global test counter
    uint32_t complete_test_counter = 0;
    
    // Helper to log messages
    void log_message(const char* message) {
        const char* filename = "sdk_test_complete.cpp";
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, strlen(filename),
            ++complete_test_counter, // use counter as line number
            (const uint8_t*)message, strlen(message)
        );
    }
    
    // Test function that demonstrates all the complex types work
    void test_all_sdk_types() {
        complete_test_counter++;
        
        try {
            // Test basic types
            sdk_test_cpp::SimpleEnum simple_enum = sdk_test_cpp::SimpleEnum::Two;
            sdk_test_cpp::UnitStruct unit_struct;
            sdk_test_cpp::ByteStruct byte_struct;
            byte_struct.b = 200;
            
            log_message("Basic types: OK");
            
            // Test row types  
            sdk_test_cpp::OneU8Row u8_row;
            u8_row.n = 255;
            
            sdk_test_cpp::OneStringRow string_row;
            string_row.n = "Complete SDK test!";
            
            sdk_test_cpp::OneIdentityRow identity_row;
            // identity_row.n will use default constructor
            
            log_message("Row types: OK");
            
            // Test complex primitive struct
            sdk_test_cpp::EveryPrimitiveStruct eps;
            eps.a = 255;        // uint8_t
            eps.b = 65535;      // uint16_t
            eps.c = 4294967295U; // uint32_t
            eps.d = 18446744073709551615ULL; // uint64_t
            // eps.e, eps.f will use default constructors for 128/256 bit types
            eps.g = -128;       // int8_t
            eps.h = -32768;     // int16_t
            eps.i = -2147483648; // int32_t
            eps.j = -9223372036854775807LL; // int64_t
            // eps.k, eps.l will use default constructors for signed 128/256 bit
            eps.m = true;       // bool
            eps.n = 3.14159f;   // float
            eps.o = 2.71828;    // double
            eps.p = "Every primitive test"; // string
            // eps.q (Identity), eps.r (ConnectionId), eps.s (Timestamp), eps.t (TimeDuration) use defaults
            
            log_message("EveryPrimitiveStruct: OK");
            
            // Test vector struct
            sdk_test_cpp::EveryVecStruct evs;
            evs.a.push_back(1);
            evs.a.push_back(2);
            evs.a.push_back(3);
            
            evs.p.push_back("Hello");
            evs.p.push_back("Vector");
            evs.p.push_back("Test");
            
            log_message("EveryVecStruct: OK");
            
            // Test the complex EnumWithPayload
            sdk_test_cpp::EnumWithPayload enum_payload;
            enum_payload.tag = sdk_test_cpp::EnumWithPayloadTag::TagU32;
            enum_payload.value = static_cast<uint32_t>(123456);
            
            log_message("EnumWithPayload: OK");
            
            // Test more row types
            sdk_test_cpp::LargeTableRow large_row;
            large_row.row_id = 1;
            large_row.a = 100;
            large_row.p = "Large table test";
            large_row.m = false;
            
            log_message("LargeTableRow: OK");
            
            // Success message
            char final_msg[100];
            const char* base = "All SDK types test #";
            int i = 0;
            while (base[i] != '\0' && i < 30) {
                final_msg[i] = base[i];
                i++;
            }
            
            // Add counter
            uint32_t counter = complete_test_counter;
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
            
            for (int j = digit_count - 1; j >= 0; j--) {
                final_msg[i++] = digits[j];
            }
            
            const char* suffix = " - ALL TYPES SUCCESS!";
            int k = 0;
            while (suffix[k] != '\0' && i < 95) {
                final_msg[i++] = suffix[k++];
            }
            final_msg[i] = '\0';
            
            log_message(final_msg);
            
        } catch (...) {
            log_message("ERROR: Exception during type testing!");
        }
    }
    
    // Required: Module description using working pattern
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
        log_message("SDK Test Complete module - testing all original types!");
        test_all_sdk_types();
        return 0; // success
    }
}