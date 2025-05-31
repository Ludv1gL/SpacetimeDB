#ifndef SPACETIME_SDK_TEST_TYPES_H
#define SPACETIME_SDK_TEST_TYPES_H

#include "spacetimedb/spacetime_macros.h"
#include "spacetimedb/bsatn_reader.h" // Required by generated deserialize
#include "spacetimedb/bsatn_writer.h" // Required by generated serialize
#include "spacetimedb/uint128_placeholder.h" // For SpacetimeDB::Types::uint128_t_placeholder

#include <string>
#include <vector>
#include <optional>
#include <cstdint> // For fixed-width integers

// This file assumes that necessary bsatn::deserialize<PrimitiveType> specializations
// and SpacetimeDB::bsatn::serialize(Writer&, const PrimitiveType&) overloads
// are available through includes from bsatn_reader.h/writer.h or a global bsatn_lib.h.
// For example, from the C++ codegen step, bsatn_lib.h should provide these.
// If not, they might need to be added/stubbed in test_common.h for tests to link.

namespace SpacetimeDB {
namespace Test {

// Basic Enum for testing
enum class BasicEnum : uint8_t {
    ValZero = 0,
    ValOne = 1,
    ValTwo = 2
};
SPACETIMEDB_TYPE_ENUM(BasicEnum, "TestBasicEnum", {
    SPACETIMEDB_ENUM_VARIANT("ValZero"),
    SPACETIMEDB_ENUM_VARIANT("ValOne"),
    SPACETIMEDB_ENUM_VARIANT("ValTwo")
});

// Basic Struct for nesting and vectors
struct NestedData {
    uint32_t item_id;
    std::string item_name;
    std::optional<bool> is_active;

