#ifndef SDK_TEST_H
#define SDK_TEST_H

#include <spacetimedb/macros.h>
#include <spacetimedb/sdk/spacetimedb_sdk_types.h> // For Identity, Timestamp (though not used yet)
#include <string>
#include <vector>
#include <variant> // For EnumWithPayload later
#include <cstdint> // For uint8_t etc.

namespace sdk_test_cpp {

enum class SimpleEnum : uint8_t {
    Zero,
    One,
    Two,
};

// Schema registration for SimpleEnum
SPACETIMEDB_TYPE_ENUM(
    sdk_test_cpp::SimpleEnum,
    "SimpleEnum",
    {
        SPACETIMEDB_ENUM_VARIANT("Zero"),
        SPACETIMEDB_ENUM_VARIANT("One"),
        SPACETIMEDB_ENUM_VARIANT("Two")
    }
);

struct UnitStruct {
    // bool operator==(const UnitStruct&) const { return true; } // Optional
};

#define UNIT_STRUCT_FIELDS(XX) /* No fields */
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::UnitStruct,
    "UnitStruct",
    UNIT_STRUCT_FIELDS,
    { /* No fields */ }
);

struct ByteStruct {
    uint8_t b;
    // bool operator==(const ByteStruct& other) const { return b == other.b; } // Optional
};

#define BYTE_STRUCT_FIELDS(XX) \
    XX(uint8_t, b, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::ByteStruct,
    "ByteStruct",
    BYTE_STRUCT_FIELDS,
    {
        SPACETIMEDB_FIELD("b", SpacetimeDb::CoreType::U8, false, false)
    }
);

// Forward declare SpacetimeDB SDK types if not fully included or for clarity
// For this example, we assume spacetimedb_sdk_types.h provides these if they are structs:
// namespace spacetimedb { namespace sdk {
//     struct u128; struct i128; struct u256; struct i256;
//     struct Identity; struct ConnectionId; struct Timestamp; struct TimeDuration;
// }}

struct EveryPrimitiveStruct {
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint64_t d;
    spacetimedb::sdk::u128 e;
    spacetimedb::sdk::u256 f;
    int8_t g;
    int16_t h;
    int32_t i;
    int64_t j;
    spacetimedb::sdk::i128 k;
    spacetimedb::sdk::i256 l;
    bool m;
    float n;
    double o;
    std::string p;
    spacetimedb::sdk::Identity q;
    spacetimedb::sdk::ConnectionId r; // Assuming this is a type from spacetimedb_sdk_types.h
    spacetimedb::sdk::Timestamp s;
    spacetimedb::sdk::TimeDuration t; // Assuming this is a type from spacetimedb_sdk_types.h
};

#define EVERY_PRIMITIVE_STRUCT_FIELDS(XX) \
    XX(uint8_t, a, false, false) \
    XX(uint16_t, b, false, false) \
    XX(uint32_t, c, false, false) \
    XX(uint64_t, d, false, false) \
    XX(spacetimedb::sdk::u128, e, false, false) \
    XX(spacetimedb::sdk::u256, f, false, false) \
    XX(int8_t, g, false, false) \
    XX(int16_t, h, false, false) \
    XX(int32_t, i, false, false) \
    XX(int64_t, j, false, false) \
    XX(spacetimedb::sdk::i128, k, false, false) \
    XX(spacetimedb::sdk::i256, l, false, false) \
    XX(bool, m, false, false) \
    XX(float, n, false, false) \
    XX(double, o, false, false) \
    XX(std::string, p, false, false) \
    XX(spacetimedb::sdk::Identity, q, false, false) \
    XX(spacetimedb::sdk::ConnectionId, r, false, false) \
    XX(spacetimedb::sdk::Timestamp, s, false, false) \
    XX(spacetimedb::sdk::TimeDuration, t, false, false)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::EveryPrimitiveStruct,
    "EveryPrimitiveStruct",
    EVERY_PRIMITIVE_STRUCT_FIELDS,
    {
        SPACETIMEDB_FIELD("a", SpacetimeDb::CoreType::U8, false, false),
        SPACETIMEDB_FIELD("b", SpacetimeDb::CoreType::U16, false, false),
        SPACETIMEDB_FIELD("c", SpacetimeDb::CoreType::U32, false, false),
        SPACETIMEDB_FIELD("d", SpacetimeDb::CoreType::U64, false, false),
        SPACETIMEDB_FIELD("e", SpacetimeDb::CoreType::U128, false, false),
        SPACETIMEDB_FIELD("f", SpacetimeDb::CoreType::U256, false, false),
        SPACETIMEDB_FIELD("g", SpacetimeDb::CoreType::I8, false, false),
        SPACETIMEDB_FIELD("h", SpacetimeDb::CoreType::I16, false, false),
        SPACETIMEDB_FIELD("i", SpacetimeDb::CoreType::I32, false, false),
        SPACETIMEDB_FIELD("j", SpacetimeDb::CoreType::I64, false, false),
        SPACETIMEDB_FIELD("k", SpacetimeDb::CoreType::I128, false, false),
        SPACETIMEDB_FIELD("l", SpacetimeDb::CoreType::I256, false, false),
        SPACETIMEDB_FIELD("m", SpacetimeDb::CoreType::Bool, false, false),
        SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::F32, false, false),
        SPACETIMEDB_FIELD("o", SpacetimeDb::CoreType::F64, false, false),
        SPACETIMEDB_FIELD("p", SpacetimeDb::CoreType::String, false, false),
        SPACETIMEDB_FIELD_CUSTOM("q", "Identity", false, false),
        SPACETIMEDB_FIELD_CUSTOM("r", "ConnectionId", false, false), // Assuming ConnectionId is a registered struct
        SPACETIMEDB_FIELD_CUSTOM("s", "Timestamp", false, false),
        SPACETIMEDB_FIELD_CUSTOM("t", "TimeDuration", false, false)
    }
);

