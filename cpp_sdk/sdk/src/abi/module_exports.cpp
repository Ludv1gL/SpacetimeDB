#include "spacetimedb/abi/spacetimedb_abi.h"
#include "spacetimedb/internal/Module.h"  // Use new Module API
#include "spacetimedb/types.h"     // For Timestamp

#include <vector>
#include <cstddef> // For size_t
#include <string>  // For std::string in error handling
#include <iostream> // For temporary error logging if needed

// Note: SPACETIMEDB_WASM_EXPORT is applied in the header "spacetime_module_exports.h"

extern "C" {

    void __describe_module__(uint32_t description_sink_handle) {
        // Use the new Module API
        SpacetimeDb::Internal::Module::__describe_module__(description_sink_handle);
    }

    int16_t __call_reducer__(
        uint32_t reducer_id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp_us,
        uint32_t args,
        uint32_t error
    ) {
        // Create timestamp
        SpacetimeDb::sdk::Timestamp ts;
        ts.microseconds_since_epoch = timestamp_us;
        
        // Call Module's implementation
        auto result = SpacetimeDb::Internal::Module::__call_reducer__(
            reducer_id,
            sender_0, sender_1, sender_2, sender_3,
            conn_id_0, conn_id_1,
            ts,
            args,
            error
        );
        
        // Convert Errno to int16_t
        switch (result) {
            case SpacetimeDb::Internal::FFI::Errno::OK:
                return 0;
            case SpacetimeDb::Internal::FFI::Errno::NO_SUCH_REDUCER:
                return -1;
            case SpacetimeDb::Internal::FFI::Errno::HOST_CALL_FAILURE:
                return -3;
            default:
                return -4;
        }
    }

} // extern "C"
