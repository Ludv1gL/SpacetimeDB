// Clean SpacetimeDB module with even cleaner reducer syntax!
#include <spacetimedb/spacetimedb.h>

using namespace spacetimedb;

// Table macro BEFORE struct (like Rust!)
SPACETIMEDB_TABLE(OneU8, "one_u8", true)
struct OneU8 {
    uint8_t n;
};

// New cleaner reducer syntax - combines declaration and definition!
// Now we specify the full parameter types and names in the macro
SPACETIMEDB_REDUCER(insert_one_u8, ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db.table<OneU8>().insert(row);
}

// That's the cleanest we can get in C++!