struct EveryVecStruct {
    std::vector<uint8_t> a;
    std::vector<uint16_t> b;
    std::vector<uint32_t> c;
    std::vector<uint64_t> d;
    std::vector<spacetimedb::sdk::u128> e;
    std::vector<spacetimedb::sdk::u256> f;
    std::vector<int8_t> g;
    std::vector<int16_t> h;
    std::vector<int32_t> i;
    std::vector<int64_t> j;
    std::vector<spacetimedb::sdk::i128> k;
    std::vector<spacetimedb::sdk::i256> l;
    std::vector<bool> m;
    std::vector<float> n;
    std::vector<double> o;
    std::vector<std::string> p;
    std::vector<spacetimedb::sdk::Identity> q;
    std::vector<spacetimedb::sdk::ConnectionId> r;
    std::vector<spacetimedb::sdk::Timestamp> s;
    std::vector<spacetimedb::sdk::TimeDuration> t;
};

#define EVERY_VEC_STRUCT_FIELDS(XX) \
    XX(uint8_t, a, false, true) \
    XX(uint16_t, b, false, true) \
    XX(uint32_t, c, false, true) \
    XX(uint64_t, d, false, true) \
    XX(spacetimedb::sdk::u128, e, false, true) \
    XX(spacetimedb::sdk::u256, f, false, true) \
    XX(int8_t, g, false, true) \
    XX(int16_t, h, false, true) \
    XX(int32_t, i, false, true) \
    XX(int64_t, j, false, true) \
    XX(spacetimedb::sdk::i128, k, false, true) \
    XX(spacetimedb::sdk::i256, l, false, true) \
    XX(bool, m, false, true) \
    XX(float, n, false, true) \
    XX(double, o, false, true) \
    XX(std::string, p, false, true) \
    XX(spacetimedb::sdk::Identity, q, false, true) \
    XX(spacetimedb::sdk::ConnectionId, r, false, true) \
    XX(spacetimedb::sdk::Timestamp, s, false, true) \
    XX(spacetimedb::sdk::TimeDuration, t, false, true)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::EveryVecStruct,
    "EveryVecStruct",
    EVERY_VEC_STRUCT_FIELDS,
    { // Schema describes element types. Vector nature is implicit in C++ type + X-Macro IS_VECTOR flag.
        SPACETIMEDB_FIELD("a", SpacetimeDb::CoreType::U8, false, false),
        SPACETIMEDB_FIELD("b", SpacetimeDb::CoreType::U16, false, false),
        SPACETIMEDB_FIELD("c", SpacetimeDb::CoreType::U32, false, false),
        SPACETIMEDB_FIELD("d", SpacetimeDb::CoreType::U64, false, false),
        SPACETIMEDB_FIELD("e", SpacetimeDb::CoreType::U128, false, false),
        SPACETIMEDB_FIELD("f", SpacetimeDb::CoreType::U256, false, false),
        SPACETIMEDB_FIELD("g", SpacetimeDb::CoreType::I8, false, false),
        SPACETIMEDB_FIELD("h", SpacetimeDb::CoreType::I16, false, false),
        SPACETIMEDB_FIELD("i", SpacetimeDb::CoreType::I32, false, false),
        SPACETIMEDB_FIELD("j", SpacetimeDb::CoreType::I64, false, false),
        SPACETIMEDB_FIELD("k", SpacetimeDb::CoreType::I128, false, false),
        SPACETIMEDB_FIELD("l", SpacetimeDb::CoreType::I256, false, false),
        SPACETIMEDB_FIELD("m", SpacetimeDb::CoreType::Bool, false, false),
        SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::F32, false, false),
        SPACETIMEDB_FIELD("o", SpacetimeDb::CoreType::F64, false, false),
        SPACETIMEDB_FIELD("p", SpacetimeDb::CoreType::String, false, false),
        SPACETIMEDB_FIELD_CUSTOM("q", "Identity", false, false),
        SPACETIMEDB_FIELD_CUSTOM("r", "ConnectionId", false, false),
        SPACETIMEDB_FIELD_CUSTOM("s", "Timestamp", false, false),
        SPACETIMEDB_FIELD_CUSTOM("t", "TimeDuration", false, false)
    }
);

// Forward declare for bsatn methods if necessary, or define fully before registration
struct EnumWithPayload;

// Define an enum for the tags
enum class EnumWithPayloadTag : uint8_t {
    TagU8, TagU16, TagU32, TagU64, TagU128, TagU256,
    TagI8, TagI16, TagI32, TagI64, TagI128, TagI256,
    TagBool, TagF32, TagF64, TagStr, TagIdentity, TagConnectionId,
    TagTimestamp, TagBytes, TagInts, TagStrings, TagSimpleEnums
};

// Manual BSATN for EnumWithPayloadTag
inline void serialize(SpacetimeDB::bsatn::Writer& writer, const EnumWithPayloadTag& value) {
    writer.write_u8(static_cast<uint8_t>(value));
}
inline EnumWithPayloadTag deserialize_EnumWithPayloadTag(SpacetimeDB::bsatn::Reader& reader) {
    return static_cast<EnumWithPayloadTag>(reader.read_u8());
}
} // namespace sdk_test_cpp

