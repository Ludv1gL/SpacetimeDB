/**
 * SpacetimeDB C++ SDK Example: Clean Module Syntax
 * 
 * This example demonstrates the clean syntax for SpacetimeDB C++ modules using
 * the spacetimedb_easy.h header. It showcases:
 * 
 * - Declaring tables within SPACETIMEDB_MODULE()
 * - Using ReducerContext for type-safe database access
 * - Multi-parameter reducers with custom types
 * - Public and private table declarations
 * 
 * To build:
 *   emcc -std=c++20 -s STANDALONE_WASM=1 -s FILESYSTEM=0 \
 *        -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry \
 *        -I../../sdk/include -o module.wasm reducer_clean_ctx.cpp
 * 
 * To publish:
 *   spacetime publish --bin-path module.wasm my-database
 */

#include <spacetimedb/spacetimedb_easy.h>

using namespace spacetimedb;

/**
 * Example table row type.
 * This struct will be automatically serialized using BSATN.
 */
struct OneU8 {
    uint8_t n;
};

/**
 * Table declarations.
 * 
 * SPACETIMEDB_TABLE(Type, table_name, is_public)
 * - Type: The C++ struct type for table rows
 * - table_name: The table name (becomes ctx.db.table_name())
 * - is_public: true for public tables, false for private
 */
SPACETIMEDB_TABLE(OneU8, one_u8, true)      // Public table
SPACETIMEDB_TABLE(OneU8, another_u8, false) // Private table

// Insert a single value into the public table
SPACETIMEDB_REDUCER(insert_one_u8, ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db.one_u8().insert(row);
}

// Insert a single value into the private table
SPACETIMEDB_REDUCER(insert_another_u8, ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db.another_u8().insert(row);
}

// Initialize the database with default values
SPACETIMEDB_REDUCER(init_db, ReducerContext ctx) {
    log("Database initialized!");
    ctx.db.one_u8().insert({42});
    ctx.db.another_u8().insert({100});
}

// Insert a value with an offset calculation
SPACETIMEDB_REDUCER(insert_with_offset, ReducerContext ctx, uint8_t n, uint8_t offset) {
    OneU8 row{static_cast<uint8_t>(n + offset)};
    ctx.db.one_u8().insert(row);
}

// Insert a range of values with a specified step
SPACETIMEDB_REDUCER(insert_range, ReducerContext ctx, uint8_t start, uint8_t end, uint8_t step) {
    for (uint8_t i = start; i <= end; i += step) {
        OneU8 row{i};
        ctx.db.one_u8().insert(row);
    }
}