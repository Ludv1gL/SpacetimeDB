# Module Description Format Issue Analysis

## Problem Summary

The C++ SDK's `__describe_module__` function is generating incorrect BSATN data, preventing modules with complex types from being published to SpacetimeDB.

## Root Causes Identified

1. **Hardcoded Field Structure**: The `add_fields_for_type()` function assumes all structs have a single `uint8_t` field named 'n', regardless of actual struct layout.

2. **Incorrect BSATN Format**: The manual byte-by-byte construction of the module description doesn't match the expected BSATN format for `RawModuleDefV9`.

3. **Missing Type System**: The AlgebraicType encoding is incomplete - we're writing simple tags instead of full type descriptions.

## Current Limitations

- Only structs with a single `uint8_t n` field work correctly
- No support for strings, vectors, or other complex types
- No support for optional fields
- No reflection mechanism to discover struct fields

## Errors Encountered

1. **"unknown tag 0x69 for sum type option"**: Occurs when string data is read as Option tags
2. **"data too short for [u8]: Expected 1953719668"**: String length being misinterpreted
3. These errors indicate the deserializer is reading data at wrong offsets

## Working Examples

- `test_minimal.cpp`: Single table with uint8_t field ✓
- `test_single_field.cpp`: Multiple tables with uint8_t fields ✓
- `reducer_clean_ctx.cpp`: Uses same pattern ✓

## Failed Examples

- `module_test.cpp`: Complex types (strings, vectors, custom structs) ✗
- `test_gradual.cpp`: String fields cause errors ✗

## Solutions

### Short-term (Current SDK)
1. Document that only single uint8_t fields are supported
2. Create examples that work within these constraints
3. Use simplified module_test_simple.cpp for testing

### Medium-term
1. Implement proper field registration using macros:
   ```cpp
   SPACETIMEDB_STRUCT(Person,
       FIELD(id, uint32_t),
       FIELD(name, std::string),
       FIELD(age, uint8_t)
   )
   ```

2. Use existing SPACETIMEDB_BSATN_STRUCT macro infrastructure

### Long-term
1. Implement full BSATN serialization in C++
2. Support for all SpacetimeDB types
3. Proper AlgebraicType and Typespace generation
4. Match Rust SDK functionality

## Next Steps

1. Fix the immediate issue by implementing proper field registration
2. Study the Rust bindings to understand correct BSATN format
3. Implement a minimal BSATN writer for C++
4. Test with progressively complex types

## Technical Details

The correct module description format (RawModuleDefV9) requires:
- Typespace with AlgebraicType definitions
- Table definitions with proper column types
- Reducer definitions with parameter types
- All serialized using BSATN format

Current implementation writes raw bytes without proper BSATN structure, causing deserialization failures.