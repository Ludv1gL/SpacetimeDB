/**
 * Test module using the refactored centralized ABI
 */

#include "spacetimedb/abi/spacetimedb_abi.h"
#include <string>
#include <vector>
#include <cstring>

// BSATN writer for module description
class BsatnWriter {
    std::vector<uint8_t> buffer;
    
public:
    void write_u8(uint8_t val) {
        buffer.push_back(val);
    }
    
    void write_u32_le(uint32_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back((val >> 16) & 0xFF);
        buffer.push_back((val >> 24) & 0xFF);
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.length());
        for (char c : str) {
            buffer.push_back(static_cast<uint8_t>(c));
        }
    }
    
    void write_vec_len(size_t len) {
        write_u32_le(static_cast<uint32_t>(len));
    }
    
    std::vector<uint8_t>& get_buffer() { return buffer; }
};

// Forward declarations
extern "C" {
    void test_log(uint32_t source, uint32_t sink);
}

// Module exports using centralized ABI
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        BsatnWriter w;
        
        // RawModuleDef::V9 with minimal content
        w.write_u8(1);
        
        // Typespace: types vec (empty)
        w.write_vec_len(0);
        
        // Typespace: names vec (empty)
        w.write_vec_len(0);
        
        // Tables (empty)
        w.write_vec_len(0);
        
        // Reducers (1 reducer: test_log)
        w.write_vec_len(1);
        
        w.write_string("test_log");
        w.write_u8(2); // Product type
        w.write_vec_len(1); // 1 param
        
        // Param: message (String)
        w.write_u8(0); // Some
        w.write_string("message");
        w.write_u8(15); // String
        
        w.write_u8(1); // None (no lifecycle)
        
        // types: Vec<RawTypeDefV9> (empty)
        w.write_vec_len(0);
        
        // misc_exports (empty)
        w.write_vec_len(0);
        
        // row_level_security (empty)
        w.write_vec_len(0);
        
        // Write to sink using centralized ABI
        auto& data = w.get_buffer();
        size_t len = data.size();
        uint16_t result = bytes_sink_write(sink, data.data(), &len);
        
        // Log result using centralized logging
        std::string log_msg = "Module description written: " + std::to_string(len) + " bytes, result: " + std::to_string(result);
        console_log(2, nullptr, 0, 
                   reinterpret_cast<const uint8_t*>("refactored_test.cpp"), 20,
                   __LINE__,
                   reinterpret_cast<const uint8_t*>(log_msg.c_str()), log_msg.length());
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t reducer_id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp_us,
        uint32_t args_source,
        uint32_t error_sink
    ) {
        if (reducer_id == 0) { // test_log
            test_log(args_source, error_sink);
            return 0;
        }
        
        // Unknown reducer
        std::string error_msg = "Unknown reducer ID: " + std::to_string(reducer_id);
        console_log(0, nullptr, 0,
                   reinterpret_cast<const uint8_t*>("refactored_test.cpp"), 20,
                   __LINE__,
                   reinterpret_cast<const uint8_t*>(error_msg.c_str()), error_msg.length());
        return -1;
    }
    
    __attribute__((export_name("test_log")))
    void test_log(uint32_t source, uint32_t sink) {
        // Read message argument
        uint8_t args[1024];
        size_t args_len = sizeof(args);
        int16_t result = bytes_source_read(source, args, &args_len);
        
        if (result < 0) {
            std::string error_msg = "Failed to read arguments: " + std::to_string(result);
            console_log(0, nullptr, 0,
                       reinterpret_cast<const uint8_t*>("refactored_test.cpp"), 20,
                       __LINE__,
                       reinterpret_cast<const uint8_t*>(error_msg.c_str()), error_msg.length());
            return;
        }
        
        // Simple message parsing (expecting string)
        if (args_len < 4) {
            console_log(0, nullptr, 0,
                       reinterpret_cast<const uint8_t*>("refactored_test.cpp"), 20,
                       __LINE__,
                       reinterpret_cast<const uint8_t*>("Arguments too short"), 19);
            return;
        }
        
        uint32_t msg_len = args[0] | (args[1] << 8) | (args[2] << 16) | (args[3] << 24);
        
        if (args_len < 4 + msg_len) {
            console_log(0, nullptr, 0,
                       reinterpret_cast<const uint8_t*>("refactored_test.cpp"), 20,
                       __LINE__,
                       reinterpret_cast<const uint8_t*>("Message too short"), 18);
            return;
        }
        
        std::string message(reinterpret_cast<char*>(&args[4]), msg_len);
        
        // Log the message using centralized console_log
        std::string log_msg = "test_log called with message: " + message;
        console_log(2, nullptr, 0,
                   reinterpret_cast<const uint8_t*>("refactored_test.cpp"), 20,
                   __LINE__,
                   reinterpret_cast<const uint8_t*>(log_msg.c_str()), log_msg.length());
    }
}