# Working C++ Module Pattern

This document provides a working pattern for creating SpacetimeDB C++ modules until the macro system is fully implemented.

## Basic Module Structure

### With C++ Standard Library (requires WASI shims)

```cpp
// lib_standalone_working.cpp
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// Import functions from SpacetimeDB
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, const uint8_t* row, size_t row_len);
}

// BSATN writer helper
class BsatnWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t val) {
        buffer.push_back(val);
    }
    
    void write_u32_le(uint32_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back((val >> 16) & 0xFF);
        buffer.push_back((val >> 24) & 0xFF);
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.size());
        buffer.insert(buffer.end(), str.begin(), str.end());
    }
    
    const std::vector<uint8_t>& get_buffer() const { return buffer; }
};

// Module exports
extern "C" {

__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    BsatnWriter writer;
    
    // Write RawModuleDef::V9 (variant 1)
    writer.write_u8(1);
    
    // Typespace with your types
    writer.write_u32_le(1); // number of types
    
    // Type 0: ProductType for your table (variant 2)
    writer.write_u8(2);
    writer.write_u32_le(3); // number of fields
    
    // Field definitions with correct type constants
    writer.write_u8(0); // Some
    writer.write_string("id");
    writer.write_u8(11); // U32 = 11
    
    writer.write_u8(0); // Some
    writer.write_string("name");
    writer.write_u8(4); // String = 4
    
    writer.write_u8(0); // Some
    writer.write_string("age");
    writer.write_u8(7); // U8 = 7
    
    // Tables, reducers, etc...
    // See lib_standalone_working.cpp for full example
}

__attribute__((export_name("__call_reducer__")))
int32_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    // Handle reducer calls based on id
    if (id == 0) { // First reducer
        // Read arguments, process, insert to tables
        return 0;
    }
    return -999; // Unknown reducer
}

} // extern "C"
```

### Build Command
```bash
# With library (for std::string, std::vector support)
emcmake cmake -B build -DMODULE_SOURCE=src/lib_standalone_working.cpp
cmake --build build
```

### Without C++ Standard Library

For modules that don't need std::string or std::vector, you can avoid WASI dependencies entirely:

```cpp
// See lib_truly_standalone.cpp for a complete example
// Use char arrays instead of std::string
// Use fixed-size buffers instead of std::vector
```

### Build Command
```bash
# Without library (no WASI dependencies)
emcc src/lib_truly_standalone.cpp -o build/module.wasm \
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

## Key Points

1. **AlgebraicType Constants**: Always use the correct type constants from the enum (U32=11, U8=7, String=4, etc.)

2. **WASI Dependencies**: If you use ANY C++ standard library features, you MUST link with the SpacetimeDB module library

3. **Module Exports**: You must define both `__describe_module__` and `__call_reducer__` exports

4. **BSATN Serialization**: All data must be serialized in BSATN format (little-endian integers, length-prefixed strings)

5. **Error Handling**: Return 0 for success, negative values for errors in reducers

## Testing Your Module

```bash
# Start SpacetimeDB
spacetime start

# Publish your module
spacetime publish test-module -b build/your_module.wasm

# Check the schema
spacetime describe test-module --json

# Call a reducer
spacetime call test-module your_reducer arg1 arg2 arg3

# Check logs
spacetime logs test-module -f
```

## Common Issues and Solutions

1. **"unknown import: wasi_snapshot_preview1::fd_close"**
   - Solution: Link with the module library or remove std library usage

2. **Type mismatch in spacetime describe output**
   - Solution: Check your AlgebraicType constants match the enum values

3. **Reducer not found**
   - Solution: Ensure reducer IDs in __call_reducer__ match the order in __describe_module__

4. **Module publishes but reducers fail**
   - Solution: Check logs and ensure BSATN serialization matches expected format