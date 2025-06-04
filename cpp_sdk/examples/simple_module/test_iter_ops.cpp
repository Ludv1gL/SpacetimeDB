#include <spacetimedb/spacetimedb.h>

// Simple struct without strings
struct Counter {
    uint32_t id;
    uint32_t value;
};

// Table declaration
SPACETIMEDB_TABLE(Counter, counter, true)

// Test iter and count operations
SPACETIMEDB_REDUCER(test_iter_count, spacetimedb::ReducerContext ctx) {
    auto counter_table = ctx.db.table<Counter>("counter");
    
    // Insert some test data
    counter_table.insert({1, 100});
    counter_table.insert({2, 200});
    counter_table.insert({3, 300});
    
    // Count rows
    uint64_t count = counter_table.count();
    // Note: Can't log without strings, but count operation is tested
    
    // Iterate and sum values
    uint32_t sum = 0;
    for (const auto& counter : counter_table.iter()) {
        sum += counter.value;
    }
    // sum should be 600
}

// Test delete operation
SPACETIMEDB_REDUCER(test_delete, spacetimedb::ReducerContext ctx, uint32_t id_to_delete) {
    auto counter_table = ctx.db.table<Counter>("counter");
    
    // Find and delete by matching id
    for (const auto& counter : counter_table.iter()) {
        if (counter.id == id_to_delete) {
            counter_table.delete_by_value(counter);
            break;
        }
    }
}

// Test update operation
SPACETIMEDB_REDUCER(test_update, spacetimedb::ReducerContext ctx, uint32_t id, uint32_t new_value) {
    auto counter_table = ctx.db.table<Counter>("counter");
    
    // Find and update
    for (const auto& counter : counter_table.iter()) {
        if (counter.id == id) {
            Counter updated = {id, new_value};
            counter_table.update(counter, updated);
            break;
        }
    }
}