// Minimal SpacetimeDB C++ module
#include <cstdint>
#include <vector>

// FFI exports required by SpacetimeDB
extern "C" {
    // Module description export
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t description) {
        // For now, return an empty module definition
        // This would normally serialize the module schema
    }
    
    // Reducer call handler
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp,
        uint32_t args,
        uint32_t error
    ) {
        // No reducers defined yet
        return -1; // Error: reducer not found
    }
}