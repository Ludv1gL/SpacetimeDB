#ifndef KV_STORE_H
#define KV_STORE_H

// SDK Headers - Assuming these are findable via include paths set up by CMake
// e.g., target_include_directories( ... PUBLIC ${SPACETIMEDB_SDK_INCLUDE_DIR})
// where SPACETIMEDB_SDK_INCLUDE_DIR points to the 'sdk/include' directory.
// The user would then #include "spacetimedb/sdk/types.h" etc. if headers are organized under spacetimedb/sdk
// For this regeneration, I'll assume the SDK headers are structured to be included like this:
#include <spacetimedb/sdk/spacetimedb_library_types.h>
#include <spacetimedb/bsatn_all.h> // Assuming bsatn.h is under spacetimedb/bsatn path
// #include <spacetimedb/sdk/spacetimedb_library_table_registry.h> // Removed as per plan
#include <spacetimedb/sdk/reducer_context.h>
#include <spacetimedb/macros.h> // Added as per plan

#include <string>
#include <vector> // Though not directly used in KeyValue, often useful

namespace SpacetimeDb_quickstart {

// Log levels for direct _console_log usage
// These should ideally match any enum or constants defined by the host or ABI for clarity
const uint8_t LOG_LEVEL_FATAL = 0;
const uint8_t LOG_LEVEL_ERROR = 1;
const uint8_t LOG_LEVEL_WARN = 2;
const uint8_t LOG_LEVEL_INFO = 3;
const uint8_t LOG_LEVEL_DEBUG = 4;
const uint8_t LOG_LEVEL_TRACE = 5;


struct KeyValue { // Removed inheritance from BsatnSerializable
    uint64_t id;           // New auto-incrementing PK
    std::string key_str;   // Now a unique key, not PK
    std::string value_str;

    // Default constructor
    KeyValue() = default;

    // Constructor for convenience
    KeyValue(std::string k, std::string v) : id(0), key_str(std::move(k)), value_str(std::move(v)) {}

    // bsatn_serialize and bsatn_deserialize removed

    // Optional: Comparison operator for potential use in tests or other logic
    bool operator==(const KeyValue& other) const {
        return id == other.id && key_str == other.key_str && value_str == other.value_str;
    }
};

#define KEY_VALUE_FIELDS(XX) \
    XX(uint64_t, id, false, false) \
    XX(std::string, key_str, false, false) \
    XX(std::string, value_str, false, false)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    spacetimedb_quickstart::KeyValue, spacetimedb_quickstart_KeyValue,
    "KeyValue",
    KEY_VALUE_FIELDS,
    {
        SPACETIMEDB_FIELD("id", SpacetimeDb::CoreType::U64, false /* IsUniqueBool */, true /* IsAutoIncBool */),
        SPACETIMEDB_FIELD("key_str", SpacetimeDb::CoreType::String, true /* IsUniqueBool */, false /* IsAutoIncBool */),
        SPACETIMEDB_FIELD("value_str", SpacetimeDb::CoreType::String, false /* IsUniqueBool */, false /* IsAutoIncBool */)
    }
);

// Reducer function declarations
void kv_put(SpacetimeDb::sdk::ReducerContext& ctx, const std::string& key, const std::string& value);
void kv_get(SpacetimeDb::sdk::ReducerContext& ctx, const std::string& key);
void kv_del(SpacetimeDb::sdk::ReducerContext& ctx, const std::string& key);

} // namespace SpacetimeDb_quickstart

#endif // KV_STORE_H
