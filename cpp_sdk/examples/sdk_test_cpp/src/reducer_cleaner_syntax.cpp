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

// Reducer with no parameters
SPACETIMEDB_REDUCER(init_db, ReducerContext ctx) {
    log("Database initialized");
}

// Reducer with multiple parameters
SPACETIMEDB_REDUCER(insert_with_offset, ReducerContext ctx, uint8_t n, uint8_t offset) {
    OneU8 row{static_cast<uint8_t>(n + offset)};
    ctx.db.table<OneU8>().insert(row);
}

// Reducer with three parameters
SPACETIMEDB_REDUCER(insert_range, ReducerContext ctx, uint8_t start, uint8_t end, uint8_t step) {
    for (uint8_t i = start; i <= end; i += step) {
        OneU8 row{i};
        ctx.db.table<OneU8>().insert(row);
    }
}

// That's the cleanest we can get in C++!