#include <cstdint>
#include <cstddef>
#include <vector>

// Minimal RawModuleDef implementation
extern "C" {
    // Host ABI function that we need to call
    void _bytes_sink_write(uint32_t sink, const uint8_t* data, uint32_t len);
    
    // Our module export
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
        _bytes_sink_write(sink, data.data(), data.size());
    }
}