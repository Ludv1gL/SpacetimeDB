# SDK Test C++ Module

This module is a comprehensive test suite for the SpacetimeDB C++ Module Library. It replicates the functionality of `sdk-test` (Rust) and `sdk-test-cs` (C#) using the C++ Module Library.

## Current Status

Successfully implemented:
- ✅ WASI shims to enable C++ standard library usage in WebAssembly modules
- ✅ Working C++ module that can use std::string, std::vector, std::algorithm, etc.
- ✅ Proper ABI integration with SpacetimeDB imports/exports
- ✅ Console logging that works through WASI shims
- ✅ Clean WASM output with only required SpacetimeDB imports (no WASI dependencies)

Demonstrated C++ standard library features:
- ✅ std::string for string manipulation
- ✅ std::vector for dynamic arrays
- ✅ std::algorithm (std::transform with lambdas)
- ✅ std::stringstream for string building
- ✅ std::cout (redirected through WASI shims)

In progress:
- 🚧 Table support with proper BSATN serialization
- 🚧 Integration with full SpacetimeDB C++ Module Library
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

### WASI Shim Approach
To enable C++ standard library usage in SpacetimeDB modules, we implemented WASI shims that:
- Stub out all WASI system calls with minimal implementations
- Redirect stdout/stderr output through SpacetimeDB's console_log
- Return success for most operations to avoid runtime errors
- Handle special cases like fd_write to enable std::cout

This approach allows us to:
- Use the full C++ standard library (STL containers, algorithms, iostream)
- Avoid WASI imports in the final WASM module
- Maintain compatibility with SpacetimeDB's security model

### Module Structure

- `src/lib.cpp` - Main module implementation demonstrating C++ standard library usage
- `src/wasi_shims.cpp` - WASI stub implementations to enable stdlib without WASI imports
- `CMakeLists.txt` - Build configuration with proper Emscripten flags
- `src/spacetimedb_minimal.h` - (Historical) Minimal SDK without stdlib
- `src/lib_minimal.cpp` - (Historical) Ultra-minimal module for testing

### Emscripten Flags

Critical flags for SpacetimeDB modules with C++ standard library:
```cmake
-s STANDALONE_WASM=1              # Standalone WebAssembly module
-s EXPORTED_FUNCTIONS=['_malloc','_free']  # Export memory management
-s ERROR_ON_UNDEFINED_SYMBOLS=0   # Allow SpacetimeDB imports
-s DISABLE_EXCEPTION_CATCHING=1   # Disable exception catching to avoid imports
-s DISABLE_EXCEPTION_THROWING=1   # Disable exception throwing
-s WASM=1                        # Output WebAssembly
--no-entry                       # No main function
-s FILESYSTEM=0                  # No filesystem support
-s ALLOW_MEMORY_GROWTH=1         # Allow dynamic memory allocation
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