# SDK to Module Library Migration Summary

This document summarizes the changes made to update all references from "SDK" to "Module Library" in the bindings-cpp source code.

## Changes Made

### 1. Namespace Updates
- Changed `namespace sdk` to `namespace library` throughout the codebase
- Updated all references from `SpacetimeDb::sdk::` to `SpacetimeDb::library::`
- Updated `spacetimedb::sdk::` to `spacetimedb::library::`

### 2. Include Path Updates
- Changed all `#include <spacetimedb/sdk/...>` to `#include <spacetimedb/library/...>`
- Changed all `#include "spacetimedb/sdk/..."` to `#include "spacetimedb/library/..."`

### 3. File Updates

#### Header Files Updated:
- `/bindings-cpp/library/include/spacetimedb/library/spacetimedb_library_reducer.h`
- `/bindings-cpp/library/include/spacetimedb/library/spacetimedb_library_types.h`
- `/bindings-cpp/library/include/spacetimedb/library/spacetimedb_library_table_registry.h`
- `/bindings-cpp/library/include/spacetimedb/library/database.h`
- `/bindings-cpp/library/include/spacetimedb/library/table.h`
- `/bindings-cpp/library/include/spacetimedb/library/reducer_context.h`
- `/bindings-cpp/library/include/spacetimedb/library/reducer_context_enhanced.h`
- `/bindings-cpp/library/include/spacetimedb/module.h`
- `/bindings-cpp/library/include/spacetimedb/constraint_validation.h`
- `/bindings-cpp/library/include/spacetimedb/bsatn/reader.h`
- `/bindings-cpp/library/include/spacetimedb/bsatn/writer.h`
- `/bindings-cpp/library/include/spacetimedb/internal/IReducer.h`
- `/bindings-cpp/library/include/spacetimedb/internal/FFI.h`
- `/bindings-cpp/library/include/spacetimedb/spacetimedb.h`
- `/bindings-cpp/library/include/spacetimedb/macros.h`
- `/bindings-cpp/library/include/spacetimedb/field_registration.h`

#### Source Files Updated:
- `/bindings-cpp/library/src/spacetimedb_library_types.cpp`
- `/bindings-cpp/library/src/spacetimedb_library_types_impl.cpp`
- `/bindings-cpp/library/src/spacetimedb_library_table_registry.cpp`
- `/bindings-cpp/library/src/reducer_context.cpp`
- `/bindings-cpp/library/src/database.cpp`
- `/bindings-cpp/library/src/library/logging.cpp`
- `/bindings-cpp/library/src/library/exceptions.cpp`
- `/bindings-cpp/library/src/library/index_management.cpp`
- `/bindings-cpp/library/src/constraint_validation.cpp`
- `/bindings-cpp/library/src/bsatn/reader.cpp`
- `/bindings-cpp/library/src/internal/Module.cpp`
- `/bindings-cpp/library/src/abi/module_exports.cpp`
- `/bindings-cpp/library/src/logging.cpp`

#### Test Files Updated:
- `/bindings-cpp/library/test/test_bsatn.cpp`
- `/bindings-cpp/library/test/test_bsatn_existing.cpp`  
- `/bindings-cpp/library/test/test_bsatn_refactored.cpp`

#### Build Files Updated:
- `/bindings-cpp/library/CMakeLists.txt` - Updated project name and aliases
- `/bindings-cpp/examples/simple_module/CMakeLists.txt`
- `/bindings-cpp/examples/simple_module/build_simple.sh` - Updated paths from SDK to library

#### Documentation Updated:
- `/bindings-cpp/docs/S2DB_CPP_MODULE_LIBRARY_DOCS.md` - Updated references

### 4. Comment and Documentation Updates
- Updated "SDK" to "Module Library" in comments
- Updated "C++ SDK" to "C++ Module Library" in documentation headers
- Updated error messages referencing "SDK Database" to "Module Library Database"

### 5. CMake Target Updates
- Changed `spacetimedb::sdk::spacetimedb_cpp_library` to `spacetimedb::library::spacetimedb_cpp_library`
- Updated project name from `SpacetimeDBCppSDK` to `SpacetimeDBCppModuleLibrary`

## Files NOT Modified
As per instructions, the following were NOT modified:
- Files in the `modules/` folder
- Files in `bindings-cpp/0026-module-and-sdk-api/`
- References to EMSDK (Emscripten SDK)
- References to client SDKs (JavaScript SDK, Python SDK, etc.)

## Verification
All SDK references have been successfully updated to Module Library, maintaining consistency throughout the codebase while preserving external SDK references where appropriate.