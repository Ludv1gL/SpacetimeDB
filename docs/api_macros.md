# C++ SDK API: Schema Definition Macros

The SpacetimeDB C++ Module SDK uses a set of macros to define your module's schema (types, tables, reducers) and to generate necessary helper code like BSATN (de)serialization functions.

## Core Concepts

*   **Schema Registration**: Macros register your C++ types, tables, and reducers with an internal `ModuleSchema` instance. This schema is then serialized and provided to the SpacetimeDB runtime.
*   **BSATN Generation**: For types defined with `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS` and `SPACETIMEDB_TYPE_ENUM`, the SDK automatically generates inline functions for serialization to and deserialization from the BSATN format.
*   **Manual C++ Definitions**: You must define your C++ `struct`s and `enum class`es manually. The macros augment these definitions with SpacetimeDB capabilities.

## Common Helper Macros for Schema Registration

These helpers are used within the `RegFieldsInitializerList` or `RegParamsInitializerList` arguments of the main schema macros.

*   `SPACETIMEDB_FIELD(FieldNameStr, FieldCoreType)`
    *   Defines a basic field.
    *   `FieldNameStr`: String name of the field (e.g., `"player_id"`).
    *   `FieldCoreType`: The `SpacetimeDb::CoreType` of the field (e.g., `SpacetimeDb::CoreType::U64`).
*   `SPACETIMEDB_FIELD_OPTIONAL(FieldNameStr, FieldCoreType)`
    *   Defines an optional basic field. The C++ type should be `std::optional<ActualType>`.
    *   `FieldCoreType`: The `SpacetimeDb::CoreType` of the *value* within the optional.
*   `SPACETIMEDB_FIELD_CUSTOM(FieldNameStr, UserDefinedTypeNameStr)`
    *   Defines a field whose type is another user-defined struct or enum.
    *   `UserDefinedTypeNameStr`: The string name of the user-defined C++ type (this name should match the `SpacetimeDbNameStr` used when registering that custom type).
*   `SPACETIMEDB_FIELD_CUSTOM_OPTIONAL(FieldNameStr, UserDefinedTypeNameStr)`
    *   Defines an optional field of a user-defined type. C++ type should be `std::optional<CustomType>`.

## Type Definition Macros

### `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS`

Registers a C++ `struct` and generates its BSATN (de)serialization functions using an X-Macro pattern for field iteration.

**Syntax:**
```cpp
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    CppTypeName,               // e.g., Player
    SpacetimeDbNameStr,        // e.g., "PlayerDB" (schema name)
    FIELDS_MACRO,              // Your X-Macro for fields
    RegFieldsInitializerList   // For schema registration, e.g., { SPACETIMEDB_FIELD(...) }
);
```

**Parameters:**
*   `CppTypeName`: The C++ name of your manually defined struct.
*   `SpacetimeDbNameStr`: The name of this type as it will appear in the SpacetimeDB schema.
*   `FIELDS_MACRO`: An X-Macro you define. It takes one argument (conventionally `XX`). For each field in your struct, your `FIELDS_MACRO` must call `XX` with the following arguments:
    1.  `CPP_TYPE_OR_ELEMENT_TYPE`:
        *   For regular fields: The C++ type of the field (e.g., `uint64_t`, `std::string`, `MyNestedStruct`).
        *   For `std::optional<T>` fields: The type `T` (e.g., for `std::optional<int32_t>`, pass `int32_t`).
        *   For `std::vector<T>` fields: The element type `T` (e.g., for `std::vector<MyItem>`, pass `MyItem`).
    2.  `FIELD_NAME`: The C++ name of the field in your struct (e.g., `player_id`).
    3.  `IS_OPTIONAL`: A boolean (`true` or `false`) indicating if the C++ field type is `std::optional`.
    4.  `IS_VECTOR`: A boolean (`true` or `false`) indicating if the C++ field type is `std::vector`.
*   `RegFieldsInitializerList`: An initializer list for schema registration using `SPACETIMEDB_FIELD` helpers. This describes the schema structure to SpacetimeDB.

**Example:**
```cpp
// Define your C++ struct
struct ComplexData {
    uint32_t item_id;
    std::string description;
    std::optional<int64_t> bonus_value;
    std::vector<std::string> tags;
};

// Define the X-Macro for its fields
#define COMPLEX_DATA_FIELDS(XX) \
    XX(uint32_t, item_id, false, false) \
    XX(std::string, description, false, false) \
    XX(int64_t, bonus_value, true, false)    /* For std::optional<int64_t> */ \
    XX(std::string, tags, false, true)       /* For std::vector<std::string> */

// Register and generate (de)serialization
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(ComplexData, "ComplexDataSchemaName", COMPLEX_DATA_FIELDS, {
    SPACETIMEDB_FIELD("item_id", SpacetimeDb::CoreType::U32),
    SPACETIMEDB_FIELD("description", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD_OPTIONAL("bonus_value", SpacetimeDb::CoreType::I64),
    SPACETIMEDB_FIELD("tags", SpacetimeDb::CoreType::String) // Note: Vector registration needs improvement in schema
});
```
*Generated Functions:*
*   `void SpacetimeDB::bsatn::serialize(bsatn::Writer& writer, const CppTypeName& value);`
*   `CppTypeName SpacetimeDB::bsatn::deserialize_CppTypeName(bsatn::Reader& reader);`
*   `template<> inline CppTypeName bsatn::deserialize<CppTypeName>(bsatn::Reader& reader);`

