# C++ SDK Development Lessons Learned

## Overview

This document captures the key lessons learned while working on achieving feature parity between the C++ SDK and the C#/Rust SDKs for SpacetimeDB.

## Major Issues Identified

### 1. Namespace Inconsistency

**Problem**: The SDK uses two different namespace conventions inconsistently:
- `spacetimedb` (lowercase) - 33 files
- `SpacetimeDb` (CamelCase) - 59 files

**Impact**: This causes compilation failures when trying to use the SDK, as types are defined in one namespace but referenced in another.

**Example**:
```cpp
// In bsatn/type_registry.h
namespace spacetimedb::bsatn {
    class ITypeRegistrar; // Forward declared here
}

// In internal/Module.h
namespace SpacetimeDb::Internal {
    class ITypeRegistrar { // Defined here - different namespace!
```

**Recommendation**: Standardize on `SpacetimeDb` (CamelCase) throughout the SDK since:
- It's the dominant pattern (59 vs 33 files)
- Internal implementation already uses it consistently
- It matches the C# convention

### 2. Module ABI and Exports

**Key Finding**: SpacetimeDB modules require two essential exports:
1. `__describe_module__(uint32_t sink)` - Provides module metadata
2. `__call_reducer__(...)` - Dispatches reducer calls

**Important**: The SDK is supposed to provide these exports automatically, but due to compilation issues, modules currently need to implement them manually.

### 3. Module Description Format

**Problem**: The module description must be encoded in a very specific BSATN format. Even small encoding errors cause module publishing to fail.

**Working Minimal Example**:
```cpp
// Minimal valid module description
uint8_t data[] = {
    1,  // RawModuleDef::V9 tag
    0, 0, 0, 0,  // typespace: empty vec
    0, 0, 0, 0,  // names: empty vec  
    0, 0, 0, 0,  // tables: empty vec
    0, 0, 0, 0,  // reducers: empty vec
    0, 0, 0, 0,  // types: empty vec
    0, 0, 0, 0,  // misc_exports: empty vec
    0, 0, 0, 0   // row_level_security: empty vec
};
```

### 4. Import Names

**Issue**: FFI import names must match exactly what SpacetimeDB expects:
- Use `console_log` not `_console_log`
- Use `bytes_sink_write` not `_bytes_sink_write`

### 5. Build System Complexity

**Problem**: The SDK is not header-only and requires linking multiple source files:
```bash
# Required SDK sources
${SDK_PATH}/src/abi/module_exports.cpp
${SDK_PATH}/src/internal/Module.cpp
${SDK_PATH}/src/internal/FFI.cpp
${SDK_PATH}/src/spacetimedb_sdk_table_registry.cpp
${SDK_PATH}/src/spacetimedb_sdk_types.cpp
${SDK_PATH}/src/bsatn/reader.cpp
${SDK_PATH}/src/bsatn/writer.cpp
# ... and more
```

**Current Status**: Due to namespace issues and compilation errors, building with the full SDK is currently not possible.

## Successfully Implemented Features

Despite the SDK compilation issues, we successfully implemented:

1. **BSATN Type Generation**: Complete AlgebraicType serialization
2. **Reducer Argument Deserialization**: Template-based system
3. **Field Renaming**: `SPACETIMEDB_FIELD_RENAMED` macro
4. **Row Level Security**: Full RLS policy system
5. **Module Metadata Generation**: `SPACETIMEDB_MODULE_METADATA` macro

## Working Module Example

Here's a minimal working module that bypasses SDK issues:

```cpp
#include <cstdint>
#include <cstring>

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void _console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t _bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Minimal module description
        uint8_t data[] = { /* ... */ };
        size_t len = sizeof(data);
        _bytes_sink_write(sink, data, &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(/* ... */) {
        return -1; // NO_SUCH_REDUCER
    }
}
```

## Build Commands

For manual module building without SDK:
```bash
emcc -std=c++20 \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -Wl,--export-dynamic \
    -o module.wasm \
    module.cpp
```

## Next Steps

1. **Fix Namespace Inconsistency**: Standardize on `SpacetimeDb` throughout
2. **Fix Compilation Issues**: Resolve type conflicts and missing includes
3. **Create Proper Build System**: CMake or static library for easy SDK usage
4. **Add Module Examples**: Working examples that use the SDK properly
5. **Test Coverage**: Comprehensive tests for all SDK features

## Module Publishing

Successfully published modules:
- `super-minimal`: Bare minimum exports, no functionality
- Others failed due to module description encoding issues

Key insight: The module description format is extremely sensitive to encoding errors. Even a single byte difference can cause publishing to fail.