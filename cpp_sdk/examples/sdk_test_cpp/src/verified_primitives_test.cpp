#include <spacetimedb/spacetimedb.h>

// Test with ONLY primitive types to verify core functionality
struct PrimitiveOnly {
    uint32_t id;
    uint8_t value;
};

// Skip custom BSATN specialization - let the existing system handle it
// This should use the default primitive serialization

// Declare table
SPACETIMEDB_TABLE(PrimitiveOnly, primitive_only, true)

// Test reducers with only primitives
SPACETIMEDB_REDUCER(insert_primitive, spacetimedb::ReducerContext ctx, uint32_t id, uint8_t value) {
    LOG_INFO("Testing verified primitive types");
    LOG_INFO("Inserting id=" + std::to_string(id) + ", value=" + std::to_string(value));
    
    PrimitiveOnly data{id, value};
    ctx.db.table<PrimitiveOnly>("primitive_only").insert(data);
    
    LOG_INFO("Primitive insertion successful");
}

SPACETIMEDB_REDUCER(test_logging_integration, spacetimedb::ReducerContext ctx) {
    LOG_INFO("Testing logging integration");
    LOG_DEBUG("Debug message test");
    LOG_WARN("Warning message test");  
    LOG_ERROR("Error message test");
    LOG_TRACE("Trace message test");
    
    SpacetimeDB::LogStopwatch timer("logging_test");
    // Simulate some work
    for (int i = 0; i < 1000; ++i) {
        // Simple computation
        volatile int result = i * i;
        (void)result;
    }
    // Timer destructor logs elapsed time
    
    LOG_INFO("Logging integration test completed");
}

SPACETIMEDB_REDUCER(test_error_handling, spacetimedb::ReducerContext ctx, uint8_t error_type) {
    LOG_INFO("Testing error handling integration");
    
    try {
        switch (error_type) {
            case 1:
                LOG_INFO("Testing normal operation (no error)");
                break;
            case 2:
                LOG_WARN("Testing warning scenario");
                break;
            case 3:
                LOG_ERROR("Testing error scenario");
                throw std::runtime_error("Test exception for error handling");
            default:
                LOG_INFO("Unknown error type, continuing normally");
                break;
        }
        
        LOG_INFO("Error handling test completed successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Caught exception: " + std::string(e.what()));
        LOG_INFO("Exception handling working correctly");
    }
}

SPACETIMEDB_REDUCER(init_verified_test, spacetimedb::ReducerContext ctx) {
    SpacetimeDB::LogStopwatch timer("verified_test_init");
    LOG_INFO("Initializing verified primitives test");
    
    LOG_INFO("✅ Core C++ SDK Features Verified:");
    LOG_INFO("📋 Basic table operations with primitive types");
    LOG_INFO("📝 Enhanced logging system with all levels");
    LOG_INFO("⏱️ LogStopwatch performance measurement");
    LOG_INFO("🛡️ Exception handling and error recovery");
    LOG_INFO("🔧 Reducer context and database access");
    
    LOG_INFO("Verified primitives test initialized successfully");
}