// Basic SpacetimeDB module using only simple sdk_test types
#include <cstdint>
#include <cstring>
#include <string>

// Only include the basic types from sdk_test namespace without SDK dependencies
namespace sdk_test_cpp {
    enum class SimpleEnum : uint8_t {
        Zero,
        One,
        Two,
    };

    struct UnitStruct {
        // Empty struct
    };

    struct ByteStruct {
        uint8_t b;
    };

    struct OneU8Row {
        uint8_t n;
    };

    struct OneU16Row {
        uint16_t n;
    };

    struct OneU32Row {
        uint32_t n;
    };

    struct OneStringRow {
        std::string n;
    };
}

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
    uint32_t basic_test_counter = 0;
    
    // Required: Module description - use exact working pattern from demo_module
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Minimal valid RawModuleDef::V9 (exact copy from working demo_module)
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
    
    // Required: Reducer dispatcher - handles any reducer call and demos sdk_test types
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        const char* filename = "sdk_test_basic.cpp";
        basic_test_counter++;
        
        // Test the basic sdk_test types to verify they compile and work
        sdk_test_cpp::SimpleEnum simple_enum = sdk_test_cpp::SimpleEnum::Two;
        sdk_test_cpp::UnitStruct unit_struct;
        sdk_test_cpp::ByteStruct byte_struct;
        byte_struct.b = 255;
        
        sdk_test_cpp::OneU8Row u8_row;
        u8_row.n = 42;
        
        sdk_test_cpp::OneU16Row u16_row;
        u16_row.n = 1000;
        
        sdk_test_cpp::OneU32Row u32_row;
        u32_row.n = 2000000;
        
        sdk_test_cpp::OneStringRow string_row;
        string_row.n = "Basic types work!";
        
        // Create success message with counter
        char message[100];
        const char* base_msg = "SDK Test basic types demo #";
        int i = 0;
        while (base_msg[i] != '\0' && i < 50) {
            message[i] = base_msg[i];
            i++;
        }
        
        // Add counter value
        uint32_t counter = basic_test_counter;
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
        
        const char* suffix = " completed!";
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
        
        return 0; // success
    }
}