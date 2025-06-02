// Clean SpacetimeDB module with table and reducer - no init function needed!
#include <spacetimedb/spacetimedb.h>

using namespace spacetimedb;

// Define struct
struct OneU8 {
    uint8_t n;
};
// Register table with macro below
SPACETIMEDB_TABLE(OneU8, "one_u8", true)

// Define reducer function
void insert_one_u8(ReducerContext ctx, byte n) {
    OneU8 row{n};
    ctx.db.table<OneU8>().insert(row);
}
// Register reducer with macro below
SPACETIMEDB_REDUCER(insert_one_u8)

// That's it! No initialization function needed!