#include <spacetimedb/macros.h>

// Define a simple struct without namespace
struct OneU8Row {
    uint8_t n;
};

// X-macro for BSATN serialization
#define ONE_U8_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, n, false, false)

// Register the type with SpacetimeDB
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    OneU8Row, OneU8Row,
    "OneU8Row",
    ONE_U8_ROW_FIELDS,
    ({ SPACETIMEDB_FIELD("n", ::SpacetimeDb::CoreType::U8, false, false) })
);

// Register as a table
SPACETIMEDB_TABLE(OneU8Row, "one_u8", true, "");

// Test reducer
SPACETIMEDB_REDUCER(insert_one_u8, uint8_t);