// Specialization for bsatn::deserialize must be in bsatn namespace
namespace bsatn {
    template<>
    inline sdk_test_cpp::EnumWithPayloadTag deserialize<sdk_test_cpp::EnumWithPayloadTag>(Reader& reader) {
        return sdk_test_cpp::deserialize_EnumWithPayloadTag(reader);
    }
} // namespace bsatn

namespace sdk_test_cpp {
struct EnumWithPayload {
    EnumWithPayloadTag tag;
    std::variant<
        uint8_t, uint16_t, uint32_t, uint64_t, spacetimedb::sdk::u128, spacetimedb::sdk::u256,
        int8_t, int16_t, int32_t, int64_t, spacetimedb::sdk::i128, spacetimedb::sdk::i256,
        bool, float, double, std::string, spacetimedb::sdk::Identity, spacetimedb::sdk::ConnectionId,
        spacetimedb::sdk::Timestamp, std::vector<uint8_t>, std::vector<int32_t>,
        std::vector<std::string>, std::vector<SimpleEnum>
    > value;

    // Manual BSATN methods (declarations)
    void bsatn_serialize(SpacetimeDB::bsatn::Writer& writer) const;
    void bsatn_deserialize(SpacetimeDB::bsatn::Reader& reader);
};

// Schema registration for EnumWithPayloadTag (if not already done by being part of another module)
// For this example, let's assume it needs explicit registration if it's a standalone type used in schema.
// However, SPACETIMEDB_TYPE_ENUM requires an enum class, not a free function.
// So, EnumWithPayloadTag will be registered via SPACETIMEDB_FIELD_CUSTOM("tag", "EnumWithPayloadTag", ...)
// and we assume "EnumWithPayloadTag" corresponds to a schema type that is simply a U8.
// A full registration for EnumWithPayloadTag like SimpleEnum isn't standard for a tag enum if its only purpose is to be a field.
// But if it WERE to be registered like SimpleEnum:
SPACETIMEDB_TYPE_ENUM(sdk_test_cpp::EnumWithPayloadTag, "EnumWithPayloadTag", {
    SPACETIMEDB_ENUM_VARIANT("TagU8"), SPACETIMEDB_ENUM_VARIANT("TagU16"),
    SPACETIMEDB_ENUM_VARIANT("TagU32"), SPACETIMEDB_ENUM_VARIANT("TagU64"),
    SPACETIMEDB_ENUM_VARIANT("TagU128"), SPACETIMEDB_ENUM_VARIANT("TagU256"),
    SPACETIMEDB_ENUM_VARIANT("TagI8"), SPACETIMEDB_ENUM_VARIANT("TagI16"),
    SPACETIMEDB_ENUM_VARIANT("TagI32"), SPACETIMEDB_ENUM_VARIANT("TagI64"),
    SPACETIMEDB_ENUM_VARIANT("TagI128"), SPACETIMEDB_ENUM_VARIANT("TagI256"),
    SPACETIMEDB_ENUM_VARIANT("TagBool"), SPACETIMEDB_ENUM_VARIANT("TagF32"),
    SPACETIMEDB_ENUM_VARIANT("TagF64"), SPACETIMEDB_ENUM_VARIANT("TagStr"),
    SPACETIMEDB_ENUM_VARIANT("TagIdentity"), SPACETIMEDB_ENUM_VARIANT("TagConnectionId"),
    SPACETIMEDB_ENUM_VARIANT("TagTimestamp"), SPACETIMEDB_ENUM_VARIANT("TagBytes"),
    SPACETIMEDB_ENUM_VARIANT("TagInts"), SPACETIMEDB_ENUM_VARIANT("TagStrings"),
    SPACETIMEDB_ENUM_VARIANT("TagSimpleEnums")
});


#define ENUM_WITH_PAYLOAD_FIELDS(XX) \
    XX(sdk_test_cpp::EnumWithPayloadTag, tag, false, false) \
    XX(std::vector<uint8_t>, payload_bytes_for_schema, false, false) // Placeholder for schema

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::EnumWithPayload,
    "EnumWithPayload",
    ENUM_WITH_PAYLOAD_FIELDS,
    {
        SPACETIMEDB_FIELD_CUSTOM("tag", "EnumWithPayloadTag", false, false),
        SPACETIMEDB_FIELD("payload_bytes_for_schema", SpacetimeDb::CoreType::Bytes, false, false)
    }
);

//
// 'One*' Series of Row Structs for Tables
//

// OneU8
struct OneU8Row {
    uint8_t n;
};
#define ONE_U8_ROW_FIELDS(XX) XX(uint8_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneU8Row, "OneU8Row", ONE_U8_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U8, false, false) }
);

// OneU16
struct OneU16Row {
    uint16_t n;
};
#define ONE_U16_ROW_FIELDS(XX) XX(uint16_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneU16Row, "OneU16Row", ONE_U16_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U16, false, false) }
);

// OneU32
struct OneU32Row {
    uint32_t n;
};
#define ONE_U32_ROW_FIELDS(XX) XX(uint32_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneU32Row, "OneU32Row", ONE_U32_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U32, false, false) }
);

