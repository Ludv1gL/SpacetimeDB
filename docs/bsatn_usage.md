# C++ SDK API: BSATN Usage

BSATN (Binary SpacetimeDB Abstract Type Notation) is the serialization format used by SpacetimeDB to encode data, such as reducer arguments and table row data, when communicating between the host runtime and C++ Wasm modules.

The SpacetimeDB C++ Module SDK provides two core classes for handling BSATN:
*   `bsatn::Writer` (in `bsatn_writer.h`): Used to serialize C++ objects into a BSATN byte buffer.
*   `bsatn::Reader` (in `bsatn_reader.h`): Used to deserialize data from a BSATN byte buffer into C++ objects.

## Automatic (De)serialization

For most common use cases, you **do not need to interact directly** with `bsatn::Writer` or `bsatn::Reader`. The SDK is designed to automate BSATN handling:

1.  **User-Defined Types**:
    *   When you define your structs and enums using `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS` and `SPACETIMEDB_TYPE_ENUM` respectively, the SDK automatically generates the necessary `SpacetimeDB::bsatn::serialize()` and `SpacetimeDB::bsatn::deserialize_YourTypeName()` functions for them.
    *   These functions internally use `bsatn::Writer` and `bsatn::Reader` to correctly process each field according to BSATN rules (e.g., endianness, string encoding, optional/vector representation).

2.  **Reducer Arguments**:
    *   The `SPACETIMEDB_REDUCER` macro generates an invoker for your C++ reducer function. This invoker automatically uses `bsatn::Reader` and the generated `bsatn::deserialize<ArgumentType>()` specializations to decode incoming arguments before calling your C++ function.

3.  **SDK Runtime Functions**:
    *   Functions like `SpacetimeDB::table_insert(tableName, rowObject)` and `SpacetimeDB::table_delete_by_pk(tableName, pkValue)` internally use `bsatn::Writer` and the generated `SpacetimeDB::bsatn::serialize()` functions to convert your C++ objects into BSATN byte arrays before passing them to the host environment via the ABI.
    *   Future functions that retrieve data from the host (e.g., `SpacetimeDB::table_get_by_pk()`) would internally use `bsatn::Reader` to deserialize the host's response.

## Advanced/Manual Usage

While direct use is uncommon, `bsatn::Writer` and `bsatn::Reader` are available if you have highly specialized needs:

*   **Custom Serialization for External Types**: If you are working with C++ types from external libraries that cannot be directly annotated with SDK macros, you might provide your own `SpacetimeDB::bsatn::serialize()` and `SpacetimeDB::bsatn::deserialize_YourTypeName()` implementations for them. These implementations would then use `bsatn::Writer` and `bsatn::Reader` methods.
*   **Interacting with Custom Host Functions**: If your module defines additional `extern "C"` host functions that transmit raw BSATN byte arrays, you might use the reader/writer to interpret or construct these byte arrays.
*   **Debugging or Unit Testing**: You can use `bsatn::Writer` to serialize test data and `bsatn::Reader` to inspect the byte output or to feed test byte streams into your deserialization logic. The `spacetime_test_types.h` example uses this approach for its unit tests.

**Example (Conceptual - Manually Serializing Arguments for a Test):**
```cpp
#include "bsatn_writer.h"
#include "your_custom_type.h" // Assume this has generated SpacetimeDB::bsatn::serialize

bsatn::Writer writer;
int32_t arg1 = 123;
std::string arg2 = "hello";
MyCustomType arg3 = { /* ... */ };

// Manually serialize each argument
// For primitives, global SpacetimeDB::bsatn::serialize or direct writer methods can be used.
SpacetimeDB::bsatn::serialize(writer, arg1); // Uses SDK-provided or generated serialize
SpacetimeDB::bsatn::serialize(writer, arg2);
SpacetimeDB::bsatn::serialize(writer, arg3); // Uses generated serialize for MyCustomType

std::vector<std::byte> serialized_args = writer.get_buffer();
// 'serialized_args' could then be used, e.g., in a test call to _spacetimedb_dispatch_reducer
```

In summary, while the BSATN layer is fundamental, the C++ SDK aims to abstract its direct usage away for most development tasks, promoting ease of use through macro-based code generation.
```
