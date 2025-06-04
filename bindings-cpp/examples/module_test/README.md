# SpacetimeDB C++ Module Test

This example demonstrates comprehensive feature coverage of the SpacetimeDB C++ SDK, mirroring the functionality found in:
- `modules/module-test/src/lib.rs` (Rust reference)
- `modules/module-test-cs/Lib.cs` (C# reference)

## Features Demonstrated

### Tables
- **Person**: Public table with auto-increment primary key and BTree index on age
- **TestA-D**: Various table types testing different data structures
- **Point**: Simple coordinate type
- **Player**: Complex table with multiple special types (Identity, ConnectionId, Timestamp)
- **SimpleTable**: Basic table with Identity primary key
- **EveryPrimitiveStruct**: Tests all primitive types
- **EveryVecStruct**: Tests vector collections of all types

### Data Types
- All primitive types (u8-u256, i8-i256, bool, f32, f64)
- String and byte arrays
- Vector collections
- Optional types (std::optional)
- Special types (Identity, ConnectionId, Timestamp)
- Enums (TestE, Foobar)
- Sum types (TestF - tagged union pattern)

### Reducers
- Insert operations for all table types
- Query operations with filtering
- Delete operations (by ID and by owner)
- Testing all argument types
- Connection info access
- Init reducer for module initialization
- Clear all tables functionality

### Indexes
- BTree index on Person.age
- Unique index on Player.owner
- Primary key indexes with auto-increment

## Building

```bash
# Using the build script
./build.sh

# Or compile directly with emcc
emcc -std=c++20 -I../../sdk/include -s STANDALONE_WASM=1 -s FILESYSTEM=0 \
     -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry \
     -o module_test.wasm src/module_test.cpp
```

## Publishing and Testing

```bash
# Publish the module
spacetime publish --bin-path module_test.wasm module-test-cpp

# Test insert operations
spacetime call module-test-cpp insert_person "Alice" 30
spacetime call module-test-cpp insert_person "Bob" 25
spacetime call module-test-cpp insert_player "Player1" 100 200

# Test queries
spacetime call module-test-cpp query_person_by_name "Alice"
spacetime call module-test-cpp query_players_by_owner

# Test special types
spacetime call module-test-cpp test_connection_info
spacetime call module-test-cpp insert_every_primitive "test-data"

# SQL queries
spacetime sql module-test-cpp "SELECT * FROM person"
spacetime sql module-test-cpp "SELECT * FROM player"

# Clean up
spacetime call module-test-cpp clear_all_tables
```

## Current Status

The module compiles successfully and generates all expected exports. However, there are some limitations:

### Working Features
- Table definitions using X-Macro pattern
- Basic reducer registration and implementation
- Insert operations for all table types
- All primitive types (with placeholders for 128/256-bit integers)
- Vector/array types
- Basic logging functionality

### Current Limitations
- **Module Description Format**: The `__describe_module__` function needs updates to generate correct BSATN format for SpacetimeDB v9
- **Query Operations**: `select_all()` method not yet implemented in TableHandle
- **Delete Operations**: `remove()` method not yet implemented in TableHandle
- **Special Types**: Identity, ConnectionId, and Timestamp types are declared but not implemented (using uint64_t placeholders)
- **ReducerContext**: Currently only has `db` member; missing `sender`, `connection_id`, and `timestamp` fields
- **Scheduled Reducers**: Not yet supported (like `repeating_test` in Rust)
- **Index Definitions**: Multi-column and unique indexes not yet supported
- **Enum Serialization**: Complex enum types (sum types) need proper BSATN serialization

### Next Steps
1. Fix the module description format to match SpacetimeDB's expectations
2. Implement missing TableHandle methods (select_all, remove, update)
3. Implement special types (Identity, ConnectionId, Timestamp)
4. Add missing ReducerContext fields
5. Support for scheduled reducers and advanced features