// OneU64
struct OneU64Row {
    uint64_t n;
};
#define ONE_U64_ROW_FIELDS(XX) XX(uint64_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneU64Row, "OneU64Row", ONE_U64_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U64, false, false) }
);

// OneU128
struct OneU128Row {
    spacetimedb::sdk::u128 n;
};
#define ONE_U128_ROW_FIELDS(XX) XX(spacetimedb::sdk::u128, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneU128Row, "OneU128Row", ONE_U128_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U128, false, false) }
);

// OneU256
struct OneU256Row {
    spacetimedb::sdk::u256 n;
};
#define ONE_U256_ROW_FIELDS(XX) XX(spacetimedb::sdk::u256, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneU256Row, "OneU256Row", ONE_U256_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U256, false, false) }
);

// OneI8
struct OneI8Row {
    int8_t n;
};
#define ONE_I8_ROW_FIELDS(XX) XX(int8_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneI8Row, "OneI8Row", ONE_I8_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::I8, false, false) }
);

// OneI16
struct OneI16Row {
    int16_t n;
};
#define ONE_I16_ROW_FIELDS(XX) XX(int16_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneI16Row, "OneI16Row", ONE_I16_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::I16, false, false) }
);

// OneI32
struct OneI32Row {
    int32_t n;
};
#define ONE_I32_ROW_FIELDS(XX) XX(int32_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneI32Row, "OneI32Row", ONE_I32_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::I32, false, false) }
);

// OneI64
struct OneI64Row {
    int64_t n;
};
#define ONE_I64_ROW_FIELDS(XX) XX(int64_t, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneI64Row, "OneI64Row", ONE_I64_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::I64, false, false) }
);

// OneI128
struct OneI128Row {
    spacetimedb::sdk::i128 n;
};
#define ONE_I128_ROW_FIELDS(XX) XX(spacetimedb::sdk::i128, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneI128Row, "OneI128Row", ONE_I128_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::I128, false, false) }
);

// OneI256
struct OneI256Row {
    spacetimedb::sdk::i256 n;
};
#define ONE_I256_ROW_FIELDS(XX) XX(spacetimedb::sdk::i256, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneI256Row, "OneI256Row", ONE_I256_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::I256, false, false) }
);

// OneBool
struct OneBoolRow {
    bool n;
};
#define ONE_BOOL_ROW_FIELDS(XX) XX(bool, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneBoolRow, "OneBoolRow", ONE_BOOL_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::Bool, false, false) }
);

// OneF32
struct OneF32Row {
    float n;
};
#define ONE_F32_ROW_FIELDS(XX) XX(float, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneF32Row, "OneF32Row", ONE_F32_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::F32, false, false) }
);

// OneF64
struct OneF64Row {
    double n;
};
#define ONE_F64_ROW_FIELDS(XX) XX(double, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneF64Row, "OneF64Row", ONE_F64_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::F64, false, false) }
);

// OneString
struct OneStringRow {
    std::string n;
};
#define ONE_STRING_ROW_FIELDS(XX) XX(std::string, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneStringRow, "OneStringRow", ONE_STRING_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::String, false, false) }
);

// OneIdentity
struct OneIdentityRow {
    spacetimedb::sdk::Identity n;
};
#define ONE_IDENTITY_ROW_FIELDS(XX) XX(spacetimedb::sdk::Identity, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneIdentityRow, "OneIdentityRow", ONE_IDENTITY_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("n", "Identity", false, false) }
);

// OneConnectionId
struct OneConnectionIdRow {
    spacetimedb::sdk::ConnectionId n;
};
#define ONE_CONNECTION_ID_ROW_FIELDS(XX) XX(spacetimedb::sdk::ConnectionId, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneConnectionIdRow, "OneConnectionIdRow", ONE_CONNECTION_ID_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("n", "ConnectionId", false, false) } // Assuming ConnectionId is registered type
);

// OneTimestamp
struct OneTimestampRow {
    spacetimedb::sdk::Timestamp n;
};
#define ONE_TIMESTAMP_ROW_FIELDS(XX) XX(spacetimedb::sdk::Timestamp, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneTimestampRow, "OneTimestampRow", ONE_TIMESTAMP_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("n", "Timestamp", false, false) }
);

// OneSimpleEnum
struct OneSimpleEnumRow {
    SimpleEnum n;
};
#define ONE_SIMPLE_ENUM_ROW_FIELDS(XX) XX(sdk_test_cpp::SimpleEnum, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneSimpleEnumRow, "OneSimpleEnumRow", ONE_SIMPLE_ENUM_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("n", "SimpleEnum", false, false) }
);

// OneEnumWithPayload
struct OneEnumWithPayloadRow {
    EnumWithPayload n;
};
#define ONE_ENUM_WITH_PAYLOAD_ROW_FIELDS(XX) XX(sdk_test_cpp::EnumWithPayload, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneEnumWithPayloadRow, "OneEnumWithPayloadRow", ONE_ENUM_WITH_PAYLOAD_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("n", "EnumWithPayload", false, false) }
);

// OneUnitStruct
struct OneUnitStructRow {
    UnitStruct s; // Field name 's' as in Rust example (though it's a unit struct)
};
#define ONE_UNIT_STRUCT_ROW_FIELDS(XX) XX(sdk_test_cpp::UnitStruct, s, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneUnitStructRow, "OneUnitStructRow", ONE_UNIT_STRUCT_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("s", "UnitStruct", false, false) }
);

