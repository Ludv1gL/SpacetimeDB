// Test module with only spacetimedb_abi.h included
#include "spacetimedb/abi/spacetimedb_abi.h"

// Required exports for SpacetimeDB module
extern "C" {
    // Module descriptor - minimal valid module
    STDB_EXPORT(__describe_module__)
    void __describe_module__(uint32_t description_sink) {
        // Minimal module descriptor V9 format:
        // tables array (empty) + reducers array (empty) + types array (empty) + 
        // misc_exports array (empty) + row_level_security array (empty)
        static const uint8_t module_def[] = {
            0x00, 0x00, 0x00, 0x00,  // tables: u32 length = 0
            0x00, 0x00, 0x00, 0x00,  // reducers: u32 length = 0
            0x00, 0x00, 0x00, 0x00,  // types: u32 length = 0
            0x00, 0x00, 0x00, 0x00,  // misc_exports: u32 length = 0
            0x00, 0x00, 0x00, 0x00   // row_level_security: u32 length = 0
        };
        
        // Write to the bytes sink
        size_t len = sizeof(module_def);
        bytes_sink_write(description_sink, module_def, &len);
    }
    
    // Call a reducer - required export
    STDB_EXPORT(__call_reducer__)
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args, 
        uint32_t error)
    {
        // No reducers in this module
        return -1; // Error: no such reducer
    }
}