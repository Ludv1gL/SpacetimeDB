# SpacetimeDB C++ SDK Test Implementation

This directory contains the comprehensive test implementation for the SpacetimeDB C++ SDK.

## Directory Structure

```
sdk_test_cpp/
├── src/
│   ├── sdk_test.h      # Complete type definitions (52 table types, enums, structs)
│   ├── sdk_test.cpp    # Main implementation file (currently ~25% complete)
│   └── archive/        # Historical iteration files for reference
├── CMakeLists.txt      # Build configuration
└── README.md           # This file
```

## Current Implementation Status

### Completed (in sdk_test.cpp)
- ✅ EnumWithPayload serialization/deserialization
- ✅ Basic primitive type tables (u8, u16, u32, u64, u128, u256)
- ✅ Signed integer tables (i8, i16, i32, i64, i128, i256)
- ✅ Boolean and floating point tables (bool, f32, f64)
- ✅ String and special type tables (string, identity, connection_id, timestamp)
- ✅ Some complex type tables (byte_struct, enum_with_payload, every_primitive_struct, every_vec_struct)

### TODO - Remaining Tables
- [ ] Vector tables (vec_u8, vec_u16, vec_u32, etc.)
- [ ] Option tables (option_i32, option_string, etc.)
- [ ] Unique constraint tables (unique_u8, unique_u16, etc.)
- [ ] Primary key tables (pk_u8, pk_u16, etc.)
- [ ] Indexed tables (indexed_table, indexed_table_2, btree_u32)
- [ ] Large table and table_holds_table
- [ ] Simple enum table

### TODO - Reducers
Need to implement reducers for all table types:
- [ ] insert_<type> reducers for basic tables
- [ ] update_<type> reducers for updatable tables
- [ ] delete_<type> reducers for deletable tables
- [ ] Special reducers for testing constraints and indexes

## Building and Testing

### Compile to WASM
```bash
# From this directory
emcc -std=c++20 -s STANDALONE_WASM=1 -s FILESYSTEM=0 -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry -o sdk_test.wasm src/sdk_test.cpp
```

### Publish to SpacetimeDB
```bash
spacetime publish --bin-path sdk_test.wasm sdk-test
```

### Verify Schema
```bash
# Get actual schema
spacetime describe sdk-test --json > actual-schema.json

# Compare with expected schema
diff actual-schema.json ../../../tests/sdk-test-desc.json
```

## Development Guidelines

1. **Add tables in logical groups** - Keep related tables together (e.g., all vector tables, all option tables)
2. **Use consistent naming** - Table names should match the pattern from sdk-test-desc.json
3. **Test incrementally** - Compile and test after adding each group of tables
4. **Follow the reference** - The expected schema in tests/sdk-test-desc.json is the authoritative reference

## Next Steps

1. Continue adding SPACETIMEDB_TABLE registrations for remaining tables
2. Implement reducer functions for each table type
3. Add proper index definitions where needed
4. Test that the generated schema matches the reference exactly