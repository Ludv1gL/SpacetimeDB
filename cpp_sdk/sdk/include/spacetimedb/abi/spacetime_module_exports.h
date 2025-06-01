#ifndef SPACETIMEDB_MODULE_EXPORTS_H
#define SPACETIMEDB_MODULE_EXPORTS_H

#include "common_defs.h" // For BytesSink, BytesSource, Status etc.
#include <cstdint>

// This header defines the C-style ABI functions that a SpacetimeDB C++ Wasm module
// *exports* for the SpacetimeDB runtime to call.

#ifndef SPACETIMEDB_WASM_EXPORT
    #ifdef __EMSCRIPTEN__
        #include <emscripten.h>
        #define SPACETIMEDB_WASM_EXPORT EMSCRIPTEN_KEEPALIVE
    #else
        #define SPACETIMEDB_WASM_EXPORT
    #endif
#endif // SPACETIMEDB_WASM_EXPORT


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Provides the BSATN-serialized ModuleDef to the host.
 * @details The module calls host function `bytes_sink_write` on `description_sink_handle`
 *          to transfer the serialized module definition.
 * @param description_sink_handle An opaque handle provided by the host, representing a buffer
 *                                the module can write into.
 * @ingroup module_abi_exports
 */
SPACETIMEDB_WASM_EXPORT void __describe_module__(BytesSink description_sink_handle);

/**
 * @brief Called by the host to execute a specific reducer within the module.
 * @details The module reads arguments from `args_source_handle`, calls the appropriate
 *          C++ reducer function, and writes any error messages to `error_sink_handle`.
 *
 * @param reducer_id A numeric identifier for the reducer to be called. The module
 *                   is responsible for mapping this ID to its internal reducer representation.
 * @param sender_identity_p0 Part 0 of the sender's identity (uint64_t).
 * @param sender_identity_p1 Part 1 of the sender's identity (uint64_t).
 * @param sender_identity_p2 Part 2 of the sender's identity (uint64_t).
 * @param sender_identity_p3 Part 3 of the sender's identity (uint64_t).
 * @param connection_id_p0 Part 0 of the connection ID (uint64_t). (If applicable, may be 0)
 * @param connection_id_p1 Part 1 of the connection ID (uint64_t). (If applicable, may be 0)
 * @param timestamp The host-provided timestamp for the event.
 * @param args_source_handle An opaque handle from which BSATN-serialized reducer arguments can be read.
 * @param error_sink_handle An opaque handle to which BSATN-serialized error messages (strings) can be written if execution fails.
 * @return A Status code (as int16_t for ABI): 0 for success, non-zero for errors (e.g., reducer not found, deserialization failure before invoker runs).
 * @ingroup module_abi_exports
 */
SPACETIMEDB_WASM_EXPORT int16_t __call_reducer__(
    uint32_t reducer_id,
    uint64_t sender_identity_p0,
    uint64_t sender_identity_p1,
    uint64_t sender_identity_p2,
    uint64_t sender_identity_p3,
    uint64_t connection_id_p0,
    uint64_t connection_id_p1,
    uint64_t timestamp,
    BytesSource args_source_handle,
    BytesSink error_sink_handle
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SPACETIMEDB_MODULE_EXPORTS_H
