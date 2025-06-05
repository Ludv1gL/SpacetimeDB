#include "kv_store.h" // Local header

// SDK Headers
#include <spacetimedb/sdk/spacetimedb_library_reducer.h>
#include <spacetimedb/sdk/database.h>
#include <spacetimedb/sdk/table.h>
#include <spacetimedb/abi/spacetimedb_abi.h> // For direct ABI calls like _console_log

// Standard Library
#include <string>
#include <vector>
#include <stdexcept> // For std::runtime_error (used by SDK components)
#include <cstring>   // For std::strlen in log_message_abi

// The _spacetimedb_library_init() function is defined in spacetimedb_library_reducer.h
// and will be exported. The host calls it to initialize the SDK, including the
// global database instance needed by the ReducerContext.

namespace SpacetimeDb_quickstart {

// KeyValue BSATN implementation
// void KeyValue::bsatn_serialize(SpacetimeDb::sdk::bsatn::bsatn_writer& writer) const {
//     writer.write_string(key_str);   // First field, assumed PK (index 0)
//     writer.write_string(value_str); // Second field
// }

// void KeyValue::bsatn_deserialize(SpacetimeDb::sdk::bsatn::bsatn_reader& reader) {
//     key_str = reader.read_string();
//     value_str = reader.read_string();
// }

// Register the KeyValue table with the SDK's global registry.
// "key_str" is declared as the primary key field.
// The SDK's table registration currently assumes if a PK field name is provided,
// its column index is 0 (i.e., it's the first field serialized).
// SPACETIMEDB_REGISTER_TABLE(spacetimedb_quickstart::KeyValue, "kv_pairs", "key_str"); // Removed as per plan

SPACETIMEDB_TABLE(spacetimedb_quickstart::KeyValue, "kv_pairs", true /* is_public */);
SPACETIMEDB_PRIMARY_KEY("kv_pairs", "id"); // Changed PK to "id"
SPACETIMEDB_INDEX("kv_pairs", "idx_key_str", { "key_str" });

// Helper for logging from reducers via the raw ABI
static void log_message_abi(uint8_t level, const std::string& context_info, const std::string& message) {
    std::string full_message = "[" + context_info + "] " + message;
    // _console_log(uint8_t level, const uint8_t *target, size_t target_len,
    //              const uint8_t *filename, size_t filename_len, uint32_t line_number,
    //              const uint8_t *text, size_t text_len)
    _console_log(level,
                 nullptr, 0,  // target (e.g. module path) - omitting for simplicity
                 nullptr, 0,  // filename - omitting for simplicity
                 0,           // line_number - omitting for simplicity
                 reinterpret_cast<const uint8_t*>(full_message.c_str()),
                 full_message.length());
}

// Reducer Implementations

void kv_put(SpacetimeDb::sdk::ReducerContext& ctx, const std::string& key, const std::string& value) {
    std::string reducer_name = "kv_put";
    try {
        auto kv_table = ctx.db().get_table<KeyValue>("kv_pairs");

        // Upsert logic based on key_str (column index 1)
        // Primary key 'id' (column index 0) is auto-incrementing.
        uint32_t key_str_col_idx = 1;
        uint32_t id_pk_col_idx = 0;

        std::vector<KeyValue> existing_rows = kv_table.find_by_col_eq(key_str_col_idx, key);
        if (!existing_rows.empty()) {
            // key_str is unique, so there should be at most one row.
            kv_table.delete_by_col_eq(id_pk_col_idx, existing_rows[0].id);
        }

        KeyValue row_to_insert(key, value); // id is 0 initially
        kv_table.insert(row_to_insert); // id will be auto-generated and updated in row_to_insert

        std::string log_msg = "Successfully put K-V: (id: " + std::to_string(row_to_insert.id) + ", key: " + key + ", value: " + value + ")";
        log_message_abi(LOG_LEVEL_INFO, reducer_name, log_msg);

    } catch (const std::runtime_error& e) {
        std::string error_msg = "Error: " + std::string(e.what());
        log_message_abi(LOG_LEVEL_ERROR, reducer_name, error_msg);
        // The reducer macro will catch this exception and return an error code to the host.
        throw; // Re-throw to be caught by the reducer macro wrapper
    }
}

void kv_get(SpacetimeDb::sdk::ReducerContext& ctx, const std::string& key) {
    std::string reducer_name = "kv_get";
    try {
        auto kv_table = ctx.db().get_table<KeyValue>("kv_pairs");
        uint32_t key_str_col_idx = 1; // Find by key_str (column index 1)

        std::vector<KeyValue> rows = kv_table.find_by_col_eq(key_str_col_idx, key);

        if (!rows.empty()) {
            // key_str is unique, so there should be at most one row.
            const auto& row = rows[0];
            std::string log_msg = "Found by key_str '" + key + "': (id: " + std::to_string(row.id) + ", key: " + row.key_str + ", value: " + row.value_str + ")";
            log_message_abi(LOG_LEVEL_INFO, reducer_name, log_msg);
        } else {
            std::string log_msg = "No entry found for key_str: " + key;
            log_message_abi(LOG_LEVEL_INFO, reducer_name, log_msg);
        }
    } catch (const std::runtime_error& e) {
        std::string error_msg = "Error: " + std::string(e.what());
        log_message_abi(LOG_LEVEL_ERROR, reducer_name, error_msg);
        throw;
    }
}

void kv_del(SpacetimeDb::sdk::ReducerContext& ctx, const std::string& key) {
    std::string reducer_name = "kv_del";
    try {
        auto kv_table = ctx.db().get_table<KeyValue>("kv_pairs");
        uint32_t key_str_col_idx = 1; // Delete by key_str (column index 1)

        uint32_t deleted_count = kv_table.delete_by_col_eq(key_str_col_idx, key);

        if (deleted_count > 0) {
            std::string log_msg = "Successfully deleted " + std::to_string(deleted_count) + " item(s) for key_str: " + key;
            log_message_abi(LOG_LEVEL_INFO, reducer_name, log_msg);
        } else {
            std::string log_msg = "No items found to delete for key_str: " + key;
            log_message_abi(LOG_LEVEL_INFO, reducer_name, log_msg);
        }
    } catch (const std::runtime_error& e) {
        std::string error_msg = "Error: " + std::string(e.what());
        log_message_abi(LOG_LEVEL_ERROR, reducer_name, error_msg);
        throw;
    }
}

// Register Reducers with the SDK.
// The types listed (e.g., const std::string&) must match the C++ function signature after ReducerContext.
// The actual exported WASM function name will be "kv_put", "kv_get", "kv_del".
SPACETIMEDB_REDUCER(spacetimedb_quickstart::kv_put, const std::string&, const std::string&);
SPACETIMEDB_REDUCER(spacetimedb_quickstart::kv_get, const std::string&);
SPACETIMEDB_REDUCER(spacetimedb_quickstart::kv_del, const std::string&);

// Schema registrations for reducers
SPACETIMEDB_REDUCER("kv_put", spacetimedb_quickstart::kv_put,
    { SPACETIMEDB_REDUCER_PARAM("key", SpacetimeDb::CoreType::String),
      SPACETIMEDB_REDUCER_PARAM("value", SpacetimeDb::CoreType::String) },
    std::string, std::string
);

SPACETIMEDB_REDUCER("kv_get", spacetimedb_quickstart::kv_get,
    { SPACETIMEDB_REDUCER_PARAM("key", SpacetimeDb::CoreType::String) },
    std::string
);

SPACETIMEDB_REDUCER("kv_del", spacetimedb_quickstart::kv_del,
    { SPACETIMEDB_REDUCER_PARAM("key", SpacetimeDb::CoreType::String) },
    std::string
);

} // namespace SpacetimeDb_quickstart
