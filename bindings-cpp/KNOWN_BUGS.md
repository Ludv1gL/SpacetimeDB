# Known Bugs in SpacetimeDB C++ Module Library

This document tracks known bugs in the SpacetimeDB C++ module library. Please keep this updated as bugs are discovered and fixed.

## Critical Bugs

### 1. Reducer Parameter Type Registration (CRITICAL)
**Status**: ❌ Not Fixed  
**Severity**: Critical  
**Discovered**: 2024-06-07  
**Files Affected**: 
- `bindings-cpp/library/include/spacetimedb/module.h`
- `bindings-cpp/library/include/spacetimedb/macros.h`
- `bindings-cpp/library/include/spacetimedb/spacetimedb.h`

**Description**: 
The `Module::RegisterReducerDirectImpl` function creates an empty `ProductType` for reducer parameters, causing all parameter types to be misinterpreted:
- `uint16_t` parameters interpreted as `int16_t`
- `uint64_t` parameters interpreted as `int32_t`
- `float` parameters interpreted as `u128`
- `double` parameters interpreted as `i256`

**Impact**: Makes it impossible to use primitive type parameters in reducers correctly.

**Workaround**: 
1. Use direct FFI implementation (see `modules/sdk-test-cpp/src/lib_fixed_direct.cpp`)
2. Wrap primitive parameters in structs

**Root Cause**: No API to specify parameter types when registering reducers. The `register_reducer_impl` function doesn't capture type information properly.

---

### 2. String and Bool Encoding Error
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

### 3. Conflicting Macro Definitions
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

### 4. Missing WASI Support Detection
**Status**: ❌ Not Fixed  
**Severity**: Major  

**Description**: 
No clear error when C++ stdlib features are used without linking WASI shims. Results in cryptic import errors like "unknown import: wasi_snapshot_preview1::fd_close".

**Impact**: Confusing errors for developers using std::string, std::vector without proper linking.

**Workaround**: Always link with SpacetimeDB module library when using C++ stdlib.

---

## Minor Bugs

### 5. LogStopwatch Missing Methods
**Status**: ❌ Not Fixed  
**Severity**: Minor  
**Files Affected**: `bindings-cpp/library/include/spacetimedb/logger.h`

**Description**: 
LogStopwatch class referenced in examples but missing `info()` method implementation.

**Impact**: Cannot use LogStopwatch as documented in examples.

---

### 6. Table Iterator Comparison Issues
**Status**: ❌ Not Fixed  
**Severity**: Minor  

**Description**: 
TableIterator comparison with end iterator is not intuitive and error-prone.

**Impact**: Confusing syntax for iterating over tables.

---

### 7. SPACETIMEDB_TABLE Macro Issues
**Status**: ❌ Not Fixed  
**Severity**: Major  
**Files Affected**: `bindings-cpp/library/include/spacetimedb/macros.h`

**Description**: 
The SPACETIMEDB_TABLE macro fails to properly register tables, causing compilation errors like "use of undeclared identifier 'table_name'".

**Impact**: Cannot use traditional table registration approach in C++ modules.

**Workaround**: Use direct FFI implementation or X-Macro approach.

---

### 8. Missing Type Support
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

### 8. Incorrect BSATN Type Constants
**Status**: ✅ Fixed in commit f6366b64  
**Severity**: Critical  
**Fixed**: 2024-06-07  
**Files**: `bindings-cpp/library/include/spacetimedb/field_registration.h`

**Description**: 
BSATN type constants were incorrectly defined, not matching AlgebraicTypeTag enum values.

**Fix**: Updated all type constants to match the correct enum values.

---

### 9. Incorrect type_id Constants (Partial Fix)
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