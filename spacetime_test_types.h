#ifndef SPACETIME_TEST_TYPES_H
#define SPACETIME_TEST_TYPES_H

#include "spacetime_macros.h"
#include "bsatn_writer.h"
#include "bsatn_reader.h"
#include "uint128_placeholder.h" // For SpacetimeDB::Types::uint128_t_placeholder

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <cassert>
#include <iomanip> // For std::hex

// Ensure bsatn::serialize and bsatn::deserialize specializations for primitives are available.
// These would typically be in bsatn_lib.h or similar. For this test, ensure they are declared/defined.
// Assuming bsatn_writer.h/cpp and bsatn_reader.h/cpp provide necessary global serialize/deserialize for primitives
// or the deserialize<T> specializations. Let's add a few common ones here if not fully covered by bsatn_lib.h from codegen.

namespace SpacetimeDB { namespace bsatn {
    // These are expected to be provided by bsatn_lib.h or similar central BSATN utilities
    // For example, for uint64_t:
    inline void serialize(Writer& w, const uint64_t& val) { w.write_u64_le(val); }
    // inline uint64_t deserialize_uint64_t(Reader& r) { return r.read_u64_le(); } // Old style
    // template<> inline uint64_t deserialize<uint64_t>(Reader& r) { return r.read_u64_le(); } // New style

    // For int32_t:
    inline void serialize(Writer& w, const int32_t& val) { w.write_i32_le(val); }
    // inline int32_t deserialize_int32_t(Reader& r) { return r.read_i32_le(); }
    // template<> inline int32_t deserialize<int32_t>(Reader& r) { return r.read_i32_le(); }

    // For std::string:
    inline void serialize(Writer& w, const std::string& val) { w.write_string(val); }
    // inline std::string deserialize_std_string(Reader& r) { return r.read_string(); }
    // template<> inline std::string deserialize<std::string>(Reader& r) { return r.read_string(); }

    // For bool
    inline void serialize(Writer& w, const bool& val) { w.write_bool(val); }
    // template<> inline bool deserialize<bool>(Reader& r) { return r.read_bool(); }

    // For uint8_t
    inline void serialize(Writer& w, const uint8_t& val) { w.write_u8(val); }
    // template<> inline uint8_t deserialize<uint8_t>(Reader& r) { return r.read_u8(); }

    // For SpacetimeDB::Types::uint128_t_placeholder
    inline void serialize(Writer& w, const SpacetimeDB::Types::uint128_t_placeholder& val) { w.write_u128_le(val); }
    // template<> inline SpacetimeDB::Types::uint128_t_placeholder deserialize<SpacetimeDB::Types::uint128_t_placeholder>(Reader& r) { return r.read_u128_le(); }

}} // SpacetimeDB::bsatn

// Test Enum
enum class TestStatus : uint8_t {
    Unknown = 0,
    Active = 1,
    Inactive = 2
};
SPACETIMEDB_TYPE_ENUM(TestStatus, "TestStatusDB", {
    SPACETIMEDB_ENUM_VARIANT("Unknown"),
    SPACETIMEDB_ENUM_VARIANT("Active"),
    SPACETIMEDB_ENUM_VARIANT("Inactive")
});

// Test Nested Struct
struct TestNestedStruct {
    uint32_t nested_id;
    std::string description;

    bool operator==(const TestNestedStruct& other) const {
        return nested_id == other.nested_id && description == other.description;
    }
};
// Fields macro for TestNestedStruct
#define TEST_NESTED_STRUCT_FIELDS(XX) \
    XX(uint32_t, nested_id, false, false) \
    XX(std::string, description, false, false)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(TestNestedStruct, "TestNestedStructDB", TEST_NESTED_STRUCT_FIELDS, {
    SPACETIMEDB_FIELD("nested_id", SpacetimeDb::CoreType::U32),
    SPACETIMEDB_FIELD("description", SpacetimeDb::CoreType::String)
});


// Test Outer Struct
struct TestOuterStruct {
    uint64_t id;
    std::string name;
    TestStatus status;
    SpacetimeDB::Types::uint128_t_placeholder big_num;

    std::optional<int32_t> optional_code;
    std::optional<std::string> optional_memo;
    std::optional<TestNestedStruct> optional_nested;

    std::vector<uint8_t> byte_vector; // std::vector<std::byte> would be more accurate for BSATN bytes type
    std::vector<std::string> string_vector;
    std::vector<TestNestedStruct> nested_vector;

    bool operator==(const TestOuterStruct& other) const {
        return id == other.id &&
               name == other.name &&
               status == other.status &&
               big_num == other.big_num &&
               optional_code == other.optional_code &&
               optional_memo == other.optional_memo &&
               optional_nested == other.optional_nested &&
               byte_vector == other.byte_vector &&
               string_vector == other.string_vector &&
               nested_vector == other.nested_vector;
    }
};

