/**
 * SpacetimeDB C++ SDK: Simple Error Handling Test
 * 
 * A minimal test of the error handling system without complex types.
 */

// Define a simple table for testing
#define SPACETIMEDB_TABLES_LIST \
    X(SimpleRow, simple_table, true)

#include <spacetimedb/spacetimedb_easy.h>
#include <spacetimedb/sdk/exceptions.h>

using namespace spacetimedb;
using namespace SpacetimeDB;

/**
 * Simple test row type.
 */
struct SimpleRow {
    uint32_t id;
    uint8_t value;
};

/**
 * Test successful operation with error handling.
 */
SPACETIMEDB_REDUCER(test_error_success, ReducerContext ctx, uint32_t id, uint8_t value) {
    LOG_INFO("Testing successful operation with error handling");
    
    try {
        SimpleRow row{id, value};
        ctx.db.simple_table().insert(row);
        LOG_INFO("Successfully inserted row");
    } catch (const StdbException& e) {
        LOG_ERROR("SpacetimeDB error occurred");
        throw; // Re-throw the exception
    }
}

/**
 * Test error code checking.
 */
SPACETIMEDB_REDUCER(test_error_codes, ReducerContext ctx, int16_t error_code_value) {
    LOG_INFO("Testing error code handling");
    
    try {
        // Test the error checking function
        if (error_code_value != 0) {
            check_error(static_cast<uint16_t>(error_code_value), "Test error");
        }
        LOG_INFO("No error occurred");
    } catch (const StdbException& e) {
        LOG_INFO("Caught expected error");
    }
}

/**
 * Test resource cleanup with scope guards.
 */
SPACETIMEDB_REDUCER(test_cleanup, ReducerContext ctx, uint32_t id) {
    LOG_INFO("Testing resource cleanup");
    
    bool resource_allocated = false;
    
    auto cleanup_guard = make_scope_guard([&resource_allocated]() {
        if (resource_allocated) {
            LOG_DEBUG("Cleaning up resource");
            resource_allocated = false;
        }
    });
    
    try {
        resource_allocated = true;
        LOG_DEBUG("Resource allocated");
        
        if (id == 999) {
            throw std::runtime_error("Test exception for cleanup demonstration");
        }
        
        SimpleRow row{id, 42};
        ctx.db.simple_table().insert(row);
        LOG_INFO("Operation completed successfully");
    } catch (const std::exception& e) {
        LOG_WARN("Exception caught during operation: " + std::string(e.what()));
        LOG_INFO("Resource cleanup will still occur via RAII");
        // Note: cleanup_guard will still execute its cleanup function
    }
}

/**
 * Initialize the error test database.
 */
SPACETIMEDB_REDUCER(init_simple_error_test, ReducerContext ctx) {
    LOG_INFO("Initializing simple error test database");
    
    {
        LogStopwatch timer("simple_error_test_init");
        
        try {
            ctx.db.simple_table().insert({1, 10});
            ctx.db.simple_table().insert({2, 20});
            LOG_INFO("Test database initialized successfully");
        } catch (const StdbException& e) {
            LOG_ERROR("Failed to initialize test database");
            throw;
        }
    }
}