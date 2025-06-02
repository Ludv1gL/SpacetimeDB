// Simple working example that uses the same approach as minimal_module.cpp
// but with cleaner procedural-style syntax
#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/module_bindings_generator.h>

using namespace spacetimedb;

// Define table with clean procedural syntax
SPACETIMEDB_TABLE(name = "one_u8", public = true)
struct OneU8 {
    uint8_t n;
};

// Define reducer with clean procedural syntax  
SPACETIMEDB_REDUCER()
void insert_one_u8(ReducerContext ctx, byte n) {
    OneU8 row{n};
    ctx.db.table<OneU8>().insert(row);
}

// Single procedural call that registers everything
SPACETIMEDB_MODULE_INIT()