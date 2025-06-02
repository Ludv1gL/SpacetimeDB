#include <cstdint>
#include <cstring>
#include <vector>

// Iteration 5: Simplify and trace the exact BSATN format

extern "C" {
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
    
    void log_hex(const char* label, const std::vector<uint8_t>& data) {
        char hex_str[1024];
        int pos = 0;
        pos += snprintf(hex_str + pos, sizeof(hex_str) - pos, "%s: ", label);
        for (size_t i = 0; i < data.size() && i < 50; i++) {
            pos += snprintf(hex_str + pos, sizeof(hex_str) - pos, "%02x ", data[i]);
        }
        if (data.size() > 50) {
            pos += snprintf(hex_str + pos, sizeof(hex_str) - pos, "...");
        }
        
        const char* filename = "sdk_test_iter5.cpp";
        console_log(
            1, // info level
            nullptr, 0,
            (const uint8_t*)filename, strlen(filename),
            __LINE__,
            (const uint8_t*)hex_str, strlen(hex_str)
        );
    }
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        std::vector<uint8_t> data;
        
        // Let's build the exact minimal module that worked before
        data.push_back(1);  // RawModuleDef::V9
        
        // Empty typespace
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Empty tables
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Empty reducers
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Empty types
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Empty misc_exports
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Empty row_level_security
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        log_hex("Module data", data);
        
        size_t len = data.size();
        bytes_sink_write(sink, data.data(), &len);
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
        return 0;
    }
}