// OneByteStruct
struct OneByteStructRow {
    ByteStruct s;
};
#define ONE_BYTE_STRUCT_ROW_FIELDS(XX) XX(sdk_test_cpp::ByteStruct, s, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneByteStructRow, "OneByteStructRow", ONE_BYTE_STRUCT_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("s", "ByteStruct", false, false) }
);

// OneEveryPrimitiveStruct
struct OneEveryPrimitiveStructRow {
    EveryPrimitiveStruct s;
};
#define ONE_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS(XX) XX(sdk_test_cpp::EveryPrimitiveStruct, s, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneEveryPrimitiveStructRow, "OneEveryPrimitiveStructRow", ONE_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("s", "EveryPrimitiveStruct", false, false) }
);

// OneEveryVecStruct
struct OneEveryVecStructRow {
    EveryVecStruct s;
};
#define ONE_EVERY_VEC_STRUCT_ROW_FIELDS(XX) XX(sdk_test_cpp::EveryVecStruct, s, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OneEveryVecStructRow, "OneEveryVecStructRow", ONE_EVERY_VEC_STRUCT_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("s", "EveryVecStruct", false, false) }
);

//
// "Vec*" Series of Row Structs for Tables
//

// VecU8
struct VecU8Row { std::vector<uint8_t> n; };
#define VEC_U8_ROW_FIELDS(XX) XX(uint8_t, n, false, true)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::VecU8Row, "VecU8Row", VEC_U8_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U8, false, false) } // Schema for element type
);

// VecString
struct VecStringRow { std::vector<std::string> n; };
#define VEC_STRING_ROW_FIELDS(XX) XX(std::string, n, false, true)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::VecStringRow, "VecStringRow", VEC_STRING_ROW_FIELDS,
    { SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::String, false, false) }
);

// VecSimpleEnum
struct VecSimpleEnumRow { std::vector<SimpleEnum> n; };
#define VEC_SIMPLE_ENUM_ROW_FIELDS(XX) XX(sdk_test_cpp::SimpleEnum, n, false, true)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::VecSimpleEnumRow, "VecSimpleEnumRow", VEC_SIMPLE_ENUM_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("n", "SimpleEnum", false, false) }
);

// VecEveryPrimitiveStruct
struct VecEveryPrimitiveStructRow { std::vector<EveryPrimitiveStruct> s; };
#define VEC_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS(XX) XX(sdk_test_cpp::EveryPrimitiveStruct, s, false, true)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::VecEveryPrimitiveStructRow, "VecEveryPrimitiveStructRow", VEC_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS,
    { SPACETIMEDB_FIELD_CUSTOM("s", "EveryPrimitiveStruct", false, false) }
);

//
// "Unique*" Series of Row Structs for Tables
//

// UniqueU8Row
struct UniqueU8Row {
    uint64_t row_id;
    uint8_t n;
    int32_t data;
};
#define UNIQUE_U8_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(uint8_t, n, true, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::UniqueU8Row, "UniqueU8Row", UNIQUE_U8_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U8, true, false),
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// UniqueStringRow
struct UniqueStringRow {
    uint64_t row_id;
    std::string s;
    int32_t data;
};
#define UNIQUE_STRING_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(std::string, s, true, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::UniqueStringRow, "UniqueStringRow", UNIQUE_STRING_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD("s", SpacetimeDb::CoreType::String, true, false),
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// UniqueIdentityRow
struct UniqueIdentityRow {
    uint64_t row_id;
    spacetimedb::sdk::Identity i;
    int32_t data;
};
#define UNIQUE_IDENTITY_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(spacetimedb::sdk::Identity, i, true, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::UniqueIdentityRow, "UniqueIdentityRow", UNIQUE_IDENTITY_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD_CUSTOM("i", "Identity", true, false),
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// UniqueConnectionIdRow
struct UniqueConnectionIdRow {
    uint64_t row_id;
    spacetimedb::sdk::ConnectionId c; // Assuming ConnectionId is a distinct type
    int32_t data;
};
#define UNIQUE_CONNECTION_ID_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(spacetimedb::sdk::ConnectionId, c, true, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::UniqueConnectionIdRow, "UniqueConnectionIdRow", UNIQUE_CONNECTION_ID_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD_CUSTOM("c", "ConnectionId", true, false), // Assuming ConnectionId is registered as "ConnectionId"
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);


//
// Remaining Complex and Manually Defined Tables
//

// LargeTableRow
struct LargeTableRow {
    uint64_t row_id; // PK
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint64_t d;
    spacetimedb::sdk::u128 e;
    spacetimedb::sdk::u256 f;
    int8_t g;
    int16_t h;
    int32_t i;
    int64_t j;
    spacetimedb::sdk::i128 k;
    spacetimedb::sdk::i256 l;
    bool m;
    float n;
    double o;
    std::string p;
    spacetimedb::sdk::Identity q;
    spacetimedb::sdk::ConnectionId r;
    spacetimedb::sdk::Timestamp s;
    spacetimedb::sdk::TimeDuration t;
    std::vector<uint8_t> va;
    std::vector<std::string> vp;
    std::vector<EveryPrimitiveStruct> vs;
    std::optional<int32_t> oi;
    std::optional<std::string> os;
    std::optional<EveryPrimitiveStruct> oes;
    SimpleEnum en;
    EnumWithPayload ewp;
    UnitStruct us;
    ByteStruct bs;
    EveryPrimitiveStruct eps;
    EveryVecStruct evs;
};

