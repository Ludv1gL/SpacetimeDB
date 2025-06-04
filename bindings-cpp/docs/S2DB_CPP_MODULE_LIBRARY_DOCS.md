# SpacetimeDB C++ Module Library Documentation

## 1. Introduction

### Purpose
The SpacetimeDB C++ Module Library enables developers to build high-performance database modules using modern C++. It provides a complete toolkit for defining schemas, implementing business logic, and leveraging SpacetimeDB's unique architecture where application code runs inside the database.

### Key Features
- **Declarative Schema Definition**: Define tables using simple macros
- **Type-Safe Operations**: Compile-time checking for all database operations
- **Automatic Serialization**: BSATN serialization handled automatically
- **Advanced Features**: Constraints, RLS, scheduled reducers, transactions
- **100% Feature Parity**: Complete compatibility with Rust and C# modules
- **WebAssembly Target**: Compile to WASM for secure execution

### Prerequisites
- C++20 compatible compiler
- Emscripten SDK for WASM compilation
- CMake 3.20 or higher
- SpacetimeDB CLI

## 2. Getting Started

### Quick Example

```cpp
#include <spacetimedb/spacetimedb.h>

// Define a table
SPACETIMEDB_TABLE(User, users, public,
    autoinc<uint32_t> id;
    std::string username;
    std::string email
);

// Define a reducer
SPACETIMEDB_REDUCER(create_user, ctx, std::string username, std::string email) {
    if (username.empty() || email.empty()) {
        ctx.fail("Username and email are required");
        return;
    }
    
    User user{.username = username, .email = email};
    ctx.insert(user);
    
    spacetimedb::log("Created user: " + username);
}
```

### Project Structure

```
my_module/
├── CMakeLists.txt      # Build configuration
├── src/
│   └── lib.cpp         # Module implementation
└── build/              # Build output directory
```

### Building Your Module

1. Create a new module:
```bash
spacetime init --lang cpp my_module
```

2. Build with CMake:
```bash
mkdir build && cd build
emcmake cmake ..
make
```

3. Publish to SpacetimeDB:
```bash
spacetime publish my_module.wasm
```

## 3. Core Concepts

### Tables
Tables store your data with strong typing and automatic serialization:

```cpp
SPACETIMEDB_TABLE(Product, products, public,
    autoinc<uint32_t> id;
    std::string name;
    double price;
    uint32_t stock
);
```

### Reducers
Reducers are functions that modify database state:

```cpp
SPACETIMEDB_REDUCER(update_stock, ctx, uint32_t product_id, uint32_t new_stock) {
    auto product = ctx.find<Product>(product_id);
    if (!product) {
        ctx.fail("Product not found");
        return;
    }
    
    product->stock = new_stock;
    ctx.update(std::move(*product));
}
```

### CRUD Operations
The reducer context provides all database operations:

```cpp
// Insert
auto inserted = ctx.insert(row);

// Find by ID
auto found = ctx.find<TableType>(id);

// Select with predicate
auto results = ctx.select<TableType>([](const auto& row) {
    return row.field > value;
});

// Update
ctx.update(modified_row);

// Delete
ctx.delete_by_id<TableType>(id);
```

## 4. Advanced Features

### Indexes
Improve query performance with indexes:

```cpp
SPACETIMEDB_TABLE(User, users, public,
    autoinc<uint32_t> id;
    unique<std::string> email;     // Unique index
    btree<uint32_t> age;           // BTree index
    std::string name
);
```

### Constraints
Validate data with built-in constraints:

```cpp
SPACETIMEDB_VALIDATED_TABLE(Order, orders, public,
    ValidatedPrimaryKey<uint32_t> id;
    ValidatedRange<uint32_t, 1, 1000> quantity;
    ValidatedString<1, 255> customer_name;
    ValidatedPositive<double> total_price
);
```

### Row-Level Security
Control access at the row level:

```cpp
SPACETIMEDB_RLS_TABLE(Document, documents, private,
    uint32_t id;
    spacetimedb::Identity owner;
    std::string content,
    
    // RLS policy - only owner can access
    [](const Document& row, const spacetimedb::Identity& caller) {
        return row.owner == caller;
    }
);
```

### Scheduled Reducers
Run reducers on a schedule:

```cpp
// Run every hour
SPACETIMEDB_SCHEDULED_REDUCER(hourly_cleanup, ctx, spacetimedb::duration::hours(1)) {
    ctx.delete_where<Session>([](const Session& s) {
        return s.expired();
    });
}

// Run at specific time
SPACETIMEDB_SCHEDULED_AT(daily_report, ctx, "00:00:00") {
    // Generate daily report
}
```

### Transactions
Ensure atomicity with transactions:

```cpp
SPACETIMEDB_REDUCER(transfer_funds, ctx, uint32_t from_id, uint32_t to_id, double amount) {
    ctx.transaction([&] {
        auto from = ctx.find<Account>(from_id);
        auto to = ctx.find<Account>(to_id);
        
        if (!from || !to) {
            ctx.fail("Account not found");
            return;
        }
        
        if (from->balance < amount) {
            ctx.fail("Insufficient funds");
            return;
        }
        
        from->balance -= amount;
        to->balance += amount;
        
        ctx.update(std::move(*from));
        ctx.update(std::move(*to));
    });
}
```

