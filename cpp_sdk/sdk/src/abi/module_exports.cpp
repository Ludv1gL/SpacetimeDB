#include "spacetimedb/abi/spacetime_module_exports.h"
#include "spacetimedb/abi/abi_utils.h"
#include "spacetimedb/internal/module_def.h"  // Updated path

#include <vector>
#include <cstddef> // For std::byte
#include <string>  // For std::string in error handling
#include <iostream> // For temporary error logging if needed

// Note: SPACETIMEDB_WASM_EXPORT is applied in the header "spacetime_module_exports.h"

extern "C" {

void __describe_module__(BytesSink description_sink_handle) {
    try {
        // 1. Get the serialized ModuleDef
        std::vector<std::byte> module_def_bytes = SpacetimeDB::Internal::get_serialized_module_definition_bytes();

        // 2. Write it to the sink
        SpacetimeDB::Abi::Utils::write_vector_to_sink(description_sink_handle, module_def_bytes);

    } catch (const std::exception& e) {
        std::cerr << "Critical Error in __describe_module__: " << e.what() << std::endl;
        try {
            std::string error_msg = "Error generating module description: " + std::string(e.what());
            // Assuming write_string_to_sink is robust enough or we accept potential nested exception here.
            SpacetimeDB::Abi::Utils::write_string_to_sink(description_sink_handle, error_msg);
        } catch (const std::exception& sink_e) {
            std::cerr << "Additionally, failed to write error to sink in __describe_module__: " << sink_e.what() << std::endl;
        }
    } catch (...) {
        std::cerr << "Critical Unknown Error in __describe_module__." << std::endl;
        try {
            std::string error_msg = "Unknown error generating module description.";
            SpacetimeDB::Abi::Utils::write_string_to_sink(description_sink_handle, error_msg);
        } catch (...) {
            // Silent failure to write to sink if error reporting itself fails
        }
    }
}

// __call_reducer__ implementation will be in spacetime_reducer_bridge.cpp (or a similar named file)
// and included in the build. If it's also considered part of "module_exports", it could be here too.
// For now, keeping it separate as per original plan.

} // extern "C"
