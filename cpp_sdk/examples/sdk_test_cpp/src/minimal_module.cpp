// Minimal SpacetimeDB module with clean macro syntax
#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/module_bindings_generator.h>

using namespace spacetimedb;

// Define table with clean syntax
SPACETIMEDB_TABLE(name = "one_u8", public = true)
struct OneU8 {
    uint8_t n;
};

// Define reducer with clean syntax
SPACETIMEDB_REDUCER()
void insert_one_u8(ReducerContext ctx, byte n) {
    OneU8 row{n};
    ctx.db.table<OneU8>().insert(row);
    // log("Inserted value: " + std::to_string(n));
}

// Single macro that handles all registrations
SPACETIMEDB_MODULE_INIT()