## 5. Type System

### Basic Types
All standard C++ types are supported:
- Integers: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- Floating point: `float`, `double`
- Strings: `std::string`
- Containers: `std::vector<T>`, `std::optional<T>`, `std::map<K,V>`

### Special Types
SpacetimeDB-specific types:
- `spacetimedb::Identity` - User identity
- `spacetimedb::Address` - Database address
- `spacetimedb::Timestamp` - Time points
- `spacetimedb::TimeDuration` - Time intervals
- `spacetimedb::ConnectionId` - Client connections

### Custom Types
Register custom types for use in tables:

```cpp
struct UserProfile {
    std::string bio;
    std::vector<std::string> interests;
    std::optional<std::string> website;
};

SPACETIMEDB_REGISTER_TYPE(UserProfile,
    SPACETIMEDB_FIELD(UserProfile, bio),
    SPACETIMEDB_FIELD(UserProfile, interests),
    SPACETIMEDB_FIELD(UserProfile, website)
);
```

## 6. Built-in Reducers

### Init Reducer
Runs when module is first deployed:

```cpp
SPACETIMEDB_INIT_REDUCER(ctx) {
    // Initialize default data
    Config config{.version = "1.0", .initialized = true};
    ctx.insert(config);
}
```

### Connect/Disconnect
Handle client connections:

```cpp
SPACETIMEDB_CONNECT_REDUCER(ctx) {
    Session session{
        .connection_id = ctx.connection_id(),
        .identity = ctx.sender(),
        .connected_at = spacetimedb::now()
    };
    ctx.insert(session);
}

SPACETIMEDB_DISCONNECT_REDUCER(ctx) {
    ctx.delete_where<Session>([&](const Session& s) {
        return s.connection_id == ctx.connection_id();
    });
}
```

## 7. Error Handling

Handle errors gracefully:

```cpp
SPACETIMEDB_REDUCER(safe_operation, ctx, uint32_t id, std::string value) {
    try {
        auto item = ctx.find<Item>(id);
        if (!item) {
            ctx.fail("Item not found");
            return;
        }
        
        if (!validate_value(value)) {
            ctx.fail("Invalid value");
            return;
        }
        
        item->value = value;
        ctx.update(std::move(*item));
        
    } catch (const std::exception& e) {
        ctx.fail(std::string("Operation failed: ") + e.what());
    }
}
```

## 8. Best Practices

1. **Validate Input**: Always validate reducer arguments
2. **Handle Missing Data**: Check optional returns from queries
3. **Use Appropriate Indexes**: Design tables with query patterns in mind
4. **Log Important Events**: Use `spacetimedb::log()` for debugging
5. **Keep Reducers Focused**: Each reducer should do one thing well
6. **Use Transactions**: For operations that must be atomic
7. **Consider Performance**: Use batch operations when possible

## 9. Debugging

### Logging
Use the built-in logging system:

```cpp
spacetimedb::log("Info message");
spacetimedb::log_warn("Warning message");
spacetimedb::log_error("Error message");
spacetimedb::log_debug("Debug message");
```

### Performance Tracking
Measure operation timing:

```cpp
spacetimedb::LogStopwatch sw("operation_name");
// ... perform operation ...
// Automatically logs duration when sw goes out of scope
```

## 10. Migration and Versioning

Handle schema evolution:

```cpp
SPACETIMEDB_MODULE_METADATA(
    name = "my_module",
    version = "2.0.0",
    description = "My SpacetimeDB module"
);

// Handle migration in init reducer
SPACETIMEDB_INIT_REDUCER(ctx) {
    auto config = ctx.find<Config>(0);
    if (config && config->version == "1.0.0") {
        // Perform migration
        migrate_v1_to_v2(ctx);
    }
}
```

## 11. Examples

Complete working examples are available in the `examples/` directory:
- `simple_module/` - Basic CRUD operations
- `module_test/` - Comprehensive feature testing
- `quickstart_cpp_kv/` - Key-value store implementation

## 12. Troubleshooting

### Common Issues

1. **Compilation Errors**: Ensure you're using C++20
2. **Linking Issues**: Check CMakeLists.txt configuration
3. **WASM Size**: Use release builds for smaller binaries
4. **Performance**: Profile with browser dev tools

### Getting Help

- [Discord Community](https://discord.gg/spacetimedb)
- [GitHub Issues](https://github.com/clockworklabs/SpacetimeDB/issues)
- [Documentation](https://docs.spacetimedb.com)

## 13. API Reference

For detailed API documentation, see the header files in `library/include/spacetimedb/`:
- `spacetimedb.h` - Main include file
- `types.h` - Core type definitions
- `module.h` - Module macros and registration
- `table_ops.h` - Table operation details
- `bsatn_all.h` - Serialization system

## Conclusion

The SpacetimeDB C++ Module Library provides everything needed to build high-performance database modules in C++. With its intuitive API, comprehensive feature set, and excellent performance characteristics, it's an ideal choice for developers who want to leverage C++'s power within SpacetimeDB's innovative architecture.