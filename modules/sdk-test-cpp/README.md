# SDK Test C++ Module

This module is a comprehensive test suite for the SpacetimeDB C++ Module Library. It replicates the functionality of `sdk-test` (Rust) and `sdk-test-cs` (C#) using the C++ Module Library.

## Current Status

Successfully implemented:
- ✅ Minimal C++ module without WASI dependencies
- ✅ Working reducer with proper ABI integration
- ✅ Custom minimal SDK (spacetimedb_minimal.h) that avoids C++ standard library
- ✅ Console logging without iostream

In progress:
- 🚧 Table support (BSATN encoding issues with AlgebraicType)
- 🚧 Full parity with Rust/C# test modules

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

4. **Call a reducer**:
   ```bash
   spacetime call sdktestcpp no_op
   ```

5. **Check logs**:
   ```bash
   spacetime logs sdktestcpp --follow
   ```

## Implementation Notes

### WASI Dependencies
The full SpacetimeDB C++ library currently has WASI dependencies from:
- C++ standard library (iostream, exceptions, etc.)
- Memory allocation routines
- File I/O operations

To work around this, we've created a minimal SDK (`spacetimedb_minimal.h`) that:
- Uses only C-style imports/exports
- Avoids C++ standard library features
- Implements basic BSATN serialization manually
- Provides simple logging without iostream

### Module Structure

- `src/lib.cpp` - Main module implementation using minimal SDK
- `src/spacetimedb_minimal.h` - Minimal SDK without WASI dependencies
- `src/lib_minimal.cpp` - Ultra-minimal module for testing
- `CMakeLists.txt` - Build configuration with proper Emscripten flags

### Emscripten Flags

Critical flags to avoid WASI dependencies:
```cmake
-s STANDALONE_WASM=1    # Standalone WebAssembly module
-s EXPORT_ALL=1         # Export all functions
-s ERROR_ON_UNDEFINED_SYMBOLS=0  # Allow undefined imports
-s DISABLE_EXCEPTION_CATCHING=1  # No C++ exceptions
-s MALLOC=emmalloc      # Simple allocator
-s WASM=1              # Output WebAssembly
--no-entry             # No main function
-s FILESYSTEM=0        # No filesystem support
```

## Features Tested

### Data Types
- All primitive types (u8-u256, i8-i256, bool, f32, f64, string)
- SpacetimeDB special types (Identity, ConnectionId, Timestamp, TimeDuration)
- Vectors of all types
- Optional types
- Custom structs (unit, simple, complex)
- Enums (simple and with payloads)

### Table Features (Planned)
- Basic tables
- Tables with unique constraints
- Tables with primary keys
- Tables with indexes (btree)
- Auto-increment fields
- Scheduled tables

### Operations
- ✅ Reducer calls
- ✅ Console logging
- 🚧 Insert operations for all types
- 🚧 Update operations for unique/primary key tables
- 🚧 Delete operations
- 🚧 Batch operations
- 🚧 Complex queries

### Advanced Features (Planned)
- Client visibility filters
- Scheduled reducers
- Connection/identity tracking
- Timestamp tracking