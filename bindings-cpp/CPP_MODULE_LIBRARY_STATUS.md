# C++ Module Library Status

## Overview

The SpacetimeDB C++ Module Library is a comprehensive solution for building SpacetimeDB modules in C++. It provides full feature parity with the Rust and C# module libraries.

## Current Status: Production Ready ✅

The C++ Module Library is fully implemented and ready for production use.

## Features

### Core Features
- **Table Definitions**: Declarative table syntax with `SPACETIMEDB_TABLE`
- **Reducer Definitions**: Type-safe reducers with `SPACETIMEDB_REDUCER`
- **CRUD Operations**: Complete insert, update, delete, query support
- **Type System**: Full BSATN serialization for all types
- **Index Support**: BTree, Hash, Unique, Primary Key indexes

### Advanced Features
- **Constraints**: Validated fields with ranges, patterns, and custom validation
- **Row-Level Security**: Fine-grained access control policies
- **Scheduled Reducers**: Time-based and interval-based scheduling
- **Transactions**: ACID-compliant transaction support
- **Field Renaming**: Map C++ field names to database columns
- **Module Versioning**: Support for schema evolution
- **Client Visibility**: Control data visibility to clients

## Architecture

The library is organized into logical components:

```
bindings-cpp/
├── library/
│   ├── include/spacetimedb/    # Public headers
│   │   ├── spacetimedb.h       # Main include file
│   │   ├── types.h             # Core types
│   │   ├── module.h            # Module API
│   │   ├── table_ops.h         # Table operations
│   │   ├── bsatn_all.h         # Serialization
│   │   └── ...                 # Feature-specific headers
│   └── src/                    # Implementation files
├── examples/                   # Example modules
└── docs/                       # Documentation
```

## Quick Start

1. Include the main header:
```cpp
#include <spacetimedb/spacetimedb.h>
```

2. Define tables and reducers:
```cpp
SPACETIMEDB_TABLE(User, users, public, 
    autoinc<uint32_t> id;
    std::string name;
    std::string email
);

SPACETIMEDB_REDUCER(create_user, ctx, std::string name, std::string email) {
    User user{.name = name, .email = email};
    ctx.insert(user);
}
```

3. Build with CMake (see examples for CMakeLists.txt templates)

## Documentation

- **[README.md](README.md)** - Getting started guide
- **[MIGRATION_GUIDE.md](MIGRATION_GUIDE.md)** - Migration from older versions
- **[TABLE_OPERATIONS_GUIDE.md](TABLE_OPERATIONS_GUIDE.md)** - Detailed CRUD operations
- **[BUILTIN_REDUCERS.md](BUILTIN_REDUCERS.md)** - System reducer documentation
- **[PARITY_TRACKER.md](PARITY_TRACKER.md)** - Feature comparison

## Examples

Working examples demonstrate all features:
- `examples/simple_module/` - Basic tables and reducers
- `examples/module_test/` - Comprehensive feature testing
- `examples/module_library_test_cpp/` - Advanced features

## Testing

The library includes comprehensive test coverage:
- Unit tests for core functionality
- Integration tests with SpacetimeDB
- Example modules as functional tests

## Support

- [Discord Community](https://discord.gg/spacetimedb)
- [GitHub Issues](https://github.com/clockworklabs/SpacetimeDB/issues)
- [Documentation](https://docs.spacetimedb.com)

## License

Apache 2.0 - See LICENSE file for details