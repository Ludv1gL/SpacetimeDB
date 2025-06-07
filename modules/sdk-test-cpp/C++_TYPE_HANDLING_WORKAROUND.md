# C++ Module Type Handling Workaround

## Issue
The SpacetimeDB C++ module library has a critical bug where reducer parameter types are not properly registered in the module definition. The `Module::RegisterReducerDirectImpl` function creates an empty `ProductType` for parameters, which causes type misinterpretation:

- u16 parameters are interpreted as i16
- u64 parameters are interpreted as i32  
- float parameters are interpreted as u128
- double parameters are interpreted as i256
- etc.

## Root Cause
In the C++ module library, there's no API to specify parameter types when registering reducers. The library creates an empty ProductType regardless of actual parameter types.

## Workaround Solutions

### Option 1: Direct FFI Implementation (Recommended)
Use direct FFI calls and manually construct the module definition with correct type information. See `lib_fixed_direct.cpp` for a complete example.

**Pros:**
- Full control over type registration
- All primitive types work correctly
- Can use primitive parameters in reducers

**Cons:**
- More verbose code
- Need to manually handle BSATN serialization
- No C++ stdlib support without WASI shims

### Option 2: Struct Parameters
Instead of primitive parameters, wrap all reducer parameters in structs:

```cpp
// Instead of:
SPACETIMEDB_REDUCER(insert_u8, SpacetimeDb::ReducerContext ctx, uint8_t n) {
    // This will fail - n will be misinterpreted
}

// Use:
struct U8Param {
    uint8_t n;
    
    void bsatn_serialize(BsatnWriter& writer) const {
        writer.write_u8(n);
    }
    
    void bsatn_deserialize(BsatnReader& reader) {
        n = reader.read_u8();
    }
};

SPACETIMEDB_REDUCER(insert_u8, SpacetimeDb::ReducerContext ctx, U8Param param) {
    // This works - struct types are properly registered
    uint8_t value = param.n;
}
```

**Pros:**
- Works with the existing C++ module library
- Can use C++ stdlib features

**Cons:**
- Extra boilerplate for parameter structs
- Less intuitive API

## Verification
The fix has been verified to work correctly for all primitive types:
- Unsigned integers: u8, u16, u32, u64
- Signed integers: i8, i16, i32, i64
- Floating point: f32, f64

All types now correctly handle their full value ranges without truncation or misinterpretation.

## Long-term Solution
The C++ module library needs to be updated to properly register reducer parameter types. This requires changes to:
1. The `SPACETIMEDB_REDUCER` macro to capture parameter type information
2. The `Module::RegisterReducerDirectImpl` function to use actual parameter types
3. Type mapping functions to handle all primitive types correctly