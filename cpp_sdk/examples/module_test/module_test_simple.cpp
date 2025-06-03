// Simplified module_test that works within current SDK limitations
// Current limitation: All tables must have a single uint8_t field named 'n'

#define SPACETIMEDB_TABLES_LIST \
    X(TestValue, test_value, true) \
    X(Counter, counter, true) \
    X(Flag, flag, false)

#include <spacetimedb/spacetimedb.h>

using namespace spacetimedb;

// All structs must match the current SDK limitation
struct TestValue {
    uint8_t n;  // Required: single field named 'n' of type uint8_t
};

struct Counter {
    uint8_t n;  // Represents a counter value
};

struct Flag {
    uint8_t n;  // 0 or 1 for boolean flag
};

// Test various reducer patterns
SPACETIMEDB_REDUCER(insert_value, ReducerContext ctx, uint8_t value) {
    TestValue row{value};
    ctx.db.test_value().insert(row);
    LOG_INFO("Inserted value: " + std::to_string(value));
}

SPACETIMEDB_REDUCER(increment_counter, ReducerContext ctx) {
    // TODO: When select_all() is implemented, we could read and increment
    // For now, just insert a new counter value
    static uint8_t counter = 0;
    counter++;
    Counter row{counter};
    ctx.db.counter().insert(row);
    LOG_INFO("Counter incremented to: " + std::to_string(counter));
}

SPACETIMEDB_REDUCER(set_flag, ReducerContext ctx, bool flag_value) {
    Flag row{flag_value ? uint8_t(1) : uint8_t(0)};
    ctx.db.flag().insert(row);
    LOG_INFO("Flag set to: " + std::to_string(flag_value));
}

// Test multiple parameters
SPACETIMEDB_REDUCER(calculate_and_store, ReducerContext ctx, uint8_t a, uint8_t b) {
    uint8_t result = a + b;
    // Ensure result fits in uint8_t
    if (result > 255) result = 255;
    
    TestValue row{result};
    ctx.db.test_value().insert(row);
    LOG_INFO("Calculated " + std::to_string(a) + " + " + std::to_string(b) + 
             " = " + std::to_string(result));
}

// Init reducer
SPACETIMEDB_REDUCER(init, ReducerContext ctx) {
    LOG_INFO("Module initialized");
    
    // Insert initial values
    TestValue initial{42};
    ctx.db.test_value().insert(initial);
    
    Counter counter{0};
    ctx.db.counter().insert(counter);
    
    Flag flag{0};
    ctx.db.flag().insert(flag);
    
    LOG_INFO("Initial values inserted");
}

// Test error conditions
SPACETIMEDB_REDUCER(test_bounds, ReducerContext ctx, uint8_t value) {
    if (value > 100) {
        LOG_WARN("Value " + std::to_string(value) + " exceeds recommended maximum of 100");
    }
    
    TestValue row{value};
    ctx.db.test_value().insert(row);
}

/*
 * CURRENT SDK LIMITATIONS:
 * 
 * 1. Tables can only have a single field named 'n' of type uint8_t
 * 2. No support for complex types (strings, vectors, structs)
 * 3. No select_all() or remove() operations
 * 4. No access to sender, connection_id, or timestamp in ReducerContext
 * 5. No support for indexes, constraints, or scheduled reducers
 * 
 * These limitations exist because:
 * - C++ lacks reflection for automatic field discovery
 * - The module description format needs proper BSATN serialization
 * - Additional FFI bindings need to be implemented
 * 
 * Future improvements will address these limitations.
 */