#define LARGE_TABLE_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(uint8_t, a, false, false) \
    XX(uint16_t, b, false, false) \
    XX(uint32_t, c, false, false) \
    XX(uint64_t, d, false, false) \
    XX(spacetimedb::sdk::u128, e, false, false) \
    XX(spacetimedb::sdk::u256, f, false, false) \
    XX(int8_t, g, false, false) \
    XX(int16_t, h, false, false) \
    XX(int32_t, i, false, false) \
    XX(int64_t, j, false, false) \
    XX(spacetimedb::sdk::i128, k, false, false) \
    XX(spacetimedb::sdk::i256, l, false, false) \
    XX(bool, m, false, false) \
    XX(float, n, false, false) \
    XX(double, o, false, false) \
    XX(std::string, p, false, false) \
    XX(spacetimedb::sdk::Identity, q, false, false) \
    XX(spacetimedb::sdk::ConnectionId, r, false, false) \
    XX(spacetimedb::sdk::Timestamp, s, false, false) \
    XX(spacetimedb::sdk::TimeDuration, t, false, false) \
    XX(uint8_t, va, false, true) /* Vector of u8 */ \
    XX(std::string, vp, false, true) /* Vector of string */ \
    XX(sdk_test_cpp::EveryPrimitiveStruct, vs, false, true) /* Vector of EveryPrimitiveStruct */ \
    XX(int32_t, oi, true, false) /* Optional i32 */ \
    XX(std::string, os, true, false) /* Optional string */ \
    XX(sdk_test_cpp::EveryPrimitiveStruct, oes, true, false) /* Optional EveryPrimitiveStruct */ \
    XX(sdk_test_cpp::SimpleEnum, en, false, false) \
    XX(sdk_test_cpp::EnumWithPayload, ewp, false, false) \
    XX(sdk_test_cpp::UnitStruct, us, false, false) \
    XX(sdk_test_cpp::ByteStruct, bs, false, false) \
    XX(sdk_test_cpp::EveryPrimitiveStruct, eps, false, false) \
    XX(sdk_test_cpp::EveryVecStruct, evs, false, false)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::LargeTableRow, "LargeTableRow", LARGE_TABLE_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD("a", SpacetimeDb::CoreType::U8, false, false),
        SPACETIMEDB_FIELD("b", SpacetimeDb::CoreType::U16, false, false),
        SPACETIMEDB_FIELD("c", SpacetimeDb::CoreType::U32, false, false),
        SPACETIMEDB_FIELD("d", SpacetimeDb::CoreType::U64, false, false),
        SPACETIMEDB_FIELD("e", SpacetimeDb::CoreType::U128, false, false),
        SPACETIMEDB_FIELD("f", SpacetimeDb::CoreType::U256, false, false),
        SPACETIMEDB_FIELD("g", SpacetimeDb::CoreType::I8, false, false),
        SPACETIMEDB_FIELD("h", SpacetimeDb::CoreType::I16, false, false),
        SPACETIMEDB_FIELD("i", SpacetimeDb::CoreType::I32, false, false),
        SPACETIMEDB_FIELD("j", SpacetimeDb::CoreType::I64, false, false),
        SPACETIMEDB_FIELD("k", SpacetimeDb::CoreType::I128, false, false),
        SPACETIMEDB_FIELD("l", SpacetimeDb::CoreType::I256, false, false),
        SPACETIMEDB_FIELD("m", SpacetimeDb::CoreType::Bool, false, false),
        SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::F32, false, false),
        SPACETIMEDB_FIELD("o", SpacetimeDb::CoreType::F64, false, false),
        SPACETIMEDB_FIELD("p", SpacetimeDb::CoreType::String, false, false),
        SPACETIMEDB_FIELD_CUSTOM("q", "Identity", false, false),
        SPACETIMEDB_FIELD_CUSTOM("r", "ConnectionId", false, false),
        SPACETIMEDB_FIELD_CUSTOM("s", "Timestamp", false, false),
        SPACETIMEDB_FIELD_CUSTOM("t", "TimeDuration", false, false),
        SPACETIMEDB_FIELD("va", SpacetimeDb::CoreType::U8, false, false), // Schema for element type
        SPACETIMEDB_FIELD("vp", SpacetimeDb::CoreType::String, false, false), // Schema for element type
        SPACETIMEDB_FIELD_CUSTOM("vs", "EveryPrimitiveStruct", false, false), // Schema for element type
        SPACETIMEDB_FIELD_OPTIONAL("oi", SpacetimeDb::CoreType::I32, false, false),
        SPACETIMEDB_FIELD_OPTIONAL("os", SpacetimeDb::CoreType::String, false, false),
        SPACETIMEDB_FIELD_OPTIONAL_CUSTOM("oes", "EveryPrimitiveStruct", false, false),
        SPACETIMEDB_FIELD_CUSTOM("en", "SimpleEnum", false, false),
        SPACETIMEDB_FIELD_CUSTOM("ewp", "EnumWithPayload", false, false),
        SPACETIMEDB_FIELD_CUSTOM("us", "UnitStruct", false, false),
        SPACETIMEDB_FIELD_CUSTOM("bs", "ByteStruct", false, false),
        SPACETIMEDB_FIELD_CUSTOM("eps", "EveryPrimitiveStruct", false, false),
        SPACETIMEDB_FIELD_CUSTOM("evs", "EveryVecStruct", false, false)
    }
);

