// Clean SpacetimeDB module with BOTH macros above their definitions!
#include <spacetimedb/spacetimedb.h>

using namespace spacetimedb;

// Table macro BEFORE struct (like Rust!)
SPACETIMEDB_TABLE(OneU8, "one_u8", true)
struct OneU8 {
    uint8_t n;
};

// Reducer macro BEFORE function (like Rust!)
SPACETIMEDB_REDUCER(insert_one_u8)
void insert_one_u8(ReducerContext ctx, byte n) {
    OneU8 row{n};
    ctx.db.table<OneU8>().insert(row);
}

// That's it! Both macros are now above their definitions!