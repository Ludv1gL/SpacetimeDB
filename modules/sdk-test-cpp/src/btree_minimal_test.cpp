/**
 * Minimal BTree Index Test
 * Tests that BTree index operations compile successfully
 */

#define SPACETIMEDB_TABLES_LIST \
    X(SimpleRecord, records, true)

#include "spacetimedb/spacetimedb.h"

using namespace SpacetimeDb;

// Simple record for testing
struct SimpleRecord {
    uint32_t id;
    uint32_t value;
};

// Register constraints and indexes
SPACETIMEDB_PRIMARY_KEY(SimpleRecord, id);
SPACETIMEDB_INDEX(SimpleRecord, value, value_idx);

SPACETIMEDB_REDUCER(test_btree_operations, ReducerContext ctx) {
    LOG_INFO("Testing BTree index operations");
    
    // Insert test data
    SimpleRecord record1{1, 100};
    SimpleRecord record2{2, 200};
    SimpleRecord record3{3, 150};
    
    ctx.db->records().insert(record1);
    ctx.db->records().insert(record2);
    ctx.db->records().insert(record3);
    
    LOG_INFO("Test data inserted successfully");
    
    // Test that the BTree index classes can be instantiated
    try {
        auto value_index = std::make_unique<BTreeIndex<SimpleRecord, uint32_t>>("value_idx");
        LOG_INFO("BTree index instantiated successfully");
        
        // Test range creation
        Range<uint32_t> test_range(
            Bound<uint32_t>::Inclusive(100),
            Bound<uint32_t>::Inclusive(200)
        );
        LOG_INFO("Range created successfully");
        
        // Note: FilterRange would require BSATN traits for SimpleRecord
        // For now, just test that the infrastructure compiles
        
    } catch (const std::exception& e) {
        LOG_INFO("BTree test error: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(init_btree_minimal_test, ReducerContext ctx) {
    LOG_INFO("BTree Minimal Test Module initialized");
    LOG_INFO("Run test_btree_operations to test BTree index operations");
}