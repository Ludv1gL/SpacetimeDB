#include <spacetimedb/spacetimedb_easy.h>

// Test with only primitive types that should already work
struct SimpleStruct {
    uint32_t id;
    uint8_t value;
};

// Table using only primitives
SPACETIMEDB_TABLE(SimpleStruct, simple_struct, true)

// Test reducer for primitives
SPACETIMEDB_REDUCER(test_primitives, spacetimedb::ReducerContext ctx, uint32_t id, uint8_t value) {
    LOG_INFO("Testing primitive types only");
    
    SimpleStruct data{id, value};
    ctx.db.table<SimpleStruct>("simple_struct").insert(data);
    
    LOG_INFO("Primitive types inserted successfully");
}

SPACETIMEDB_REDUCER(init_primitives_test, spacetimedb::ReducerContext ctx) {
    SpacetimeDB::LogStopwatch timer("primitives_test_init");
    LOG_INFO("Initializing primitives test database");
    
    LOG_INFO("Primitives test database initialized successfully");
}