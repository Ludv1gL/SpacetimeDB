#include "spacetimedb/abi/spacetime_module_exports.h" // For __call_reducer__ declaration
#include "spacetimedb/abi/abi_utils.h"           // For SpacetimeDB::Abi::Utils helpers
#include "spacetimedb/internal/module_schema.h"  // Updated path, For SpacetimeDb::ModuleSchema
#include "spacetimedb/bsatn_all.h"            // For BSATN types

#include <string>
#include <vector>
#include <stdexcept> // For std::runtime_error
#include <iostream>  // For temporary error logging
#include <algorithm> // For std::advance (iterator increment)
#include <cstddef>   // For std::byte

// Note: SPACETIMEDB_WASM_EXPORT is applied in the header "spacetime_module_exports.h"

extern "C" {

    // Helper to get the Nth reducer definition from the schema's map.
    // THIS IS A TEMPORARY AND INEFFICIENT APPROACH for mapping reducer_id.
    // A proper solution would involve registering reducers with stable numeric IDs.
    const SpacetimeDb::ReducerDefinition* get_reducer_by_id(const SpacetimeDb::ModuleSchema& schema, uint32_t reducer_id) {
        if (reducer_id >= schema.reducers.size()) {
            std::cerr << "Error: reducer_id " << reducer_id << " is out of bounds. Total reducers: " << schema.reducers.size() << std::endl;
            return nullptr;
        }
        auto it = schema.reducers.begin();
        // std::advance is O(N), direct indexing would be better if reducers were in a vector or map<id, def>
        for (uint32_t i = 0; i < reducer_id; ++i) {
            if (it == schema.reducers.end()) { // Should not happen if size check is correct
                std::cerr << "Error: Advanced past end of reducers map unexpectedly for reducer_id " << reducer_id << std::endl;
                return nullptr;
            }
            ++it;
        }
        if (it == schema.reducers.end() && reducer_id < schema.reducers.size()) { // Should not happen
            std::cerr << "Error: Iterator is end but reducer_id " << reducer_id << " should be valid." << std::endl;
            return nullptr;
        }
        return &(it->second);
    }


    int16_t __call_reducer__(
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
    ) {
        (void)sender_identity_p0; (void)sender_identity_p1; (void)sender_identity_p2; (void)sender_identity_p3;
        (void)connection_id_p0; (void)connection_id_p1;
        (void)timestamp;

        // args_source_handle and error_sink_handle are externally managed.
        // We don't use ManagedBytesSource/Sink for them here as they don't take existing handles.

        try {
            std::vector<std::byte> args_bytes = SpacetimeDB::Abi::Utils::read_all_from_source(args_source_handle);
            SpacetimeDb::bsatn::Reader reader(args_bytes);
            const auto& schema = SpacetimeDb::ModuleSchema::instance();
            const SpacetimeDb::ReducerDefinition* reducer_def_ptr = get_reducer_by_id(schema, reducer_id);

            if (!reducer_def_ptr) {
                std::string error_msg = "Reducer with ID " + std::to_string(reducer_id) + " not found.";
                std::cerr << "Error in __call_reducer__: " << error_msg << std::endl;
                // Use error_sink_handle directly
                SpacetimeDB::Abi::Utils::write_string_to_sink(error_sink_handle, error_msg);
                return -1;
            }
            const SpacetimeDb::ReducerDefinition& reducer_def = *reducer_def_ptr;

            if (!reducer_def.invoker) {
                std::string error_msg = "Reducer '" + reducer_def.spacetime_name + "' (ID: " + std::to_string(reducer_id) + ") has no invoker registered.";
                std::cerr << "Error in __call_reducer__: " << error_msg << std::endl;
                // Use error_sink_handle directly
                SpacetimeDB::Abi::Utils::write_string_to_sink(error_sink_handle, error_msg);
                return -2;
            }

            // TODO: Set up ReducerContext if it becomes a thread_local or similar static instance
            // ReducerContext::Current().set_identity(...);
            // ReducerContext::Current().set_timestamp(timestamp);

            reducer_def.invoker(reader);

            // ReducerContext::Current().reset(); // Clear context after call

            if (!reader.is_eos()) {
                std::string warning_msg = "Warning: Reducer '" + reducer_def.spacetime_name + "' (ID: " +
                    std::to_string(reducer_id) + ") did not consume all arguments. " +
                    std::to_string(reader.remaining_bytes()) + " bytes remaining.";
                std::cerr << warning_msg << std::endl;
            }
            return 0; // Success

        }
        catch (const std::exception& e) {
            std::string error_msg = "Exception during reducer execution (ID: " + std::to_string(reducer_id) + "): " + e.what();
            std::cerr << "Error in __call_reducer__: " << error_msg << std::endl;
            try {
                // Use error_sink_handle directly
                SpacetimeDB::Abi::Utils::write_string_to_sink(error_sink_handle, error_msg);
            }
            catch (const std::exception& sink_e) {
                std::cerr << "Additionally, failed to write error to sink in __call_reducer__: " << sink_e.what() << std::endl;
            }
            return -3;
        }
        catch (...) {
            std::string error_msg = "Unknown exception during reducer execution (ID: " + std::to_string(reducer_id) + ").";
            std::cerr << "Error in __call_reducer__: " << error_msg << std::endl;
            try {
                // Use error_sink_handle directly
                SpacetimeDB::Abi::Utils::write_string_to_sink(error_sink_handle, error_msg);
            }
            catch (const std::exception& sink_e) {
                std::cerr << "Additionally, failed to write error to sink in __call_reducer__: " << sink_e.what() << std::endl;
            }
            return -4;
        }
    }

} // extern "C"
