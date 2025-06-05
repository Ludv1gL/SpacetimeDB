# SDK Test C++ Module

This module is a comprehensive test suite for the SpacetimeDB C++ Module Library. It replicates the functionality of `sdk-test` (Rust) and `sdk-test-cs` (C#) using the C++ Module Library.

## Features Tested

### Data Types
- All primitive types (u8-u256, i8-i256, bool, f32, f64, string)
- SpacetimeDB special types (Identity, ConnectionId, Timestamp, TimeDuration)
- Vectors of all types
- Optional types
- Custom structs (unit, simple, complex)
- Enums (simple and with payloads)

### Table Features
- Basic tables
- Tables with unique constraints
- Tables with primary keys
- Tables with indexes (btree)
- Auto-increment fields
- Scheduled tables

### Operations
- Insert operations for all types
- Update operations for unique/primary key tables
- Delete operations
- Batch operations
- Complex queries

### Advanced Features
- Client visibility filters
- Scheduled reducers
- Connection/identity tracking
- Timestamp tracking

## Building

### Prerequisites
- Emscripten SDK (properly activated with `source emsdk_env.sh`)
- CMake 3.20+
- C++20 compiler
- SpacetimeDB CLI

### Build Steps

```bash
# Using the build script
./build.sh

# Or manually
mkdir -p build && cd build
emcmake cmake ..
emmake make
```

The output WASM module will be at `build/sdk_test_cpp.wasm`.

### Publishing and Verification

1. **Start SpacetimeDB** (if not already running):
   ```bash
   spacetime start
   ```

2. **Publish the module**:
   ```bash
   spacetime publish sdktestcpp -b build/sdk_test_cpp.wasm
   ```

3. **Verify the module**:
   ```bash
   spacetime describe sdktestcpp --json
   ```

### Current Status

The module currently includes:
- A minimal implementation (`lib_minimal.cpp`) that demonstrates basic module structure
- Proper WASM compilation without WASI dependencies
- Correct SpacetimeDB ABI exports (`__describe_module__`, `__call_reducer__`)

The full implementation with tables and reducers is in progress.

## Module Structure

- `src/lib.cpp` - Main module implementation
- `CMakeLists.txt` - Build configuration
- `build.sh` - Convenience build script

## Testing

This module is designed to be used with the SpacetimeDB SDK test suite to verify that the C++ Module Library provides the same functionality as the Rust and C# implementations.