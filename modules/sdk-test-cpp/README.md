# C++ SDK Test Modules

This directory contains comprehensive test modules for the SpacetimeDB C++ SDK, providing feature parity with the Rust and C# test modules.

## Modules

### sdk-test-cpp
The main comprehensive test module that exercises all C++ SDK features:

- **Type System**: Tests all primitive types, vectors, optionals, and custom structs
- **Tables**: Basic tables, constraints, indexes, and RLS policies
- **Reducers**: Various argument patterns, lifecycle reducers, scheduled reducers
- **BSATN Serialization**: Automatic type registration and serialization
- **Field Renaming**: Database column mapping
- **Row Level Security**: Policy enforcement

### sdk-test-connect-disconnect-cpp
Focused test module for connection lifecycle events:

- Client connection tracking
- Disconnection handling
- Event logging and querying
- Connection status management

## Building

To build these modules:

```bash
cd modules/sdk-test-cpp
mkdir build && cd build
cmake .. -DCMAKE_SYSTEM_NAME=WASI -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
make
```

The output will be a WASM module that can be deployed to SpacetimeDB.

## Features Tested

### Type System
- All primitive types (u8-u64, i8-i64, bool, f32, f64)
- 128/256-bit integer placeholders
- Strings and byte arrays
- Identity, ConnectionId, Timestamp, Duration
- Vectors of all types
- Optional types
- Custom structs with field registration

### Table Features
- Table registration with public/private access
- Primary keys and auto-increment
- Unique constraints
- Check constraints
- Single and composite indexes
- Row Level Security policies
- Scheduled tables

### Reducer Features
- No-argument reducers
- Single and multiple arguments
- Complex type arguments (vectors, optionals)
- Struct arguments
- Lifecycle reducers (init, connect, disconnect)
- Scheduled reducers
- Full BSATN argument deserialization

### Advanced Features
- Field renaming (C++ name vs database column)
- Module metadata and versioning
- Comprehensive logging
- Transaction-like operations
- Query and filter operations

## Comparison with Rust/C# Tests

These C++ test modules provide equivalent functionality to:
- `modules/sdk-test` (Rust)
- `modules/sdk-test-cs` (C#)
- `modules/sdk-test-connect-disconnect` (Rust)
- `modules/sdk-test-connect-disconnect-cs` (C#)

All language SDKs now have feature parity and can be tested with equivalent modules.

## Running Tests

1. Build the module
2. Deploy to SpacetimeDB:
   ```bash
   spacetime deploy sdk-test-cpp
   ```
3. Run test clients or use the CLI to call reducers
4. Verify all features work correctly

## Notes

- The C++ SDK uses placeholders for 128/256-bit integers until native support is added
- Enum types in C++ are simulated with tagged unions due to language limitations
- All features match the functionality of Rust and C# SDKs