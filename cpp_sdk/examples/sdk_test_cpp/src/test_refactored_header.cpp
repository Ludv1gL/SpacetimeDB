#include <spacetimedb/spacetimedb_refactored.h>
// Note: Since spacetimedb_refactored.h is a reference implementation,
// we'll also need the actual implementations from the existing headers
#include <spacetimedb/spacetimedb_easy.h>
#include <spacetimedb/bsatn/traits.h>

// Test struct 
struct TestData {
    uint32_t id;
    std::string message;
};

// Use BSATN serialization
SPACETIMEDB_BSATN_STRUCT(TestData, id, message)

// Register table
SPACETIMEDB_TABLE(TestData, test_refactored, true)

SPACETIMEDB_REDUCER(test_refactored_header, spacetimedb::ReducerContext ctx, uint32_t value) {
    LOG_INFO("=== Testing Refactored Header ===");
    LOG_INFO("Value: " + std::to_string(value));
    
    TestData data{value, "refactored_test_" + std::to_string(value)};
    ctx.db.table<TestData>("test_refactored").insert(data);
    
    LOG_INFO("✅ Refactored header working with existing implementation!");
}

SPACETIMEDB_REDUCER(verify_features, spacetimedb::ReducerContext ctx) {
    LOG_INFO("=== Verifying Enhanced Features ===");
    
    // Test enhanced logging
    LOG_TRACE("Trace level message");
    LOG_DEBUG("Debug level message");
    LOG_WARN("Warning level message");
    LOG_ERROR("Error level message");
    
    // Test LogStopwatch
    {
        SpacetimeDB::LogStopwatch timer("feature_verification");
        LOG_INFO("Testing performance timing...");
    }
    
    LOG_INFO("✅ All enhanced features working!");
}

SPACETIMEDB_REDUCER(test_convenience_aliases, spacetimedb::ReducerContext ctx) {
    LOG_INFO("=== Testing Convenience Aliases ===");
    
    // Test type aliases
    spacetimedb::Context alias_ctx;  // Context alias
    spacetimedb::DB alias_db;        // DB alias
    
    LOG_INFO("✅ Convenience aliases accessible!");
    LOG_INFO("✅ Headers properly organized!");
}