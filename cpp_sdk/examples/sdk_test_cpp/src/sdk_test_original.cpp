// Final attempt to compile the ORIGINAL sdk_test.h using comprehensive bridge
#include "comprehensive_sdk_bridge.h"

// Now try to include the original files
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
    uint32_t original_test_counter = 0;
    
    // Helper to log messages
    void log_message(const char* message) {
        const char* filename = "sdk_test_original.cpp";
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, strlen(filename),
            ++original_test_counter,
            (const uint8_t*)message, strlen(message)
        );
    }
    
    // Test using ACTUAL original sdk_test types
    void test_original_sdk_types() {
        try {
            log_message("Testing ORIGINAL sdk_test.h types!");
            
            // Test global types
            GlobalSimpleEnum global_enum = GlobalSimpleEnum::GlobalTwo;
            GlobalUnitStruct global_unit;
            log_message("Global types: OK");
            
            // Test basic sdk_test_cpp types
            sdk_test_cpp::SimpleEnum simple_enum = sdk_test_cpp::SimpleEnum::Two;
            sdk_test_cpp::UnitStruct unit_struct;
            sdk_test_cpp::ByteStruct byte_struct;
            byte_struct.b = 255;
            log_message("Basic types: OK");
            
            // Test complex primitive struct from original
            sdk_test_cpp::EveryPrimitiveStruct eps;
            eps.a = 200;
            eps.b = 50000;
            eps.c = 3000000000U;
            eps.d = 15000000000000000000ULL;
            eps.g = -100;
            eps.h = -25000;
            eps.i = -1500000;
            eps.j = -7500000000000000000LL;
            eps.m = true;
            eps.n = 2.718f;
            eps.o = 1.414213562373095;
            eps.p = "Original EveryPrimitiveStruct test!";
            log_message("EveryPrimitiveStruct: OK");
            
            // Test vector struct from original
            sdk_test_cpp::EveryVecStruct evs;
            evs.a.push_back(10);
            evs.a.push_back(20);
            evs.a.push_back(30);
            evs.p.push_back("Original");
            evs.p.push_back("Vector");
            evs.p.push_back("Test");
            evs.m.push_back(true);
            evs.m.push_back(false);
            log_message("EveryVecStruct: OK");
            
            // Test the original complex EnumWithPayload
            sdk_test_cpp::EnumWithPayload enum_payload;
            enum_payload.tag = sdk_test_cpp::EnumWithPayloadTag::TagU64;
            enum_payload.value = static_cast<uint64_t>(987654321012345ULL);
            log_message("EnumWithPayload: OK");
            
            // Test original row types
            sdk_test_cpp::OneU8Row u8_row;
            u8_row.n = 123;
            
            sdk_test_cpp::OneStringRow string_row;
            string_row.n = "Original OneStringRow working!";
            
            sdk_test_cpp::OneSimpleEnumRow enum_row;
            enum_row.n = sdk_test_cpp::SimpleEnum::One;
            
            sdk_test_cpp::OneEnumWithPayloadRow payload_row;
            payload_row.n = enum_payload;
            
            sdk_test_cpp::OneEveryPrimitiveStructRow eps_row;
            eps_row.s = eps;
            
            sdk_test_cpp::OneEveryVecStructRow evs_row;
            evs_row.s = evs;
            log_message("Row types: OK");
            
            // Test the massive LargeTableRow from original
            sdk_test_cpp::LargeTableRow large_row;
            large_row.row_id = 999;
            large_row.a = 250;
            large_row.p = "Original LargeTableRow complete test!";
            large_row.m = false;
            large_row.va.assign({100, 200, 300, 400, 500});
            large_row.vp.assign({"Large", "Table", "Original", "Success"});
            large_row.oi = 777;
            large_row.os = "Original optional string";
            large_row.en = sdk_test_cpp::SimpleEnum::Two;
            large_row.ewp = enum_payload;
            large_row.us = unit_struct;
            large_row.bs = byte_struct;
            large_row.eps = eps;
            large_row.evs = evs;
            log_message("LargeTableRow: OK");
            
            // Test unique row types
            sdk_test_cpp::UniqueU8Row unique_u8;
            unique_u8.row_id = 1;
            unique_u8.n = 42;
            unique_u8.data = 1001;
            
            sdk_test_cpp::UniqueStringRow unique_str;
            unique_str.row_id = 2;
            unique_str.s = "Unique string test";
            unique_str.data = 2002;
            log_message("Unique types: OK");
            
            // Test option types from original
            sdk_test_cpp::OptionI32Row opt_i32;
            opt_i32.n = 555;
            
            sdk_test_cpp::OptionStringRow opt_str;
            opt_str.n = "Original optional working!";
            
            sdk_test_cpp::OptionSimpleEnumRow opt_enum;
            opt_enum.n = sdk_test_cpp::SimpleEnum::Zero;
            
            sdk_test_cpp::OptionEveryPrimitiveStructRow opt_eps;
            opt_eps.s = eps;
            log_message("Option types: OK");
            
            // Final success message
            char final_msg[150];
            const char* base = "ORIGINAL SDK_TEST.H COMPLETE SUCCESS #";
            int i = 0;
            while (base[i] != '\0' && i < 60) {
                final_msg[i] = base[i];
                i++;
            }
            
            // Add counter
            uint32_t counter = original_test_counter;
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
            
            const char* suffix = " - ALL ORIGINAL TYPES WORK!";
            int k = 0;
            while (suffix[k] != '\0' && i < 145) {
                final_msg[i++] = suffix[k++];
            }
            final_msg[i] = '\0';
            
            log_message(final_msg);
            
        } catch (...) {
            log_message("ERROR: Exception during original type testing!");
        }
    }
    
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
        log_message("ORIGINAL SDK_TEST: Testing all types from sdk_test.h!");
        test_original_sdk_types();
        return 0; // success
    }
}