// TableHoldsTableRow
struct TableHoldsTableRow {
    uint64_t row_id; // PK
    OneU8Row a;
    VecU8Row b;
};
#define TABLE_HOLDS_TABLE_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(sdk_test_cpp::OneU8Row, a, false, false) \
    XX(sdk_test_cpp::VecU8Row, b, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::TableHoldsTableRow, "TableHoldsTableRow", TABLE_HOLDS_TABLE_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD_CUSTOM("a", "OneU8Row", false, false),
        SPACETIMEDB_FIELD_CUSTOM("b", "VecU8Row", false, false)
    }
);

// ScheduledTableRow
struct ScheduledTableRow {
    uint64_t scheduled_id; // PK
    spacetimedb::sdk::ScheduleAt scheduled_at;
    std::string text;
};
#define SCHEDULED_TABLE_ROW_FIELDS(XX) \
    XX(uint64_t, scheduled_id, false, true) \
    XX(spacetimedb::sdk::ScheduleAt, scheduled_at, false, false) \
    XX(std::string, text, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::ScheduledTableRow, "ScheduledTableRow", SCHEDULED_TABLE_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("scheduled_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD_CUSTOM("scheduled_at", "ScheduleAt", false, false), // Assuming ScheduleAt is registered
        SPACETIMEDB_FIELD("text", SpacetimeDb::CoreType::String, false, false)
    }
);

// IndexedTableRow
struct IndexedTableRow {
    uint64_t row_id; // PK
    uint32_t player_id;
};
#define INDEXED_TABLE_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(uint32_t, player_id, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::IndexedTableRow, "IndexedTableRow", INDEXED_TABLE_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD("player_id", SpacetimeDb::CoreType::U32, false, false)
    }
);

// IndexedTable2Row
struct IndexedTable2Row {
    uint64_t row_id; // PK
    uint32_t player_id;
    float player_snazz;
};
#define INDEXED_TABLE_2_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(uint32_t, player_id, false, false) \
    XX(float, player_snazz, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::IndexedTable2Row, "IndexedTable2Row", INDEXED_TABLE_2_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD("player_id", SpacetimeDb::CoreType::U32, false, false),
        SPACETIMEDB_FIELD("player_snazz", SpacetimeDb::CoreType::F32, false, false)
    }
);

// BTreeU32Row
struct BTreeU32Row {
    uint64_t row_id; // PK
    uint32_t n;
    int32_t data;
};
#define BTREE_U32_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(uint32_t, n, false, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::BTreeU32Row, "BTreeU32Row", BTREE_U32_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U32, false, false), // Not marked unique here, index will be separate
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// UsersRow
struct UsersRow {
    spacetimedb::sdk::Identity identity; // PK
    std::string name;
};
#define USERS_ROW_FIELDS(XX) \
    XX(spacetimedb::sdk::Identity, identity, false, false) \
    XX(std::string, name, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::UsersRow, "UsersRow", USERS_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD_CUSTOM("identity", "Identity", false, false),
        SPACETIMEDB_FIELD("name", SpacetimeDb::CoreType::String, false, false)
    }
);

// IndexedSimpleEnumRow
struct IndexedSimpleEnumRow {
    uint64_t row_id; // PK
    SimpleEnum n;
};
#define INDEXED_SIMPLE_ENUM_ROW_FIELDS(XX) \
    XX(uint64_t, row_id, false, true) \
    XX(sdk_test_cpp::SimpleEnum, n, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::IndexedSimpleEnumRow, "IndexedSimpleEnumRow", INDEXED_SIMPLE_ENUM_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
        SPACETIMEDB_FIELD_CUSTOM("n", "SimpleEnum", false, false)
    }
);


//
// "Pk*" Series of Row Structs for Tables
//

