# C++ Module Library Migration Guide

This guide helps you migrate from older versions of the SpacetimeDB C++ Module Library to the current version.

## Overview

The SpacetimeDB C++ Module Library provides a comprehensive API for building SpacetimeDB modules in C++. The library has evolved significantly to provide better type safety, cleaner APIs, and full feature parity with Rust and C# module libraries.

## Current Architecture

The modern C++ Module Library consists of:

1. **Core Module API** (`spacetimedb/module.h`): Main entry point for module development
2. **Table Operations** (`spacetimedb/table_ops.h`): CRUD operations on tables
3. **BSATN Serialization** (`spacetimedb/bsatn_all.h`): Binary serialization for all types
4. **Type System** (`spacetimedb/types.h`): Core types like `Identity`, `Address`, etc.
5. **Advanced Features**:
   - Constraints and validation (`spacetimedb/constraint_validation.h`)
   - Row-level security (`spacetimedb/rls.h`)
   - Scheduled reducers (`spacetimedb/schedule_reducer.h`)
   - Transactions (`spacetimedb/transaction.h`)
   - Versioning support (`spacetimedb/versioning.h`)

## Migration from Legacy Systems

### From Manual FFI Exports

**Old approach:**
```cpp
extern "C" {
    void __reducer_insert_person(const uint8_t* data, size_t len) {
        // Manual deserialization and logic
    }
}
```

**New approach:**
```cpp
SPACETIMEDB_REDUCER(insert_person, ctx, std::string name, uint32_t age) {
    Person person{.name = name, .age = age};
    ctx.insert(person);
}
```

### From Manual Table Registration

**Old approach:**
```cpp
// Manual schema definitions and registration
```

**New approach:**
```cpp
SPACETIMEDB_TABLE(Person, person, public, autoinc<uint32_t> id; std::string name; uint32_t age);
```

## Key Improvements

1. **Macro-based API**: Simple, declarative syntax for tables and reducers
2. **Automatic Serialization**: BSATN serialization is generated automatically
3. **Type Safety**: Strong typing throughout the API
4. **Feature Parity**: All features available in Rust/C# are now in C++
5. **Better Error Handling**: Comprehensive error reporting and validation

## Quick Start Example

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

// Define an update reducer
SPACETIMEDB_REDUCER(update_email, ctx, uint32_t user_id, std::string new_email) {
    auto user = ctx.find<User>(user_id);
    if (!user) {
        ctx.fail("User not found");
        return;
    }
    
    user->email = new_email;
    ctx.update(std::move(*user));
}
```

## Advanced Features

### Constraints
```cpp
SPACETIMEDB_VALIDATED_TABLE(Product, products, public,
    ValidatedPrimaryKey<uint32_t> id;
    ValidatedUnique<std::string> sku;
    ValidatedString<1, 100> name;
    ValidatedRange<double, 0, 1000000> price
);
```

### Scheduled Reducers
```cpp
SPACETIMEDB_SCHEDULED_REDUCER(daily_cleanup, ctx) {
    // Runs on a schedule
    ctx.delete_where<Session>([](const Session& s) {
        return s.last_active < spacetimedb::now() - spacetimedb::days(7);
    });
}
```

### Row-Level Security
```cpp
SPACETIMEDB_RLS_TABLE(PrivateData, private_data, private,
    uint32_t id;
    spacetimedb::Identity owner;
    std::string data,
    
    // RLS policy
    [](const PrivateData& row, const spacetimedb::Identity& caller) {
        return row.owner == caller;
    }
);
```

## Best Practices

1. **Use the macros**: They handle all the boilerplate and ensure correctness
2. **Leverage type safety**: Use the provided types like `Identity`, `Address`, etc.
3. **Handle errors properly**: Use `ctx.fail()` for validation errors
4. **Follow naming conventions**: Use snake_case for table and reducer names
5. **Test thoroughly**: The library includes comprehensive testing utilities

## Further Resources

- See `examples/` directory for complete working examples
- Review `BUILTIN_REDUCERS.md` for system reducer documentation
- Check `TABLE_OPERATIONS_GUIDE.md` for detailed CRUD operations
- Refer to individual feature documentation in `docs/`