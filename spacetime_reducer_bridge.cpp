#include "spacetime_module_abi.h" // For declaration of _spacetimedb_dispatch_reducer if we place it there
#include "spacetime_schema.h"   // For SpacetimeDb::ModuleSchema, SpacetimeDb::ReducerDefinition
#include "bsatn_reader.h"       // For bsatn::Reader
#include <string>
#include <vector>
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For error logging (temporary)

// Make sure bsatn::Reader is available
// It should be included via spacetime_schema.h -> spacetime_macros.h -> bsatn_reader.h,
// or directly if spacetime_schema.h doesn't pull it.
// For this file, explicitly including bsatn_reader.h is good.

// Declaration for the exported C function.
// It should be declared in "spacetime_module_abi.h" or a similar central ABI header
// if other parts of the SDK or user code need to know about it.
// For now, its definition here also serves as its declaration for Emscripten export.
// To use EMSCRIPTEN_KEEPALIVE, it should be on the function definition.

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define SPACETIMEDB_WASM_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define SPACETIMEDB_WASM_EXPORT
#endif

extern "C" {

/**
 * @brief Dispatches a call to the appropriate C++ reducer function.
 * @ingroup module_abi
 * This function is called by the SpacetimeDB host environment.
 * It looks up the reducer by name, deserializes arguments from `args_data` using
 * the invoker registered by the `SPACETIMEDB_REDUCER` macro, and then calls
 * the target C++ reducer function.
 *
 * @param reducer_name_ptr Pointer to the UTF-8 encoded string for the reducer's name.
 * @param reducer_name_len Length of the reducer name string.
 * @param args_data_ptr Pointer to the byte array containing BSATN-serialized arguments.
 * @param args_data_len Length of the arguments byte array.
 */
SPACETIMEDB_WASM_EXPORT void _spacetimedb_dispatch_reducer(const char* reducer_name_ptr,
                                   uint32_t reducer_name_len,
                                   const unsigned char* args_data_ptr,
                                   uint32_t args_data_len) {
    if (!reducer_name_ptr) {
        // Or use a host logging function if available
        std::cerr << "Error: _spacetimedb_dispatch_reducer called with null reducer_name_ptr." << std::endl;
        return;
    }
    if (!args_data_ptr && args_data_len > 0) {
        std::cerr << "Error: _spacetimedb_dispatch_reducer called with null args_data_ptr but non-zero length." << std::endl;
        return;
    }

    std::string reducer_name(reducer_name_ptr, reducer_name_len);

    // Prepare args span/view for the reader
    // Using reinterpret_cast for args_data_ptr from unsigned char* to const std::byte*
    // This is generally safe if std::byte is an enum class : unsigned char or similar.
    // C++20 std::span is ideal here.
    std::span<const std::byte> args_span(reinterpret_cast<const std::byte*>(args_data_ptr), args_data_len);
    bsatn::Reader reader(args_span);

    try {
        const auto& schema = SpacetimeDb::ModuleSchema::instance();
        auto it = schema.reducers.find(reducer_name);

        if (it == schema.reducers.end()) {
            // Consider using spacetimedb_host_log_error if available
            std::cerr << "Error: Reducer '" << reducer_name << "' not found." << std::endl;
            // Potentially throw or return an error code if ABI supports it
            return;
        }

        const SpacetimeDb::ReducerDefinition& reducer_def = it->second;
        if (!reducer_def.invoker) {
            std::cerr << "Error: Reducer '" << reducer_name << "' has no invoker registered." << std::endl;
            return;
        }

        // Call the type-erased invoker
        reducer_def.invoker(reader);

        // Check if reader consumed all arguments
        if (!reader.is_eos()) {
            std::cerr << "Warning: Reducer '" << reducer_name
                      << "' did not consume all arguments. "
                      << reader.remaining_bytes() << " bytes remaining." << std::endl;
        }

    } catch (const std::exception& e) {
        // Consider using spacetimedb_host_log_error
        std::cerr << "Error while dispatching reducer '" << reducer_name << "': " << e.what() << std::endl;
        // Depending on host ABI, might need to propagate error differently
    } catch (...) {
        std::cerr << "Unknown error while dispatching reducer '" << reducer_name << "'." << std::endl;
    }
}

} // extern "C"
