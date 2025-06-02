#include <cstdint>
#include <cstddef>
#include <vector>

// Minimal RawModuleDef implementation
extern "C" {
    // Host ABI function that we need to call - note the correct import module and function name
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // Required module exports
    void __describe_module__(uint32_t sink) {
        // Create minimal RawModuleDef::V9 with empty contents
        std::vector<uint8_t> data;
        
        // RawModuleDef enum: variant V9 = 1
        data.push_back(1);
        
        // RawModuleDefV9 fields (all empty vectors):
        // 1. typespace (empty vector of AlgebraicType)
        data.push_back(0); data.push_back(0); data.push_back(0); data.push_back(0); // u32 length = 0
        
        // 2. tables (empty vector)
        data.push_back(0); data.push_back(0); data.push_back(0); data.push_back(0); // u32 length = 0
        
        // 3. reducers (empty vector)  
        data.push_back(0); data.push_back(0); data.push_back(0); data.push_back(0); // u32 length = 0
        
        // 4. types (empty vector)
        data.push_back(0); data.push_back(0); data.push_back(0); data.push_back(0); // u32 length = 0
        
        // 5. misc_exports (empty vector)
        data.push_back(0); data.push_back(0); data.push_back(0); data.push_back(0); // u32 length = 0
        
        // 6. row_level_security (empty vector)
        data.push_back(0); data.push_back(0); data.push_back(0); data.push_back(0); // u32 length = 0
        
        // Write to sink
        size_t len = data.size();
        bytes_sink_write(sink, data.data(), &len);
    }
    
    // Required reducer function (even if empty)
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        // No reducers in minimal test, just return success
        return 0;
    }
}