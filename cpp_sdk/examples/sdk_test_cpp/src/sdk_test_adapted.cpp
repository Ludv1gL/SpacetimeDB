// Adapted from quickstart example to create sdk_test tables

#include <spacetimedb/sdk/spacetimedb_sdk_types.h>
#include <spacetimedb/bsatn/bsatn.h>
#include <spacetimedb/sdk/reducer_context.h>
#include <spacetimedb/macros.h>
#include <spacetimedb/sdk/spacetimedb_sdk_reducer.h>
#include <spacetimedb/sdk/database.h>
#include <spacetimedb/sdk/table.h>
#include <spacetimedb/abi/spacetimedb_abi.h>

#include <string>
#include <vector>
#include <cstring>

namespace sdk_test_cpp {

// Simple table OneU8
struct OneU8Row {
    uint8_t n;
    
    OneU8Row() = default;
    OneU8Row(uint8_t n_) : n(n_) {}
};

#define ONE_U8_ROW_FIELDS(XX) \
    XX(uint8_t, n, false, false)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneU8Row, sdk_test_cpp_OneU8Row,
    "OneU8Row",
    ONE_U8_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U8, false, false)
    }
);

// Register the table using the same pattern as quickstart
SPACETIMEDB_TABLE(sdk_test_cpp::OneU8Row, "one_u8", true);

// Reducer function
void insert_one_u8(spacetimedb::sdk::ReducerContext& ctx, uint8_t n) {
    auto table = ctx.db().get_table<OneU8Row>("one_u8");
    OneU8Row row(n);
    table.insert(row);
}

// Register reducer with old-style macro (as in quickstart)
SPACETIMEDB_REDUCER(sdk_test_cpp::insert_one_u8, uint8_t);

// Schema registration for reducer
SPACETIMEDB_REDUCER("insert_one_u8", sdk_test_cpp::insert_one_u8,
    { SPACETIMEDB_REDUCER_PARAM("n", SpacetimeDb::CoreType::U8) },
    uint8_t
);

} // namespace sdk_test_cpp