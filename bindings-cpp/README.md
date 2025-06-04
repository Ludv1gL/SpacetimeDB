# SpacetimeDB C++ Module Library

Build SpacetimeDB modules in C++ with a clean, type-safe API that matches the power and simplicity of the Rust and C# module libraries.

## Overview

The SpacetimeDB C++ Module Library provides everything you need to build database modules in C++:
- Simple macro-based table and reducer definitions
- Automatic BSATN serialization
- Type-safe database operations
- Advanced features like constraints, RLS, and scheduled reducers
- 100% feature parity with Rust and C# module libraries

## Quick Start

```cpp
#include <spacetimedb/spacetimedb.h>

// Define a table
SPACETIMEDB_TABLE(User, users, public, 
    autoinc<uint32_t> id;
    std::string username;
    std::string email
);

// Define a reducer (like a stored procedure)
SPACETIMEDB_REDUCER(create_user, ctx, std::string username, std::string email) {
    User user{.username = username, .email = email};
    ctx.insert(user);
    spacetimedb::log("Created user: " + username);
}

// Query data
SPACETIMEDB_REDUCER(find_user, ctx, std::string username) {
    auto users = ctx.select<User>([&](const User& u) {
        return u.username == username;
    });
    
    if (!users.empty()) {
        spacetimedb::log("Found user: " + users[0].email);
    }
}
```

## Features

### Core Features
- **Tables**: Define strongly-typed database tables with simple macros
- **Reducers**: Write database logic that runs inside SpacetimeDB
- **CRUD Operations**: Type-safe insert, update, delete, and query operations
- **Indexes**: Support for BTree, Hash, Unique, and Primary Key indexes
- **Serialization**: Automatic BSATN serialization for all types

### Advanced Features
- **Constraints**: Validated fields with ranges, patterns, and custom validation
- **Row-Level Security**: Fine-grained access control per table
- **Scheduled Reducers**: Run reducers on a schedule
- **Transactions**: ACID-compliant transaction support
- **Client Visibility**: Control what data clients can subscribe to
- **Module Versioning**: Support for schema evolution

## Installation

### Prerequisites
- C++20 compatible compiler
- CMake 3.20 or higher
- SpacetimeDB CLI

### Building a Module

1. Create a new module:
```bash
spacetime init --lang cpp mymodule
```

2. Write your module code in `src/lib.cpp`

3. Build:
```bash
mkdir build && cd build
cmake ..
make
```

4. Publish to SpacetimeDB:
```bash
spacetime publish mymodule
```

## Documentation

- **[Migration Guide](MIGRATION_GUIDE.md)** - Migrate from older versions
- **[Table Operations Guide](TABLE_OPERATIONS_GUIDE.md)** - Detailed CRUD operations
- **[Built-in Reducers](BUILTIN_REDUCERS.md)** - System reducer documentation
- **[Parity Tracker](PARITY_TRACKER.md)** - Feature comparison with other languages
- **[Examples](examples/)** - Complete working examples

### Example Modules
- `examples/simple_module/` - Basic module with tables and reducers
- `examples/module_test/` - Comprehensive feature testing
- `examples/quickstart_cpp_kv/` - Key-value store example

### Advanced Documentation
- `docs/CONSTRAINT_VALIDATION.md` - Constraint system documentation
- `docs/SCHEDULE_REDUCER_GUIDE.md` - Scheduled reducer patterns
- `docs/TRANSACTION_API.md` - Transaction usage guide
- `docs/MODULE_VERSIONING_GUIDE.md` - Schema evolution

## API Reference

### Table Definition
```cpp
SPACETIMEDB_TABLE(TypeName, table_name, access_level, 
    field_type field_name;
    ...
);
```

### Reducer Definition
```cpp
SPACETIMEDB_REDUCER(reducer_name, ctx, arg1_type arg1, ...) {
    // Reducer logic
}
```

### Table Operations
```cpp
// Insert
ctx.insert(row);

// Update
ctx.update(row);

// Delete
ctx.delete_by_id<TableType>(id);

// Query
auto results = ctx.select<TableType>([](const TableType& row) {
    return /* condition */;
});
```

## Architecture

The C++ Module Library follows SpacetimeDB's module architecture:
1. Modules are compiled to WebAssembly
2. Tables are stored in SpacetimeDB's in-memory database
3. Reducers run inside the database when called
4. Clients connect via WebSocket for real-time updates

## Contributing

We welcome contributions! Please see the main SpacetimeDB repository for contribution guidelines.

## License

Apache 2.0 - See LICENSE file for details

## Support

- [Discord](https://discord.gg/spacetimedb)
- [Documentation](https://docs.spacetimedb.com)
- [GitHub Issues](https://github.com/clockworklabs/SpacetimeDB/issues)