#include "spacetimedb/abi/spacetime_module_exports.h"
#include "spacetimedb/abi/abi_utils.h" // For SpacetimeDB::Abi::Utils::write_vector_to_sink etc.
#include "spacetimedb/internal/module_def.h"  // For SpacetimeDB::Internal::get_serialized_module_definition_bytes

#include <vector>
#include <cstddef> // For size_t
#include <iostream> // For temporary error logging if needed

// Note: SPACETIMEDB_WASM_EXPORT is applied in the header "spacetime_module_exports.h"

extern "C" {

    void __describe_module__(BytesSink description_sink_handle) {
        try {
            // 1. Get the serialized ModuleDef
            std::vector<uint8_t> module_def_bytes = SpacetimeDb::Internal::get_serialized_module_definition_bytes();

            // 2. Write it to the sink
            SpacetimeDB::Abi::Utils::write_vector_to_sink(description_sink_handle, module_def_bytes);

            // The sink is typically "done" by the host after it has read the bytes,
            // or if the sink is single-use. If our wrapper implies "done" it should be there.
            // The ManagedBytesSink RAII class calls _bytes_sink_done on destruction.
            // Here, we are passed a raw handle, so the host manages its lifetime.
        }
        catch (const std::exception& e) {
            // How to report errors from __describe_module__? The ABI doesn't specify a return type.
            // Option 1: Log via host call if available (but we might be too early in init).
            // Option 2: Write an "error marker" or empty content to the sink (problematic).
            // Option 3: Trap / abort. This is severe.
            // For now, log to stderr (if Wasm environment routes it) and write nothing or minimal error.
            // This function is critical; if it fails, the module likely won't load.
            // A robust solution would be for the host to provide a way to signal critical init errors.

            // Using iostream for errors for now if no host log is available here.
            std::cerr << "Critical Error in __describe_module__: " << e.what() << std::endl;

            // Try to write an empty or error marker to the sink if possible,
            // although the sink might be in an undefined state if the previous write failed.
            // This is a best-effort attempt.
            try {
                std::string error_msg = "Error generating module description: " + std::string(e.what());
                SpacetimeDB::Abi::Utils::write_string_to_sink(description_sink_handle, error_msg);
            }
            catch (const std::exception& sink_e) {
                std::cerr << "Additionally, failed to write error to sink in __describe_module__: " << sink_e.what() << std::endl;
            }
            // The module is likely in a non-functional state if this fails.
        }
        catch (...) {
            std::cerr << "Critical Unknown Error in __describe_module__." << std::endl;
            try {
                std::string error_msg = "Unknown error generating module description.";
                SpacetimeDB::Abi::Utils::write_string_to_sink(description_sink_handle, error_msg);
            }
            catch (...) {
                // Silent failure to write to sink
            }
        }
    }

} // extern "C"
