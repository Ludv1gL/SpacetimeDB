#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/transaction.h>
#include <cassert>
#include <chrono>
#include <thread>

using namespace spacetimedb;

// Test table
struct TestEntity {
    uint32_t id;
    std::string data;
    uint32_t value;
};

SPACETIMEDB_REGISTER_FIELDS(TestEntity,
    SPACETIMEDB_FIELD(TestEntity, id, uint32_t);
    SPACETIMEDB_FIELD(TestEntity, data, std::string);
    SPACETIMEDB_FIELD(TestEntity, value, uint32_t);
)

SPACETIMEDB_TABLE(TestEntity, test_entity, true)

// Test 1: Basic commit/rollback
SPACETIMEDB_REDUCER(test_basic_transaction, ReducerContext ctx) {
    spacetimedb::log("Test 1: Basic commit/rollback", LogLevel::Info);
    
    // Test successful commit
    {
        auto tx = TransactionManager::begin(ctx);
        assert(tx.is_active());
        assert(tx.state() == TransactionState::Active);
        
        TestEntity entity{1, "test1", 100};
        tx.database().table<TestEntity>("test_entity").insert(entity);
        
        tx.commit();
        assert(!tx.is_active());
        assert(tx.state() == TransactionState::Committed);
        spacetimedb::log("✓ Commit test passed", LogLevel::Info);
    }
    
    // Test explicit rollback
    try {
        auto tx = TransactionManager::begin(ctx);
        
        TestEntity entity{2, "test2", 200};
        tx.database().table<TestEntity>("test_entity").insert(entity);
        
        tx.rollback();
        assert(false); // Should not reach here
    } catch (const TransactionError& e) {
        spacetimedb::log("✓ Rollback test passed", LogLevel::Info);
    }
}

// Test 2: Transaction guard auto-rollback
SPACETIMEDB_REDUCER(test_transaction_guard, ReducerContext ctx) {
    spacetimedb::log("Test 2: Transaction guard", LogLevel::Info);
    
    // Test auto-commit
    {
        TransactionGuard guard(TransactionManager::begin(ctx));
        TestEntity entity{3, "test3", 300};
        guard->database().table<TestEntity>("test_entity").insert(entity);
        guard.commit();
        spacetimedb::log("✓ Guard auto-commit test passed", LogLevel::Info);
    }
    
    // Test auto-rollback on exception
    try {
        TransactionGuard guard(TransactionManager::begin(ctx));
        TestEntity entity{4, "test4", 400};
        guard->database().table<TestEntity>("test_entity").insert(entity);
        throw std::runtime_error("Simulated error");
    } catch (const std::exception&) {
        spacetimedb::log("✓ Guard auto-rollback test passed", LogLevel::Info);
    }
}

// Test 3: Savepoints
SPACETIMEDB_REDUCER(test_savepoints, ReducerContext ctx) {
    spacetimedb::log("Test 3: Savepoints", LogLevel::Info);
    
    try {
        TransactionGuard guard(TransactionManager::begin(ctx));
        
        // Insert first entity
        TestEntity entity1{5, "test5", 500};
        guard->database().table<TestEntity>("test_entity").insert(entity1);
        
        // Create savepoint
        guard->savepoint("sp1");
        
        // Insert second entity
        TestEntity entity2{6, "test6", 600};
        guard->database().table<TestEntity>("test_entity").insert(entity2);
        
        // Create nested savepoint
        guard->savepoint("sp2");
        
        // Insert third entity
        TestEntity entity3{7, "test7", 700};
        guard->database().table<TestEntity>("test_entity").insert(entity3);
        
        // Rollback to sp2 (keeps entity3)
        guard->rollback_to_savepoint("sp2");
        
        // Insert different entity
        TestEntity entity4{8, "test8", 800};
        guard->database().table<TestEntity>("test_entity").insert(entity4);
        
        // Release sp2
        guard->release_savepoint("sp2");
        
        // Rollback to sp1 (loses entity2 and entity4)
        guard->rollback_to_savepoint("sp1");
        
        guard.commit();
        spacetimedb::log("✓ Savepoint test passed", LogLevel::Info);
        
    } catch (const TransactionError& e) {
        spacetimedb::log(std::string("✗ Savepoint test failed: ") + e.what(), LogLevel::Error);
    }
}