    bool operator==(const NestedData& other) const {
        return item_id == other.item_id &&
               item_name == other.item_name &&
               is_active == other.is_active;
    }
    // For printing in tests if needed
    friend std::ostream& operator<<(std::ostream& os, const NestedData& nd) {
        os << "NestedData{id:" << nd.item_id << ", name:'" << nd.item_name << "', active:"
           << (nd.is_active.has_value() ? (*nd.is_active ? "true" : "false") : "nullopt") << "}";
        return os;
    }
};
#define NESTED_DATA_FIELDS(XX) \
    XX(uint32_t, item_id, false, false) \
    XX(std::string, item_name, false, false) \
    XX(bool, is_active, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(NestedData, "TestNestedData", NESTED_DATA_FIELDS, {
    SPACETIMEDB_FIELD("item_id", SpacetimeDb::CoreType::U32),
    SPACETIMEDB_FIELD("item_name", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD_OPTIONAL("is_active", SpacetimeDb::CoreType::Bool)
});


// Complex Struct with various types
struct ComplexType {
    uint64_t id_field;
    std::string string_field;
    SpacetimeDB::Types::uint128_t_placeholder u128_field;
    BasicEnum enum_field;

    std::optional<int32_t> opt_i32_field;
    std::optional<std::string> opt_string_field;
    std::optional<NestedData> opt_nested_field;
    std::optional<BasicEnum> opt_enum_field;

    std::vector<uint8_t> vec_u8_field; // std::vector<std::byte> often used for raw bytes
    std::vector<std::string> vec_string_field;
    std::vector<NestedData> vec_nested_field;
    std::vector<BasicEnum> vec_enum_field;
    std::vector<std::optional<int32_t>> vec_opt_i32_field; // Vector of optionals

    // For assertions in tests
    bool operator==(const ComplexType& other) const {
        return id_field == other.id_field &&
               string_field == other.string_field &&
               u128_field == other.u128_field &&
               enum_field == other.enum_field &&
               opt_i32_field == other.opt_i32_field &&
               opt_string_field == other.opt_string_field &&
               opt_nested_field == other.opt_nested_field &&
               opt_enum_field == other.opt_enum_field &&
               vec_u8_field == other.vec_u8_field &&
               vec_string_field == other.vec_string_field &&
               vec_nested_field == other.vec_nested_field &&
               vec_enum_field == other.vec_enum_field &&
               vec_opt_i32_field == other.vec_opt_i32_field;
    }
};

// X-Macro for ComplexType fields
// For optionals, CPP_TYPE is the underlying type (e.g., int32_t for std::optional<int32_t>)
// For vectors, CPP_TYPE is the element type (e.g., uint8_t for std::vector<uint8_t>)
#define COMPLEX_TYPE_FIELDS(XX) \
    XX(uint64_t, id_field, false, false) \
    XX(std::string, string_field, false, false) \
    XX(SpacetimeDB::Types::uint128_t_placeholder, u128_field, false, false) \
    XX(BasicEnum, enum_field, false, false) \
    XX(int32_t, opt_i32_field, true, false) \
    XX(std::string, opt_string_field, true, false) \
    XX(NestedData, opt_nested_field, true, false) \
    XX(BasicEnum, opt_enum_field, true, false) \
    XX(uint8_t, vec_u8_field, false, true) \
    XX(std::string, vec_string_field, false, true) \
    XX(NestedData, vec_nested_field, false, true) \
    XX(BasicEnum, vec_enum_field, false, true) \
    XX(std::optional<int32_t>, vec_opt_i32_field, false, true) /* Element type is std::optional<int32_t> */

// Register ComplexType with SpacetimeDB
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(ComplexType, "TestComplexType", COMPLEX_TYPE_FIELDS, {
    SPACETIMEDB_FIELD("id_field", SpacetimeDb::CoreType::U64),
    SPACETIMEDB_FIELD("string_field", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD("u128_field", SpacetimeDb::CoreType::U128),
    SPACETIMEDB_FIELD_CUSTOM("enum_field", "TestBasicEnum"),
    SPACETIMEDB_FIELD_OPTIONAL("opt_i32_field", SpacetimeDb::CoreType::I32),
    SPACETIMEDB_FIELD_OPTIONAL("opt_string_field", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("opt_nested_field", "TestNestedData"),
    SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("opt_enum_field", "TestBasicEnum"),
    SPACETIMEDB_FIELD("vec_u8_field", SpacetimeDb::CoreType::Bytes), // Typically Bytes for vector<u8> or vector<byte>
    // Schema registration for vector<string>, vector<custom>, vector<optional<primitive>>
    // currently doesn't have specific CoreType variants. These are illustrative for (de)serialization.
    // The (de)serialization macros should handle them based on C++ type, but schema def might be simplified.
    SPACETIMEDB_FIELD("vec_string_field", SpacetimeDb::CoreType::String), // Placeholder, needs vector type in schema
    SPACETIMEDB_FIELD_CUSTOM("vec_nested_field", "TestNestedData"),   // Placeholder
    SPACETIMEDB_FIELD_CUSTOM("vec_enum_field", "TestBasicEnum"),       // Placeholder
    SPACETIMEDB_FIELD_OPTIONAL("vec_opt_i32_field", SpacetimeDb::CoreType::I32) // Placeholder
});


// Struct specifically for testing reducer arguments
struct ReducerArgsTestStruct {
    std::string s_arg;
    uint64_t u_arg;
    std::optional<NestedData> opt_nested_arg;
    std::vector<BasicEnum> vec_enum_arg;

     bool operator==(const ReducerArgsTestStruct& other) const {
        return s_arg == other.s_arg &&
               u_arg == other.u_arg &&
               opt_nested_arg == other.opt_nested_arg &&
               vec_enum_arg == other.vec_enum_arg;
    }
};
#define REDUCER_ARGS_TEST_STRUCT_FIELDS(XX) \
    XX(std::string, s_arg, false, false) \
    XX(uint64_t, u_arg, false, false) \
    XX(NestedData, opt_nested_arg, true, false) \
    XX(BasicEnum, vec_enum_arg, false, true)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(ReducerArgsTestStruct, "TestReducerArgsStruct", REDUCER_ARGS_TEST_STRUCT_FIELDS, {
    SPACETIMEDB_FIELD("s_arg", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD("u_arg", SpacetimeDb::CoreType::U64),
    SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("opt_nested_arg", "TestNestedData"),
    SPACETIMEDB_FIELD_CUSTOM("vec_enum_arg", "TestBasicEnum") // Placeholder for vector in schema
});


} // namespace Test
} // namespace SpacetimeDB

#endif // SPACETIME_SDK_TEST_TYPES_H