// Fields macro for TestOuterStruct
// For optional<T>, T is the first arg. For vector<T>, T is the first arg.
#define TEST_OUTER_STRUCT_FIELDS(XX) \
    XX(uint64_t, id, false, false) \
    XX(std::string, name, false, false) \
    XX(TestStatus, status, false, false) \
    XX(SpacetimeDB::Types::uint128_t_placeholder, big_num, false, false) \
    XX(int32_t, optional_code, true, false) \
    XX(std::string, optional_memo, true, false) \
    XX(TestNestedStruct, optional_nested, true, false) \
    XX(uint8_t, byte_vector, false, true) \
    XX(std::string, string_vector, false, true) \
    XX(TestNestedStruct, nested_vector, false, true)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(TestOuterStruct, "TestOuterStructDB", TEST_OUTER_STRUCT_FIELDS, {
    SPACETIMEDB_FIELD("id", SpacetimeDb::CoreType::U64),
    SPACETIMEDB_FIELD("name", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD_CUSTOM("status", "TestStatusDB"), // Assuming SpacetimeDbName is used for lookup
    SPACETIMEDB_FIELD("big_num", SpacetimeDb::CoreType::U128),
    SPACETIMEDB_FIELD_OPTIONAL("optional_code", SpacetimeDb::CoreType::I32),
    SPACETIMEDB_FIELD_OPTIONAL("optional_memo", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("optional_nested", "TestNestedStructDB"),
    // Vector registration needs a new helper or specific CoreType variant if not just "Bytes"
    // For now, let's assume vector fields are not explicitly registered in ModuleSchema in detail,
    // but the (de)serialization macros will handle them.
    // Or, we'd need a SPACETIMEDB_FIELD_VECTOR helper.
    // The current SPACETIMEDB_FIELD only takes a single CoreType or UserDefinedTypeName.
    // Let's simplify registration for vectors for now and focus on (de)serialization generation.
    // The schema registration part might need further enhancement for vector<primitive> and vector<custom>.
    SPACETIMEDB_FIELD("byte_vector", SpacetimeDb::CoreType::Bytes) // This maps to std::vector<std::byte>
    // For string_vector and nested_vector, schema registration is more complex.
    // For this test, we'll focus on the generated (de)serialization.
});


inline void print_bytes(const std::vector<std::byte>& bytes) {
    std::cout << std::hex << std::setfill('0');
    for (std::byte b : bytes) {
        std::cout << std::setw(2) << static_cast<uint32_t>(b) << " ";
    }
    std::cout << std::dec << std::endl;
}

inline void run_bsatn_tests() {
    std::cout << "Running BSATN (De)serialization Tests..." << std::endl;

    // Test Enum
    TestStatus status_orig = TestStatus::Active;
    bsatn::Writer status_writer;
    SpacetimeDB::bsatn::serialize(status_writer, status_orig);
    std::vector<std::byte> status_bytes = status_writer.take_buffer();
    bsatn::Reader status_reader(status_bytes);
    TestStatus status_deser = SpacetimeDB::bsatn::deserialize_TestStatus(status_reader);
    assert(status_orig == status_deser);
    std::cout << "TestStatus (De)serialization: SUCCESS" << std::endl;

    // Test Nested Struct
    TestNestedStruct nested_orig;
    nested_orig.nested_id = 123;
    nested_orig.description = "I am nested.";

    bsatn::Writer nested_writer;
    SpacetimeDB::bsatn::serialize(nested_writer, nested_orig);
    std::vector<std::byte> nested_bytes = nested_writer.take_buffer();
    bsatn::Reader nested_reader(nested_bytes);
    TestNestedStruct nested_deser = SpacetimeDB::bsatn::deserialize_TestNestedStruct(nested_reader);
    assert(nested_orig == nested_deser);
     std::cout << "TestNestedStruct (De)serialization: SUCCESS" << std::endl;


    // Test Outer Struct
    TestOuterStruct original;
    original.id = 101;
    original.name = "TestObject";
    original.status = TestStatus::Active;
    original.big_num = {0x123456789abcdef0ULL, 0x0fedcba987654321ULL};
    original.optional_code = 42;
    original.optional_memo = "This is an optional memo.";
    original.optional_nested = TestNestedStruct{202, "Optional nested struct"};
    original.byte_vector = {std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};
    original.string_vector = {"hello", "world", "bsatn"};
    original.nested_vector = {
        {303, "Nested vec item 1"},
        {404, "Nested vec item 2"}
    };

    bsatn::Writer writer;
    SpacetimeDB::bsatn::serialize(writer, original);
    std::vector<std::byte> serialized_bytes = writer.take_buffer();

    std::cout << "Serialized TestOuterStruct (" << serialized_bytes.size() << " bytes): ";
    print_bytes(serialized_bytes);

    bsatn::Reader reader(serialized_bytes);
    TestOuterStruct deserialized = SpacetimeDB::bsatn::deserialize_TestOuterStruct(reader);

    assert(original == deserialized);
    std::cout << "TestOuterStruct (De)serialization: SUCCESS" << std::endl;

    // Test optional absent
    TestOuterStruct original_optional_absent = original;
    original_optional_absent.optional_code.reset();
    original_optional_absent.optional_memo.reset();
    original_optional_absent.optional_nested.reset();

    bsatn::Writer writer2;
    SpacetimeDB::bsatn::serialize(writer2, original_optional_absent);
    std::vector<std::byte> serialized_bytes2 = writer2.take_buffer();

    std::cout << "Serialized TestOuterStruct with absent optionals (" << serialized_bytes2.size() << " bytes): ";
    print_bytes(serialized_bytes2);

    bsatn::Reader reader2(serialized_bytes2);
    TestOuterStruct deserialized2 = SpacetimeDB::bsatn::deserialize_TestOuterStruct(reader2);
    assert(original_optional_absent == deserialized2);
    std::cout << "TestOuterStruct with absent optionals (De)serialization: SUCCESS" << std::endl;


    std::cout << "All BSATN tests passed!" << std::endl;
}


#endif // SPACETIME_TEST_TYPES_H