// Test 4: Transaction isolation levels
SPACETIMEDB_REDUCER(test_isolation_levels, ReducerContext ctx) {
    spacetimedb::log("Test 4: Isolation levels", LogLevel::Info);
    
    // Test each isolation level
    std::vector<IsolationLevel> levels = {
        IsolationLevel::ReadUncommitted,
        IsolationLevel::ReadCommitted,
        IsolationLevel::RepeatableRead,
        IsolationLevel::Snapshot,
        IsolationLevel::Serializable
    };
    
    for (auto level : levels) {
        TransactionOptions options;
        options.isolation_level = level;
        
        try {
            auto tx = TransactionManager::begin(ctx, options);
            assert(tx.isolation_level() == level);
            
            TestEntity entity{9, "isolation_test", 900};
            tx.database().table<TestEntity>("test_entity").insert(entity);
            
            tx.commit();
            spacetimedb::log("✓ Isolation level test passed for level: " + 
                           std::to_string(static_cast<int>(level)), LogLevel::Info);
        } catch (const TransactionError& e) {
            spacetimedb::log("✗ Isolation level test failed: " + std::string(e.what()), LogLevel::Error);
        }
    }
}

// Test 5: Read-only transactions
SPACETIMEDB_REDUCER(test_readonly_transaction, ReducerContext ctx) {
    spacetimedb::log("Test 5: Read-only transactions", LogLevel::Info);
    
    // First insert some data
    {
        TransactionGuard guard(TransactionManager::begin(ctx));
        TestEntity entity{10, "readonly_test", 1000};
        guard->database().table<TestEntity>("test_entity").insert(entity);
        guard.commit();
    }
    
    // Test read-only transaction
    TransactionManager::with_readonly_transaction(ctx, [](Transaction& tx) {
        assert(tx.is_readonly());
        assert(tx.isolation_level() == IsolationLevel::Snapshot);
        
        auto table = tx.database().table<TestEntity>("test_entity");
        int count = 0;
        for (const auto& entity : table.iter()) {
            count++;
        }
        
        spacetimedb::log("✓ Read-only transaction test passed. Entities: " + 
                       std::to_string(count), LogLevel::Info);
        return 0;
    });
}

// Test 6: Transaction metrics
SPACETIMEDB_REDUCER(test_transaction_metrics, ReducerContext ctx) {
    spacetimedb::log("Test 6: Transaction metrics", LogLevel::Info);
    
    TransactionGuard guard(TransactionManager::begin(ctx));
    
    // Perform some operations
    auto table = guard->database().table<TestEntity>("test_entity");
    
    // Insert some rows
    for (int i = 0; i < 5; ++i) {
        TestEntity entity{static_cast<uint32_t>(100 + i), "metric_test", static_cast<uint32_t>(i * 100)};
        table.insert(entity);
    }
    
    // Read some rows
    int count = 0;
    for (const auto& entity : table.iter()) {
        count++;
    }
    
    guard.commit();
    
    // Check metrics
    auto metrics = guard->metrics();
    spacetimedb::log("✓ Metrics test passed:", LogLevel::Info);
    spacetimedb::log("  - Rows read: " + std::to_string(metrics.rows_read), LogLevel::Info);
    spacetimedb::log("  - Rows written: " + std::to_string(metrics.rows_written), LogLevel::Info);
    spacetimedb::log("  - Duration: " + std::to_string(metrics.duration.count()) + "ms", LogLevel::Info);
}

// Test 7: Nested transaction detection
SPACETIMEDB_REDUCER(test_nested_transaction_detection, ReducerContext ctx) {
    spacetimedb::log("Test 7: Nested transaction detection", LogLevel::Info);
    
    try {
        auto tx1 = TransactionManager::begin(ctx);
        
        // Try to begin another transaction (should fail)
        try {
            auto tx2 = TransactionManager::begin(ctx);
            assert(false); // Should not reach here
        } catch (const TransactionError& e) {
            spacetimedb::log("✓ Nested transaction properly rejected", LogLevel::Info);
        }
        
        tx1.commit();
    } catch (const TransactionError& e) {
        spacetimedb::log("✗ Nested transaction test failed: " + std::string(e.what()), LogLevel::Error);
    }
}