### `SPACETIMEDB_TYPE_STRUCT` (Registration Only)

Registers a C++ `struct` for schema definition but does **not** generate BSATN (de)serialization functions. Use this if you plan to provide manual BSATN functions or if the type is not directly sent over the network.

**Syntax:**
```cpp
SPACETIMEDB_TYPE_STRUCT(
    CppTypeName,
    SpacetimeDbNameStr,
    RegFieldsInitializerList
);
```
(Parameters are the same as `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS` except for the missing `FIELDS_MACRO`).

### `SPACETIMEDB_TYPE_ENUM`

Registers a C++ `enum class` and generates its BSATN (de)serialization functions.

**Requirements:**
*   The enum class **must** be defined with an underlying type of `uint8_t`.

**Syntax:**
```cpp
SPACETIMEDB_TYPE_ENUM(
    CppEnumName,             // e.g., PlayerStatus
    SpacetimeDbNameStr,      // e.g., "PlayerStatusDb"
    RegVariantsInitializerList // e.g., { SPACETIMEDB_ENUM_VARIANT("Online"), ... }
);
```
*   `RegVariantsInitializerList`: Uses `SPACETIMEDB_ENUM_VARIANT(VariantNameStr)` to list enum members.

**Example:**
```cpp
enum class MyEnum : uint8_t { VariantA = 0, VariantB = 1 };
SPACETIMEDB_TYPE_ENUM(MyEnum, "MyEnumSchemaName", {
    SPACETIMEDB_ENUM_VARIANT("VariantA"),
    SPACETIMEDB_ENUM_VARIANT("VariantB")
});
```
*Generated Functions:* (Similar to `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS`)
*   `void SpacetimeDB::bsatn::serialize(bsatn::Writer& writer, const CppEnumName& value);`
*   `CppEnumName SpacetimeDB::bsatn::deserialize_CppEnumName(bsatn::Reader& reader);`
*   `template<> inline CppEnumName bsatn::deserialize<CppEnumName>(bsatn::Reader& reader);`

## Table Definition Macros

### `SPACETIMEDB_TABLE`

Declares a SpacetimeDB table, associating a registered C++ row type with a table name in the schema.

**Syntax:**
```cpp
SPACETIMEDB_TABLE(CppRowTypeName, SpacetimeDbTableNameStr);
```
*   `CppRowTypeName`: The C++ name of the struct used for rows (e.g., `Player`). This type must have been previously registered.
*   `SpacetimeDbTableNameStr`: The schema name for this table (e.g., `"Players"`).

### `SPACETIMEDB_PRIMARY_KEY`

Specifies the primary key for a table.

**Syntax:**
```cpp
SPACETIMEDB_PRIMARY_KEY(SpacetimeDbTableNameStr, FieldNameStr);
```
*   `SpacetimeDbTableNameStr`: The schema name of the table (must match `SPACETIMEDB_TABLE`).
*   `FieldNameStr`: The name of the field within the row type that is the primary key (e.g., `"player_id"`).

## Reducer Definition Macros

### `SPACETIMEDB_REDUCER_PARAM` & `SPACETIMEDB_REDUCER_PARAM_CUSTOM`

Helpers for defining reducer parameters for schema registration. Used within `SPACETIMEDB_REDUCER`.

*   `SPACETIMEDB_REDUCER_PARAM(ParamNameStr, ParamCoreType)`
*   `SPACETIMEDB_REDUCER_PARAM_CUSTOM(ParamNameStr, UserDefinedTypeNameStr)`

### `SPACETIMEDB_REDUCER`

Registers a C++ function as a SpacetimeDB reducer and generates its type-erased invoker for the dispatch system.

**Syntax:**
```cpp
SPACETIMEDB_REDUCER(
    SpacetimeDbReducerNameStr, // e.g., "CreatePlayer"
    CppFunctionName,           // e.g., create_player_impl
    RegParamsInitializerList,  // For schema: { SPACETIMEDB_REDUCER_PARAM(...), ... }
    ...                        // Variadic C++ types of function parameters, e.g., uint64_t, std::string
);
```
**Parameters:**
*   `SpacetimeDbReducerNameStr`: The name of the reducer known to SpacetimeDB.
*   `CppFunctionName`: The name of your C++ function that implements the reducer logic.
*   `RegParamsInitializerList`: Schema definition for parameters using `SPACETIMEDB_REDUCER_PARAM` helpers.
*   `...` (Variadic arguments): A comma-separated list of the C++ types of the `CppFunctionName`'s parameters, in the exact order they appear in the function signature. This is used to generate the BSATN deserialization calls in the invoker.

**Example:**
```cpp
void my_reducer_impl(uint64_t entity_id, std::string action_name) {
    // ... reducer logic ...
}

SPACETIMEDB_REDUCER("DoAction", my_reducer_impl, {
    SPACETIMEDB_REDUCER_PARAM("entity_id", SpacetimeDb::CoreType::U64),
    SPACETIMEDB_REDUCER_PARAM("action_name", SpacetimeDb::CoreType::String)
}, uint64_t, std::string);
```
This macro sets up the reducer for schema export and ensures it can be called by the SpacetimeDB runtime via the `_spacetimedb_dispatch_reducer` bridge function, with arguments automatically deserialized.
```
