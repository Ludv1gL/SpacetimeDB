/**
 * SpacetimeDB C++ SDK Example: Error Handling System
 * 
 * This example demonstrates the robust error handling system with:
 * - Comprehensive exception hierarchy
 * - Automatic error marshalling from FFI error codes
 * - Resource cleanup with RAII scope guards
 * - Integration with enhanced logging system
 * 
 * To build:
 *   emcc -std=c++20 -s STANDALONE_WASM=1 -s FILESYSTEM=0 \
 *        -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry \
 *        -I../../sdk/include -o error_test.wasm reducer_error_handling.cpp
 */

// Define tables for testing
#define SPACETIMEDB_TABLES_LIST \
    X(TestRow, test_table, true) \
    X(UniqueRow, unique_table, true)

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/sdk/exceptions.h>

using namespace SpacetimeDb;
using namespace SpacetimeDB;

/**
 * Test row type for error handling demonstrations.
 */
struct TestRow {
    uint32_t id;
    std::string name;
};

/**
 * Test row with unique constraint for testing constraint violations.
 */
struct UniqueRow {
    uint32_t unique_id;
    std::string value;
};

/**
 * Demonstrates successful operation with automatic error checking.
 */
SPACETIMEDB_REDUCER(test_success, ReducerContext ctx, uint32_t id, const std::string& name) {
    LOG_INFO("Testing successful operation with error handling");
    
    try {
        TestRow row{id, name};
        ctx.db.test_table().insert(row);
        LOG_INFO("Successfully inserted row with id: " + std::to_string(id));
    } catch (const StdbException& e) {
        LOG_ERROR("Unexpected SpacetimeDB error: " + std::string(e.what()) + 
                  " (code: " + std::to_string(e.error_value()) + ")");
        throw; // Re-throw the exception
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected standard exception: " + std::string(e.what()));
        throw;
    }
}

/**
 * Demonstrates error handling when attempting duplicate unique values.
 */
SPACETIMEDB_REDUCER(test_unique_constraint, ReducerContext ctx, uint32_t unique_id, const std::string& value) {
    LOG_INFO("Testing unique constraint handling");
    
    try {
        // Try to insert a row
        UniqueRow row{unique_id, value};
        ctx.db.unique_table().insert(row);
        LOG_INFO("Successfully inserted unique row with id: " + std::to_string(unique_id));
        
        // Try to insert the same unique_id again (this should fail if there's a unique constraint)
        UniqueRow duplicate_row{unique_id, "different_value"};
        ctx.db.unique_table().insert(duplicate_row);
        LOG_WARN("Duplicate insertion succeeded - unique constraint may not be enforced");
        
    } catch (const UniqueConstraintViolationException& e) {
        LOG_INFO("Correctly caught unique constraint violation: " + std::string(e.what()));
    } catch (const StdbException& e) {
        LOG_WARN("Caught different SpacetimeDB error: " + std::string(e.what()) + 
                 " (code: " + std::to_string(e.error_value()) + ")");
    } catch (const std::exception& e) {
        LOG_ERROR("Unexpected standard exception: " + std::string(e.what()));
        throw;
    }
}

/**
 * Demonstrates resource cleanup with scope guards.
 */
SPACETIMEDB_REDUCER(test_resource_cleanup, ReducerContext ctx, uint32_t id) {
    LOG_INFO("Testing resource cleanup with scope guards");
    
    // Simulate resource allocation that needs cleanup
    bool resource_allocated = false;
    
    // Create a scope guard for automatic cleanup
    auto cleanup_guard = make_scope_guard([&resource_allocated]() {
        if (resource_allocated) {
            LOG_DEBUG("Cleaning up allocated resource");
            resource_allocated = false;
        }
    });
    
    try {
        // Simulate resource allocation
        resource_allocated = true;
        LOG_DEBUG("Resource allocated successfully");
        
        // Simulate some operation that might fail
        if (id == 0) {
            throw std::runtime_error("Invalid ID: 0 is not allowed");
        }
        
        // Insert test data
        TestRow row{id, "cleanup_test"};
        ctx.db.test_table().insert(row);
        
        LOG_INFO("Operation completed successfully, resource will be cleaned up");
        // cleanup_guard will automatically clean up when going out of scope
        
    } catch (const std::exception& e) {
        LOG_ERROR("Operation failed: " + std::string(e.what()) + " - resource will be cleaned up");
        // cleanup_guard will still clean up due to RAII
        throw; // Re-throw after logging
    }
}

