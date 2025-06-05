# C++ Module Macro Test Results

## Summary

Testing the SPACETIMEDB macros defined in `bindings-cpp/library/include/spacetimedb/macros.h`.

## Test Results

### Working:
1. **Basic module compilation with library** - ✅
   - `lib_empty_with_library.cpp` compiles and publishes successfully
   - Modules can use the spacetimedb library without issues

2. **Module compilation without macros** - ✅
   - `test_simple_no_macros.cpp` compiles and publishes successfully
   - Direct library usage works fine

3. **Direct FFI modules** - ✅
   - `lib_standalone_working.cpp` - Works with library for WASI support
   - `lib_truly_standalone.cpp` - Works without any library dependencies
   - Successfully creates tables and reducers

### Issues Found:

1. **SPACETIMEDB_TABLE macro** - ❌ Compiles but fails at publish
   - The macro generates code that compiles successfully
   - However, it creates undefined imports at runtime: `_ZN11SpacetimeDb5bsatn17algebraic_type_ofI6PersonE3getEv`
   - The macro expects bsatn_traits with algebraic_type() method

2. **SPACETIMEDB_REDUCER macro** - ❌ Does not compile
   - The macro calls `RegisterReducer` with wrong signature
   - Expects different function signature than what Module::RegisterReducer provides
   - Mismatch between macro expectations and actual implementation

## Important Discoveries

### AlgebraicType Constants
The correct BSATN type constants for SpacetimeDB are:
```cpp
// From bindings-cpp/library/include/spacetimedb/internal/autogen/AlgebraicType.g.h
enum class Tag {
    Ref = 0,
    Sum = 1,
    Product = 2,
    Array = 3,
    String = 4,
    Bool = 5,
    I8 = 6,
    U8 = 7,
    I16 = 8,
    U16 = 9,
    I32 = 10,
    U32 = 11,
    I64 = 12,
    U64 = 13,
    I128 = 14,
    U128 = 15,
    I256 = 16,
    U256 = 17,
    F32 = 18,
    F64 = 19,
};
```

### WASI Dependencies
- If using C++ standard library (std::string, std::vector), you MUST link with the SpacetimeDB module library
- The library provides WASI shims in `bindings-cpp/library/src/abi/wasi_shims.cpp`
- Without WASI shims, you'll get errors like: `unknown import: wasi_snapshot_preview1::fd_close`

## Working Approaches

### Option 1: Direct FFI with Library (Recommended for std library usage)
```bash
emcmake cmake -B build -DMODULE_SOURCE=src/my_module.cpp
cmake --build build
```

### Option 2: Direct FFI without Library (No std library)
```bash
emcc src/module.cpp -o module.wasm \
  -s STANDALONE_WASM=1 \
  -s EXPORTED_FUNCTIONS=['_malloc','_free'] \
  -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
  -s DISABLE_EXCEPTION_CATCHING=1 \
  -s DISABLE_EXCEPTION_THROWING=1 \
  -s WASM=1 \
  --no-entry \
  -s FILESYSTEM=0 \
  -s INITIAL_MEMORY=16MB \
  -s ALLOW_MEMORY_GROWTH=0 \
  -std=c++20
```

## Next Steps

1. The macros need the underlying infrastructure to be properly implemented:
   - Proper algebraic type generation
   - Correct RegisterReducer implementation matching macro expectations
   - BSATN traits system needs to be fully implemented

2. For now, modules should use direct FFI approach rather than macros