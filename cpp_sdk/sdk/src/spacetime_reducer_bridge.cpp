#include "spacetimedb/abi/spacetime_module_exports.h" // For __call_reducer__ declaration
#include "spacetimedb/abi/abi_utils.h"           // For SpacetimeDB::Abi::Utils helpers
#include "spacetimedb/internal/module_schema.h"  // For SpacetimeDb::ModuleSchema
#include "spacetimedb/bsatn_all.h"           // For BSATN types

#include <string>
#include <vector>
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For temporary error logging
#include <algorithm> // For std::advance (iterator increment)

// Note: SPACETIMEDB_WASM_EXPORT is applied in the header "spacetime_module_exports.h"

extern "C" {

// Helper to get the Nth reducer definition from the schema's map.
// THIS IS A TEMPORARY AND INEFFICIENT APPROACH for mapping reducer_id.
// A proper solution would involve registering reducers with stable numeric IDs.
const SpacetimeDb::ReducerDefinition* get_reducer_by_id(const SpacetimeDb::ModuleSchema& schema, uint32_t reducer_id) {
    if (reducer_id >= schema.reducers.size()) {
        return nullptr; // ID out of bounds
    }
    auto it = schema.reducers.begin();
    std::advance(it, reducer_id); // Advance iterator N times
    if (it == schema.reducers.end()) { // Should not happen if size check passed, but good for safety
        return nullptr;
    }
    return &(it->second);
}


int16_t __call_reducer__(
    uint32_t reducer_id,
    uint64_t sender_identity_p0, // TODO: How to pass these to the C++ reducer context?
    uint64_t sender_identity_p1,
    uint64_t sender_identity_p2,
    uint64_t sender_identity_p3,
    uint64_t connection_id_p0,   // TODO: How to pass these?
    uint64_t connection_id_p1,
    uint64_t timestamp,          // TODO: How to pass these?
    BytesSource args_source_handle,
    BytesSink error_sink_handle
) {
    // Suppress unused warnings for parameters not yet fully handled
    (void)sender_identity_p0; (void)sender_identity_p1; (void)sender_identity_p2; (void)sender_identity_p3;
    (void)connection_id_p0; (void)connection_id_p1;
    (void)timestamp;

    SpacetimeDB::Abi::Utils::ManagedBytesSource source_manager(args_source_handle); // Ensures _bytes_source_done is called

    try {
        // 1. Read all argument bytes from args_source_handle
        std::vector<std::byte> args_bytes = SpacetimeDB::Abi::Utils::read_all_from_source(source_manager.get_handle());

        // 2. Create a bsatn::Reader for these bytes
        SpacetimeDb::bsatn::Reader reader(args_bytes);

        // 3. Look up the ReducerDefinition
        const auto& schema = SpacetimeDb::ModuleSchema::instance();

        // TODO: Replace this temporary ID mapping with a robust solution.
        const SpacetimeDb::ReducerDefinition* reducer_def_ptr = get_reducer_by_id(schema, reducer_id);

        if (!reducer_def_ptr) {
            std::string error_msg = "Reducer with ID " + std::to_string(reducer_id) + " not found.";
            std::cerr << "Error in __call_reducer__: " << error_msg << std::endl; // Temporary log
            SpacetimeDB::Abi::Utils::ManagedBytesSink err_sink_manager(error_sink_handle); // Ensures _bytes_sink_done
            SpacetimeDB::Abi::Utils::write_string_to_sink(err_sink_manager.get_handle(), error_msg);
            return -1; // Error: Reducer not found
        }
        const SpacetimeDb::ReducerDefinition& reducer_def = *reducer_def_ptr;

        if (!reducer_def.invoker) {
            std::string error_msg = "Reducer '" + reducer_def.spacetime_name + "' (ID: " + std::to_string(reducer_id) + ") has no invoker registered.";
            std::cerr << "Error in __call_reducer__: " << error_msg << std::endl; // Temporary log
            SpacetimeDB::Abi::Utils::ManagedBytesSink err_sink_manager(error_sink_handle);
            SpacetimeDB::Abi::Utils::write_string_to_sink(err_sink_manager.get_handle(), error_msg);
            return -2; // Error: Invoker not found
        }

        // TODO: Set up ReducerContext with sender_identity, connection_id, timestamp
        // SpacetimeDB::ReducerContext::set_current_context(...);

        // 4. Call the invoker lambda
        reducer_def.invoker(reader);

        // TODO: Clear ReducerContext
        // SpacetimeDB::ReducerContext::clear_current_context();

        // 5. Check if all arguments were consumed (optional, but good for debugging)
        if (!reader.is_eos()) {
            std::string warning_msg = "Warning: Reducer '" + reducer_def.spacetime_name + "' (ID: " +
                                      std::to_string(reducer_id) + ") did not consume all arguments. " +
                                      std::to_string(reader.remaining_bytes()) + " bytes remaining.";
            // This might indicate an issue with the reducer's argument parsing or the calling convention.
            // For now, log it. Could also be an error if strict parsing is required.
             std::cerr << warning_msg << std::endl; // Temporary log
            // SpacetimeDB::log_warn(warning_msg); // If logging is set up and usable here
        }

        return 0; // Success

    } catch (const std::exception& e) {
        std::string error_msg = "Exception during reducer execution (ID: " + std::to_string(reducer_id) + "): " + e.what();
        std::cerr << "Error in __call_reducer__: " << error_msg << std::endl; // Temporary log
        try {
            SpacetimeDB::Abi::Utils::ManagedBytesSink err_sink_manager(error_sink_handle);
            SpacetimeDB::Abi::Utils::write_string_to_sink(err_sink_manager.get_handle(), error_msg);
        } catch (const std::exception& sink_e) {
            std::cerr << "Additionally, failed to write error to sink in __call_reducer__: " << sink_e.what() << std::endl;
        }
        return -3; // Error: Exception during execution
    } catch (...) {
        std::string error_msg = "Unknown exception during reducer execution (ID: " + std::to_string(reducer_id) + ").";
        std::cerr << "Error in __call_reducer__: " << error_msg << std::endl; // Temporary log
        try {
            SpacetimeDB::Abi::Utils::ManagedBytesSink err_sink_manager(error_sink_handle);
            SpacetimeDB::Abi::Utils::write_string_to_sink(err_sink_manager.get_handle(), error_msg);
        } catch (const std::exception& sink_e) {
            std::cerr << "Additionally, failed to write error to sink in __call_reducer__: " << sink_e.what() << std::endl;
        }
        return -4; // Error: Unknown exception
    }
}

} // extern "C"
