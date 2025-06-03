// Test module that matches current SDK limitations
#define SPACETIMEDB_TABLES_LIST \
    X(OneU8, one_u8, true) \
    X(AnotherU8, another_u8, false)

#include <spacetimedb/spacetimedb.h>

using namespace spacetimedb;

// These structs match what add_fields_for_type expects
struct OneU8 {
    uint8_t n;  // Must be named 'n' and be uint8_t
};

struct AnotherU8 {
    uint8_t n;  // Must be named 'n' and be uint8_t
};

SPACETIMEDB_REDUCER(add_one, ReducerContext ctx, uint8_t val) {
    OneU8 row{val};
    ctx.db.one_u8().insert(row);
}

SPACETIMEDB_REDUCER(add_another, ReducerContext ctx, uint8_t val) {
    AnotherU8 row{val};
    ctx.db.another_u8().insert(row);
}