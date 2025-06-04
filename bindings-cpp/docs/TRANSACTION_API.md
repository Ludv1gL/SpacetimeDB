# SpacetimeDB C++ Module Library Transaction API

## Overview

The SpacetimeDB C++ Module Library provides a comprehensive transaction control API that offers ACID properties and advanced transaction management features. While SpacetimeDB automatically manages transactions at the reducer level, this API provides explicit control for complex scenarios requiring fine-grained transaction management.

## Key Features

- **ACID Compliance**: Full support for Atomicity, Consistency, Isolation, and Durability
- **Multiple Isolation Levels**: From ReadUncommitted to Serializable
- **Savepoints**: Nested transaction support via savepoints
- **RAII Guards**: Automatic transaction cleanup with C++ RAII patterns
- **Retry Logic**: Built-in retry mechanisms for handling serialization conflicts
- **Metrics**: Detailed transaction performance metrics
- **Optimistic Locking**: Support for version-based concurrency control

## Basic Usage

### Simple Transaction

```cpp
SPACETIMEDB_REDUCER(simple_transaction, ReducerContext ctx) {
    // Begin a transaction
    auto tx = TransactionManager::begin(ctx);
    
    // Perform database operations
    auto table = tx.database().table<MyTable>("my_table");
    MyTable entity{1, "data", 100};
    table.insert(entity);
    
    // Commit the transaction
    tx.commit();
}
```

### Using Transaction Guards (RAII)

```cpp
SPACETIMEDB_REDUCER(guarded_transaction, ReducerContext ctx) {
    // TransactionGuard automatically rolls back if not explicitly committed
    TransactionGuard guard(TransactionManager::begin(ctx));
    
    // Perform operations
    auto table = guard->database().table<MyTable>("my_table");
    table.insert({2, "data", 200});
    
    // Explicitly commit
    guard.commit();
    // If commit() is not called, transaction automatically rolls back
}
```

### With Automatic Retry

```cpp
SPACETIMEDB_REDUCER(retry_transaction, ReducerContext ctx) {
    auto result = TransactionManager::with_transaction(ctx, [&](Transaction& tx) {
        // This lambda will be retried up to 3 times on serialization errors
        auto table = tx.database().table<MyTable>("my_table");
        table.insert({3, "data", 300});
        return true;
    });
}
```

## Isolation Levels

The transaction API supports all standard SQL isolation levels:

```cpp
TransactionOptions options;
options.isolation_level = IsolationLevel::Serializable; // Default

// Available levels:
// - IsolationLevel::ReadUncommitted
// - IsolationLevel::ReadCommitted
// - IsolationLevel::RepeatableRead
// - IsolationLevel::Snapshot
// - IsolationLevel::Serializable

auto tx = TransactionManager::begin(ctx, options);
```

### Isolation Level Guarantees

| Level | Dirty Read | Non-Repeatable Read | Phantom Read | Write Skew |
|-------|------------|-------------------|--------------|------------|
| ReadUncommitted | Possible | Possible | Possible | Possible |
| ReadCommitted | Prevented | Possible | Possible | Possible |
| RepeatableRead | Prevented | Prevented | Possible | Possible |
| Snapshot | Prevented | Prevented | Prevented | Possible |
| Serializable | Prevented | Prevented | Prevented | Prevented |

## Savepoints

Savepoints provide nested transaction capabilities:

```cpp
SPACETIMEDB_REDUCER(savepoint_example, ReducerContext ctx) {
    TransactionGuard guard(TransactionManager::begin(ctx));
    
    // First operation
    table.insert(entity1);
    guard->savepoint("after_first_insert");
    
    try {
        // Risky operation
        table.insert(risky_entity);
        guard->savepoint("after_risky");
        
        // Another operation
        table.insert(entity2);
        
    } catch (const std::exception& e) {
        // Rollback to savepoint, keeping entity1
        guard->rollback_to_savepoint("after_first_insert");
    }
    
    guard.commit();
}
```

## Read-Only Transactions

For queries that don't modify data:

```cpp
SPACETIMEDB_REDUCER(readonly_query, ReducerContext ctx) {
    TransactionManager::with_readonly_transaction(ctx, [&](Transaction& tx) {
        auto table = tx.database().table<MyTable>("my_table");
        
        for (const auto& row : table.iter()) {
            // Process read-only data
            spacetimedb::log(row.toString(), LogLevel::Info);
        }
        
        return 0;
    });
}
```

## Transaction Metrics

Monitor transaction performance:

```cpp
SPACETIMEDB_REDUCER(metrics_example, ReducerContext ctx) {
    TransactionGuard guard(TransactionManager::begin(ctx));
    
    // Perform operations...
    
    guard.commit();
    
    // Get metrics
    auto metrics = guard->metrics();
    spacetimedb::log("Rows read: " + std::to_string(metrics.rows_read), LogLevel::Info);
    spacetimedb::log("Rows written: " + std::to_string(metrics.rows_written), LogLevel::Info);
    spacetimedb::log("Duration: " + std::to_string(metrics.duration.count()) + "ms", LogLevel::Info);
}
```

## Error Handling

The transaction API provides specific exception types:

```cpp
try {
    auto tx = TransactionManager::begin(ctx);
    // Operations...
    tx.commit();
    
} catch (const DeadlockError& e) {
    // Handle deadlock
} catch (const IsolationViolationError& e) {
    // Handle isolation violation
} catch (const SerializationError& e) {
    // Handle serialization conflict
} catch (const TransactionError& e) {
    // Handle general transaction error
}
```

## Optimistic Locking Pattern

For implementing optimistic concurrency control:

```cpp
struct VersionedEntity {
    uint32_t id;
    std::string data;
    uint64_t version;
};

SPACETIMEDB_REDUCER(optimistic_update, ReducerContext ctx, uint32_t id) {
    const int max_retries = 5;
    
    for (int retry = 0; retry < max_retries; ++retry) {
        try {
            TransactionGuard guard(TransactionManager::begin(ctx));
            
            // Read current version
            auto entity = table.find_by_id(id);
            auto original_version = entity.version;
            
            // Perform updates
            entity.data = "updated";
            
            // Check version hasn't changed
            if (entity.version != original_version) {
                throw SerializationError();
            }
            
            // Update with new version
            entity.version++;
            table.update(entity);
            
            guard.commit();
            break;
            
        } catch (const SerializationError&) {
            if (retry == max_retries - 1) throw;
            std::this_thread::sleep_for(std::chrono::milliseconds(10 * (1 << retry)));
        }
    }
}
```

## Transaction Options

Configure transaction behavior:

```cpp
TransactionOptions options;
options.isolation_level = IsolationLevel::Snapshot;
options.timeout = std::chrono::seconds(30);  // 30 second timeout
options.readonly = false;
options.name = "my_transaction";  // For debugging

auto tx = TransactionManager::begin(ctx, options);
```

## Best Practices

1. **Use Transaction Guards**: Always prefer `TransactionGuard` over manual transaction management
2. **Keep Transactions Short**: Minimize the time a transaction is active
3. **Handle Errors Appropriately**: Always catch and handle transaction exceptions
4. **Use Appropriate Isolation Levels**: Don't use Serializable unless necessary
5. **Avoid Long-Running Operations**: Don't perform I/O or expensive computations within transactions
6. **Test Retry Logic**: Always test code that uses automatic retry mechanisms

## Implementation Notes

- The transaction API is built on top of SpacetimeDB's implicit transaction model
- Each reducer call in SpacetimeDB automatically runs in a transaction
- This API provides logical transaction control within that context
- Actual rollback is achieved by making the reducer fail
- Savepoints are logical constructs maintained by the C++ Module Library
- Transaction metrics integrate with SpacetimeDB's internal metrics system

## Future Extensions

The transaction API is designed to support future enhancements:

- **Distributed Transactions**: Two-phase commit protocol support
- **Transaction Priorities**: Priority-based scheduling
- **Deadlock Detection**: Advanced deadlock detection and resolution
- **Transaction Replay**: Ability to replay failed transactions
- **Snapshot Management**: Manual snapshot creation and management