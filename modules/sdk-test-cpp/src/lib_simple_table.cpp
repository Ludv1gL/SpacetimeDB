// Simple test of SPACETIMEDB_TABLE macro
#include <spacetimedb/spacetimedb.h>

// Define a simple table
SPACETIMEDB_TABLE(Person, public)
struct Person {
    SPACETIMEDB_DATA_MEMBER(id)
    uint32_t id;
    
    SPACETIMEDB_DATA_MEMBER(name) 
    std::string name;
    
    SPACETIMEDB_PRIMARY_KEY(id)
};

// Simple reducer that just logs
SPACETIMEDB_REDUCER(test_reducer, UserDefined, ctx) {
    LOG_INFO("Test reducer called!");
}