// PkU8Row
struct PkU8Row {
    uint8_t n;
    int32_t data;
};
#define PK_U8_ROW_FIELDS(XX) \
    XX(uint8_t, n, false, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::PkU8Row, "PkU8Row", PK_U8_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("n", SpacetimeDb::CoreType::U8, false, false), // Uniqueness implied by PK usage
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// PkStringRow
struct PkStringRow {
    std::string s;
    int32_t data;
};
#define PK_STRING_ROW_FIELDS(XX) \
    XX(std::string, s, false, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::PkStringRow, "PkStringRow", PK_STRING_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("s", SpacetimeDb::CoreType::String, false, false),
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// PkIdentityRow
struct PkIdentityRow {
    spacetimedb::sdk::Identity i;
    int32_t data;
};
#define PK_IDENTITY_ROW_FIELDS(XX) \
    XX(spacetimedb::sdk::Identity, i, false, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::PkIdentityRow, "PkIdentityRow", PK_IDENTITY_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD_CUSTOM("i", "Identity", false, false),
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// PkSimpleEnumRow
struct PkSimpleEnumRow {
    SimpleEnum e;
    int32_t data;
};
#define PK_SIMPLE_ENUM_ROW_FIELDS(XX) \
    XX(sdk_test_cpp::SimpleEnum, e, false, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::PkSimpleEnumRow, "PkSimpleEnumRow", PK_SIMPLE_ENUM_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD_CUSTOM("e", "SimpleEnum", false, false),
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);

// PkU32TwoRow (Example of a table with two primary key components, though our macro currently only supports one)
// For now, we'll define it with 'a' as the PK as per simplified approach.
// A proper composite PK would require changes to SPACETIMEDB_PRIMARY_KEY and ModuleSchema.
struct PkU32TwoRow {
    uint32_t a;
    uint32_t b;
    int32_t data;
};
#define PK_U32_TWO_ROW_FIELDS(XX) \
    XX(uint32_t, a, false, false) \
    XX(uint32_t, b, false, false) \
    XX(int32_t, data, false, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::PkU32TwoRow, "PkU32TwoRow", PK_U32_TWO_ROW_FIELDS,
    {
        SPACETIMEDB_FIELD("a", SpacetimeDb::CoreType::U32, false, false),
        SPACETIMEDB_FIELD("b", SpacetimeDb::CoreType::U32, false, false), // Not marked unique here
        SPACETIMEDB_FIELD("data", SpacetimeDb::CoreType::I32, false, false)
    }
);


//
// "Option*" Series of Row Structs for Tables
//

// OptionI32
struct OptionI32Row { std::optional<int32_t> n; };
#define OPTION_I32_ROW_FIELDS(XX) XX(int32_t, n, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::OptionI32Row, "OptionI32Row", OPTION_I32_ROW_FIELDS,
    { SPACETIMEDB_FIELD_OPTIONAL("n", SpacetimeDb::CoreType::I32, false, false) }
);

// OptionString
struct OptionStringRow { std::optional<std::string> n; };
#define OPTION_STRING_ROW_FIELDS(XX) XX(std::string, n, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::OptionStringRow, "OptionStringRow", OPTION_STRING_ROW_FIELDS,
    { SPACETIMEDB_FIELD_OPTIONAL("n", SpacetimeDb::CoreType::String, false, false) }
);

// OptionIdentity
struct OptionIdentityRow { std::optional<spacetimedb::sdk::Identity> n; };
#define OPTION_IDENTITY_ROW_FIELDS(XX) XX(spacetimedb::sdk::Identity, n, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::OptionIdentityRow, "OptionIdentityRow", OPTION_IDENTITY_ROW_FIELDS,
    { SPACETIMEDB_FIELD_OPTIONAL_CUSTOM("n", "Identity", false, false) }
);

// OptionSimpleEnum
struct OptionSimpleEnumRow { std::optional<SimpleEnum> n; };
#define OPTION_SIMPLE_ENUM_ROW_FIELDS(XX) XX(sdk_test_cpp::SimpleEnum, n, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::OptionSimpleEnumRow, "OptionSimpleEnumRow", OPTION_SIMPLE_ENUM_ROW_FIELDS,
    { SPACETIMEDB_FIELD_OPTIONAL_CUSTOM("n", "SimpleEnum", false, false) }
);

// OptionEveryPrimitiveStruct
struct OptionEveryPrimitiveStructRow { std::optional<EveryPrimitiveStruct> s; };
#define OPTION_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS(XX) XX(sdk_test_cpp::EveryPrimitiveStruct, s, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS( sdk_test_cpp::OptionEveryPrimitiveStructRow, "OptionEveryPrimitiveStructRow", OPTION_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS,
    { SPACETIMEDB_FIELD_OPTIONAL_CUSTOM("s", "EveryPrimitiveStruct", false, false) }
);

// Intermediate type for OptionVecOptionI32Row
struct VecOptionI32 {
    std::vector<std::optional<int32_t>> value;
};
// X-Macro: element_type, field_name, is_element_optional_for_struct_field, is_vector_of_elements
// For std::vector<std::optional<T>>, the element type for BSATN is std::optional<T>.
// The X-Macro's IS_OPTIONAL flag is for the field itself, not the vector's elements.
// The schema registration needs to refer to the type of the element.
// Let's assume our Writer/Reader can handle std::vector<std::optional<T>> directly.
// The X-Macro for VecOptionI32:
// CPP_TYPE_OR_ELEMENT_TYPE is std::optional<int32_t>
// FIELD_NAME is value
// IS_OPTIONAL is false (the field `value` itself is not optional)
// IS_VECTOR is true
#define VEC_OPTION_I32_FIELDS(XX) XX(std::optional<int32_t>, value, false, true)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::VecOptionI32, "VecOptionI32", VEC_OPTION_I32_FIELDS,
    // Schema for the element type of the vector.
    // This assumes that the schema system can infer that "value" is a vector of optional I32s.
    // A more explicit schema might be needed if FieldDefinition supported optionality of vector elements.
    // For now, we register the element type as optional I32.
    { SPACETIMEDB_FIELD_OPTIONAL("value_element", SpacetimeDb::CoreType::I32, false, false) }
    // The field name in schema "value_element" is a placeholder, as the actual field is "value".
    // This highlights a current limitation in representing schemas for vectors of complex/optional types.
    // A better approach would be to register "std::optional<int32_t>" as a type, then use that.
    // Or, if the schema system implies element type from C++ type in X-Macro:
    // { SPACETIMEDB_FIELD_OPTIONAL("value", SpacetimeDb::CoreType::I32, false, false) } -> This seems more plausible.
);


// OptionVecOptionI32Row
struct OptionVecOptionI32Row {
    std::optional<VecOptionI32> v;
};
#define OPTION_VEC_OPTION_I32_ROW_FIELDS(XX) XX(sdk_test_cpp::VecOptionI32, v, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OptionVecOptionI32Row, "OptionVecOptionI32Row", OPTION_VEC_OPTION_I32_ROW_FIELDS,
    { SPACETIMEDB_FIELD_OPTIONAL_CUSTOM("v", "VecOptionI32", false, false) }
);


} // namespace sdk_test_cpp
#endif // SDK_TEST_H
