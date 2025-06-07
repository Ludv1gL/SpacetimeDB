// Absolute minimal manual module
#include <cstdint>
#include <cstddef>

#define SPACETIMEDB_WASM_EXPORT extern "C" __attribute__((visibility("default")))

// Manual buffer functions
extern "C" {
    void spacetime_buffer_consume(uint32_t buffer_handle, const uint8_t* data, size_t len);
    uint8_t* spacetime_buffer_len(uint32_t buffer_handle, uint32_t* len);
}

SPACETIMEDB_WASM_EXPORT
void __describe_module__(uint32_t description) {
    // Minimal module definition - just empty module
    uint8_t module_def[] = {
        // typespace: Typespace
        0,    // typespace.types = empty Option
        0, 0, 0, 0,  // types vec length = 0
        
        // types: Vec<RawTypeDefV9>
        0, 0, 0, 0,  // empty vec
        
        // tables: Vec<RawTableDefV9>
        0, 0, 0, 0,  // empty vec
        
        // reducers: Vec<RawReducerDefV9>
        0, 0, 0, 0,  // empty vec
        
        // misc_exports: Vec<RawMiscModuleExportV9>
        0, 0, 0, 0,  // empty vec
        
        // row_level_security: Vec<RawRowLevelSecurityDefV9>
        0, 0, 0, 0   // empty vec
    };
    
    spacetime_buffer_consume(description, module_def, sizeof(module_def));
}

SPACETIMEDB_WASM_EXPORT
int16_t __call_reducer__(
    uint32_t reducer_id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp,
    uint32_t args,
    uint32_t error
) {
    return -1; // No reducers
}