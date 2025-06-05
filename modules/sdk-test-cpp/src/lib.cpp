// SpacetimeDB C++ Module demonstrating standard library usage with WASI shims
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdint>

// SpacetimeDB ABI imports
extern "C" {
uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
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

// Describe module - minimal module with one reducer
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    // Create a minimal module description
    // RawModuleDef::V9 (variant 1)
    uint8_t version = 1;
    write_to_sink(sink, &version, 1);
    
    // Empty typespace (0 types)
    uint32_t type_count = 0;
    write_to_sink(sink, (uint8_t*)&type_count, 4);
    
    // Empty tables (0 tables)
    uint32_t table_count = 0;
    write_to_sink(sink, (uint8_t*)&table_count, 4);
    
    // 1 reducer
    uint32_t reducer_count = 1;
    write_to_sink(sink, (uint8_t*)&reducer_count, 4);
    
    // Reducer: "no_op"
    // name length and name
    uint32_t name_len = 5;
    write_to_sink(sink, (uint8_t*)&name_len, 4);
    write_to_sink(sink, (uint8_t*)"no_op", 5);
    
    // params: empty ProductType (0 elements)
    uint32_t param_count = 0;
    write_to_sink(sink, (uint8_t*)&param_count, 4);
    
    // lifecycle: Some(UserDefined)
    uint8_t has_lifecycle = 1;
    write_to_sink(sink, &has_lifecycle, 1);
    uint8_t lifecycle = 0; // UserDefined
    write_to_sink(sink, &lifecycle, 1);
    
    // Empty arrays for the rest
    uint32_t empty = 0;
    write_to_sink(sink, (uint8_t*)&empty, 4); // types
    write_to_sink(sink, (uint8_t*)&empty, 4); // misc_exports
    write_to_sink(sink, (uint8_t*)&empty, 4); // row_level_security
}

// Handle reducer calls
__attribute__((export_name("__call_reducer__")))
int16_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    if (id == 0) {
        // Demonstrate various C++ standard library features
        
        // 1. Using std::string
        std::string base_message = "C++ Standard Library Demo:";
        
        // 2. Using std::vector
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        
        // 3. Using std::algorithm
        std::transform(numbers.begin(), numbers.end(), numbers.begin(), 
                      [](int n) { return n * n; });
        
        // 4. Using std::stringstream
        std::stringstream ss;
        ss << base_message << " squares = [";
        for (size_t i = 0; i < numbers.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << numbers[i];
        }
        ss << "]";
        
        // 5. Using std::cout (though it goes through our WASI shim)
        std::cout << "Debug: " << ss.str() << std::endl;
        
        // Get the final message
        std::string message = ss.str();
        
        // Log using SpacetimeDB's console_log
        console_log(2, // INFO level
                   (uint8_t*)"module", 6,
                   (uint8_t*)__FILE__, sizeof(__FILE__) - 1,
                   __LINE__,
                   (uint8_t*)message.c_str(), message.length());
        
        return 0; // Success
    }
    
    return -999; // No such reducer
}

} // extern "C"