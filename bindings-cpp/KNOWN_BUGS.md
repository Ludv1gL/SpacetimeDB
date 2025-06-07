# Known Bugs in SpacetimeDB C++ Module Library

This document tracks known bugs in the SpacetimeDB C++ module library. Please keep this updated as bugs are discovered and fixed.

## Critical Bugs

### 1. Reducer Parameter Type Registration (CRITICAL)
**Status**: ✅ FIXED - C++ Module Library registration now works correctly
**Severity**: Critical  
**Discovered**: 2024-06-07  
**Tested**: 2024-06-07  
**Fixed**: 2024-06-07  
**Files Fixed**: 
- `bindings-cpp/library/include/spacetimedb/macros.h`

**Description**: 
The `SPACETIMEDB_REDUCER_EX` macro in `macros.h` was calling the broken `Module::RegisterReducer` function instead of the working `register_reducer_impl` function. This caused empty parameter schemas to be generated.

**Original Problem**: ALL parameter types were interpreted as `uint8_t` regardless of declared type.

**Test Results After Fix**:
- ✅ C++ module schema generation now correct (verified with `spacetime describe`)
- ✅ `insert_one_u8` correctly shows `"algebraic_type": { "U8": [] }`
- ✅ `insert_u16` correctly shows `"algebraic_type": { "U16": [] }`
- ✅ `compute_i16_sum` correctly shows `"algebraic_type": { "I16": [] }`
- ✅ `bitwise_u32` correctly shows `"algebraic_type": { "U32": [] }`

**Fix Applied**: Changed `SPACETIMEDB_REDUCER_EX` macro to call `SpacetimeDb::register_reducer_impl` instead of `SpacetimeDb::Internal::Module::RegisterReducer`

**NEW ISSUE DISCOVERED**: Despite correct schema, runtime parameter values are still truncated to u8. This indicates a **SpacetimeDB host/CLI bug** where arguments are being serialized/passed incorrectly regardless of schema.

---

### 2. SpacetimeDB Host/CLI Parameter Serialization Bug (CRITICAL)
**Status**: ❌ Not Fixed - SpacetimeDB core issue
**Severity**: Critical  
**Discovered**: 2024-06-07  
**Affected**: SpacetimeDB host system and/or CLI

**Description**: 
Despite C++ modules now correctly registering parameter types in the schema, the SpacetimeDB host/CLI system still truncates all arguments to u8 values during serialization/deserialization.

**Test Evidence**:
- Schema correctly shows `insert_u16` with `"algebraic_type": { "U16": [] }`
- CLI call `spacetime call test insert_u16 65535` stores value as 255 (truncated)
- CLI call `spacetime call test insert_u16 200` stores value as 200 (fits in u8)
- Same pattern for i16, u32, i32, etc.

**Impact**: C++ modules cannot receive correct parameter values for any type beyond u8, despite having correct schemas.

**Root Cause**: Unknown - likely in SpacetimeDB argument serialization/deserialization pipeline between CLI and module execution.

**Workaround**: None known. Only u8 parameters work correctly.

---

### 3. String and Bool Encoding Error
**Status**: ❌ Not Fixed  
**Severity**: High  
**Discovered**: 2024-06-07  
**Error**: "unknown tag 0x62 for sum type option"

**Description**: 
When publishing modules with string or bool types in tables, SpacetimeDB fails with encoding errors.

**Impact**: Cannot use string or bool types in C++ modules.

**Workaround**: Avoid string and bool types until fixed.

---

## Major Bugs

### 4. Conflicting Macro Definitions
**Status**: ❌ Not Fixed  
**Severity**: Major  
**Files Affected**:
- `bindings-cpp/library/include/spacetimedb/macros.h`
- `bindings-cpp/library/include/spacetimedb/builtin_reducers.h`
- `bindings-cpp/library/include/spacetimedb/reducer.h`

**Description**: 
Multiple conflicting definitions of `SPACETIMEDB_REDUCER`, `SPACETIMEDB_INIT`, and other macros across different headers.

**Impact**: Compilation warnings and potential undefined behavior.

---

### 5. Missing WASI Support Detection
**Status**: ❌ Not Fixed  
**Severity**: Major  

**Description**: 
No clear error when C++ stdlib features are used without linking WASI shims. Results in cryptic import errors like "unknown import: wasi_snapshot_preview1::fd_close".

**Impact**: Confusing errors for developers using std::string, std::vector without proper linking.

**Workaround**: Always link with SpacetimeDB module library when using C++ stdlib.

---

## Minor Bugs

### 6. LogStopwatch Missing Methods
**Status**: ❌ Not Fixed  
**Severity**: Minor  
**Files Affected**: `bindings-cpp/library/include/spacetimedb/logger.h`

**Description**: 
LogStopwatch class referenced in examples but missing `info()` method implementation.

**Impact**: Cannot use LogStopwatch as documented in examples.

---

### 7. Table Iterator Comparison Issues
**Status**: ❌ Not Fixed  
**Severity**: Minor  

**Description**: 
TableIterator comparison with end iterator is not intuitive and error-prone.

**Impact**: Confusing syntax for iterating over tables.

---

### 8. SPACETIMEDB_TABLE Macro Issues
**Status**: ❌ Not Fixed  
**Severity**: Major  
**Files Affected**: `bindings-cpp/library/include/spacetimedb/macros.h`

**Description**: 
The SPACETIMEDB_TABLE macro fails to properly register tables, causing compilation errors like "use of undeclared identifier 'table_name'".

**Impact**: Cannot use traditional table registration approach in C++ modules.

**Workaround**: Use direct FFI implementation or X-Macro approach.

---

### 9. Missing Type Support
**Status**: ❌ Not Fixed  
**Severity**: Minor  

**Description**: 
No built-in support for common types like:
- `std::optional`
- `std::variant`
- `std::chrono` types
- Custom enum serialization

**Impact**: Limited type expressiveness compared to Rust modules.

---

## Fixed Bugs

### 10. Incorrect BSATN Type Constants
**Status**: ✅ Fixed in commit f6366b64  
**Severity**: Critical  
**Fixed**: 2024-06-07  
**Files**: `bindings-cpp/library/include/spacetimedb/field_registration.h`

**Description**: 
BSATN type constants were incorrectly defined, not matching AlgebraicTypeTag enum values.

**Fix**: Updated all type constants to match the correct enum values.

---

### 11. Incorrect type_id Constants (Partial Fix)
**Status**: ✅ Partially Fixed - type constants updated  
**Severity**: High  
**Fixed**: 2024-06-07  
**Files**: `bindings-cpp/library/include/spacetimedb/spacetimedb.h`

**Description**: 
The `type_id` template specializations used incorrect values, causing parameter type misregistration.

**Fix**: Updated type_id constants to match AlgebraicTypeTag enum. However, SPACETIMEDB_TABLE macro still has issues preventing full testing.

---

## How to Update This Document

When discovering a new bug:
1. Add it to the appropriate severity section
2. Include: Status, Severity, Date discovered, Files affected, Description, Impact, and Workaround (if any)
3. Update status when fixed, including commit hash

When fixing a bug:
1. Move it to "Fixed Bugs" section
2. Update status to ✅ Fixed
3. Add fix date and commit hash
4. Describe the fix briefly