/**
 * Demonstrates manual error code checking (for when you want explicit control).
 */
SPACETIMEDB_REDUCER(test_manual_error_check, ReducerContext ctx, const std::string& table_name) {
    LOG_INFO("Testing manual error code checking for table: " + table_name);
    
    try {
        // This would typically be done internally by TableHandle, but demonstrates manual checking
        // In a real scenario, you'd call a host function that returns a status code
        uint16_t status_code = 0; // Simulate getting this from a host call
        
        // For demonstration, simulate different error conditions based on table name
        if (table_name == "nonexistent") {
            status_code = static_cast<uint16_t>(Errno::NO_SUCH_TABLE);
        } else if (table_name == "decode_error") {
            status_code = static_cast<uint16_t>(Errno::BSATN_DECODE_ERROR);
        } else if (table_name == "no_transaction") {
            status_code = static_cast<uint16_t>(Errno::NOT_IN_TRANSACTION);
        }
        
        // Use automatic error checking
        check_error(status_code, "Failed to access table: " + table_name);
        
        LOG_INFO("Table access successful: " + table_name);
        
    } catch (const NoSuchTableException& e) {
        LOG_WARN("Table not found: " + std::string(e.what()));
    } catch (const BsatnDecodeException& e) {
        LOG_ERROR("BSATN decode error: " + std::string(e.what()));
    } catch (const NotInTransactionException& e) {
        LOG_ERROR("Transaction error: " + std::string(e.what()));
    } catch (const StdbException& e) {
        LOG_ERROR("Other SpacetimeDB error: " + std::string(e.what()) + 
                  " (code: " + std::to_string(e.error_value()) + ")");
    }
}

/**
 * Demonstrates exception hierarchy and error code inspection.
 */
SPACETIMEDB_REDUCER(test_exception_hierarchy, ReducerContext ctx, int16_t error_code_value) {
    LOG_INFO("Testing exception hierarchy with error code: " + std::to_string(error_code_value));
    
    try {
        // Convert numeric error code to enum and throw corresponding exception
        Errno error_code = static_cast<Errno>(error_code_value);
        
        // Demonstrate throwing specific error
        if (error_code != Errno::OK) {
            throw_error(error_code, "Demonstration error for code " + std::to_string(error_code_value));
        }
        
        LOG_INFO("No error to throw (code was OK)");
        
    } catch (const StdbException& e) {
        // Demonstrate polymorphic exception handling
        LOG_INFO("Caught SpacetimeDB exception: " + std::string(e.what()));
        LOG_DEBUG("Error code: " + std::to_string(e.error_value()));
        
        // Demonstrate specific exception type detection
        if (auto* specific = dynamic_cast<const NotInTransactionException*>(&e)) {
            LOG_DEBUG("Specifically caught NotInTransactionException");
        } else if (auto* specific = dynamic_cast<const BsatnDecodeException*>(&e)) {
            LOG_DEBUG("Specifically caught BsatnDecodeException");
        } else if (auto* specific = dynamic_cast<const NoSuchTableException*>(&e)) {
            LOG_DEBUG("Specifically caught NoSuchTableException");
        } else {
            LOG_DEBUG("Caught other SpacetimeDB exception type");
        }
    }
}

/**
 * Initialize the database with test data and error handling examples.
 */
SPACETIMEDB_REDUCER(init_error_test_db, ReducerContext ctx) {
    LOG_INFO("Initializing error handling test database");
    
    {
        LogStopwatch timer("error_test_initialization");
        
        try {
            // Insert some test data
            ctx.db.test_table().insert({1, "test_data_1"});
            ctx.db.test_table().insert({2, "test_data_2"});
            
            // Insert unique test data
            ctx.db.unique_table().insert({100, "unique_value_1"});
            ctx.db.unique_table().insert({101, "unique_value_2"});
            
            LOG_INFO("Error handling test database initialized successfully");
            
        } catch (const StdbException& e) {
            LOG_ERROR("Failed to initialize test database: " + std::string(e.what()));
            throw;
        }
    }
}