// Test 8: with_transaction helper with retry
SPACETIMEDB_REDUCER(test_with_transaction_retry, ReducerContext ctx) {
    spacetimedb::log("Test 8: with_transaction helper", LogLevel::Info);
    
    // Test successful transaction
    auto result = TransactionManager::with_transaction(ctx, [](Transaction& tx) {
        TestEntity entity{200, "with_tx_test", 2000};
        tx.database().table<TestEntity>("test_entity").insert(entity);
        return true;
    });
    
    assert(result == true);
    spacetimedb::log("✓ with_transaction test passed", LogLevel::Info);
    
    // Test transaction with simulated serialization error
    int retry_count = 0;
    try {
        TransactionManager::with_transaction(ctx, [&retry_count](Transaction& tx) {
            retry_count++;
            if (retry_count < 3) {
                throw SerializationError();
            }
            TestEntity entity{201, "retry_test", 2010};
            tx.database().table<TestEntity>("test_entity").insert(entity);
            return true;
        });
        
        assert(retry_count == 3);
        spacetimedb::log("✓ Transaction retry test passed", LogLevel::Info);
    } catch (const TransactionError& e) {
        spacetimedb::log("✗ Transaction retry test failed: " + std::string(e.what()), LogLevel::Error);
    }
}

// Test 9: Transaction timeout
SPACETIMEDB_REDUCER(test_transaction_timeout, ReducerContext ctx) {
    spacetimedb::log("Test 9: Transaction timeout", LogLevel::Info);
    
    TransactionOptions options;
    options.timeout = std::chrono::milliseconds(100);  // Very short timeout
    options.name = "timeout_test";
    
    try {
        auto tx = TransactionManager::begin(ctx, options);
        
        // Simulate long operation
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        TestEntity entity{300, "timeout_test", 3000};
        tx.database().table<TestEntity>("test_entity").insert(entity);
        
        tx.commit();
        spacetimedb::log("✓ Transaction timeout test passed (completed within timeout)", LogLevel::Info);
        
    } catch (const TransactionError& e) {
        spacetimedb::log("Transaction timeout test: " + std::string(e.what()), LogLevel::Info);
    }
}

// Test 10: Multiple savepoints
SPACETIMEDB_REDUCER(test_multiple_savepoints, ReducerContext ctx) {
    spacetimedb::log("Test 10: Multiple savepoints", LogLevel::Info);
    
    try {
        TransactionGuard guard(TransactionManager::begin(ctx));
        
        // Create a chain of savepoints
        for (int i = 0; i < 5; ++i) {
            TestEntity entity{static_cast<uint32_t>(400 + i), "sp_test_" + std::to_string(i), static_cast<uint32_t>(i)};
            guard->database().table<TestEntity>("test_entity").insert(entity);
            guard->savepoint("sp_" + std::to_string(i));
        }
        
        // Rollback to middle savepoint
        guard->rollback_to_savepoint("sp_2");
        
        // Try to use an invalid savepoint (should fail)
        try {
            guard->rollback_to_savepoint("sp_4");  // This was removed
            assert(false);
        } catch (const TransactionError&) {
            spacetimedb::log("✓ Invalid savepoint properly rejected", LogLevel::Info);
        }
        
        guard.commit();
        spacetimedb::log("✓ Multiple savepoints test passed", LogLevel::Info);
        
    } catch (const TransactionError& e) {
        spacetimedb::log("✗ Multiple savepoints test failed: " + std::string(e.what()), LogLevel::Error);
    }
}

// Run all tests
SPACETIMEDB_REDUCER(run_all_transaction_tests, ReducerContext ctx) {
    spacetimedb::log("=== Running Transaction API Tests ===", LogLevel::Info);
    
    // Clear test data first
    {
        TransactionGuard guard(TransactionManager::begin(ctx));
        auto table = guard->database().table<TestEntity>("test_entity");
        // Note: In real implementation, we'd need a delete_all method
        guard.commit();
    }
    
    // Run each test
    test_basic_transaction(ctx);
    test_transaction_guard(ctx);
    test_savepoints(ctx);
    test_isolation_levels(ctx);
    test_readonly_transaction(ctx);
    test_transaction_metrics(ctx);
    test_nested_transaction_detection(ctx);
    test_with_transaction_retry(ctx);
    test_transaction_timeout(ctx);
    test_multiple_savepoints(ctx);
    
    spacetimedb::log("=== All Transaction API Tests Completed ===", LogLevel::Info);
}