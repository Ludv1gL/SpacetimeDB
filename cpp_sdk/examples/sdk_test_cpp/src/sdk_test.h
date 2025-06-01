#ifndef SDK_TEST_H
#define SDK_TEST_H

#include <spacetimedb/macros.h>
#include <spacetimedb/sdk/spacetimedb_sdk_types.h> // For Identity, Timestamp (though not used yet)
#include <string>
#include <vector>
#include <variant> // For EnumWithPayload later
#include <cstdint> // For uint8_t etc.

// Test types outside namespace
enum class GlobalSimpleEnum : uint8_t {
    GlobalZero,
    GlobalOne,
    GlobalTwo,
};

SPACETIMEDB_TYPE_ENUM(
    GlobalSimpleEnum, GlobalSimpleEnum, // Sanitized name is the same for global types
    "GlobalSimpleEnum",
    ({
        SPACETIMEDB_ENUM_VARIANT("GlobalZero"),
        SPACETIMEDB_ENUM_VARIANT("GlobalOne"),
        SPACETIMEDB_ENUM_VARIANT("GlobalTwo")
        })
);

struct GlobalUnitStruct {};
#define GLOBAL_UNIT_STRUCT_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) /* No fields */
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    GlobalUnitStruct, GlobalUnitStruct, // Sanitized name is the same for global types
    "GlobalUnitStruct",
    GLOBAL_UNIT_STRUCT_FIELDS,
    ({ /* No fields */ })
);


namespace sdk_test_cpp {

    enum class SimpleEnum : uint8_t {
        Zero,
        One,
        Two,
    };

    // Schema registration for SimpleEnum
    SPACETIMEDB_TYPE_ENUM(
        sdk_test_cpp::SimpleEnum, sdk_test_cpp_SimpleEnum,
        "SimpleEnum",
        ({
            SPACETIMEDB_ENUM_VARIANT("Zero"),
            SPACETIMEDB_ENUM_VARIANT("One"),
            SPACETIMEDB_ENUM_VARIANT("Two")
            })
    );

    struct UnitStruct {
        // bool operator==(const UnitStruct&) const { return true; } // Optional
    };

#define UNIT_STRUCT_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) /* No fields, consumes args */
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::UnitStruct, sdk_test_cpp_UnitStruct,
        "UnitStruct",
        UNIT_STRUCT_FIELDS,
        ({ /* No fields */ })
    );

    struct ByteStruct {
        uint8_t b;
        // bool operator==(const ByteStruct& other) const { return b == other.b; } // Optional
    };

#define BYTE_STRUCT_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, b, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::ByteStruct, sdk_test_cpp_ByteStruct,
        "ByteStruct",
        BYTE_STRUCT_FIELDS,
        ({
            SPACETIMEDB_FIELD("b", ::SpacetimeDb::CoreType::U8, false, false)
            })
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
        ::SpacetimeDB::Types::uint128_t_placeholder e; // Corrected type
        ::spacetimedb::sdk::u256_placeholder f; // Corrected type
        int8_t g;
        int16_t h;
        int32_t i;
        int64_t j;
        ::SpacetimeDB::Types::int128_t_placeholder k; // Corrected type
        ::spacetimedb::sdk::i256_placeholder l; // Corrected type
        bool m;
        float n;
        double o;
        std::string p;
        spacetimedb::sdk::Identity q;
        spacetimedb::sdk::ConnectionId r; // Assuming this is a type from spacetimedb_sdk_types.h
        spacetimedb::sdk::Timestamp s;
        spacetimedb::sdk::TimeDuration t; // Assuming this is a type from spacetimedb_sdk_types.h
    };

#define EVERY_PRIMITIVE_STRUCT_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, a, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint16_t, b, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, c, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, d, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::uint128_t_placeholder, e, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::u256_placeholder, f, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int8_t, g, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int16_t, h, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, i, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int64_t, j, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::int128_t_placeholder, k, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::i256_placeholder, l, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, bool, m, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, float, n, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, double, o, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, p, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, q, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::ConnectionId, r, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Timestamp, s, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::TimeDuration, t, false, false)

    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::EveryPrimitiveStruct, sdk_test_cpp_EveryPrimitiveStruct,
        "EveryPrimitiveStruct",
        EVERY_PRIMITIVE_STRUCT_FIELDS,
        ({
            SPACETIMEDB_FIELD("a", ::SpacetimeDB::CoreType::U8, false, false),
            SPACETIMEDB_FIELD("b", ::SpacetimeDB::CoreType::U16, false, false),
            SPACETIMEDB_FIELD("c", ::SpacetimeDB::CoreType::U32, false, false),
            SPACETIMEDB_FIELD("d", ::SpacetimeDB::CoreType::U64, false, false),
            SPACETIMEDB_FIELD("e", ::SpacetimeDB::CoreType::U128, false, false),
            SPACETIMEDB_FIELD("f", ::SpacetimeDB::CoreType::U256, false, false),
            SPACETIMEDB_FIELD("g", ::SpacetimeDB::CoreType::I8, false, false),
            SPACETIMEDB_FIELD("h", ::SpacetimeDB::CoreType::I16, false, false),
            SPACETIMEDB_FIELD("i", ::SpacetimeDB::CoreType::I32, false, false),
            SPACETIMEDB_FIELD("j", ::SpacetimeDB::CoreType::I64, false, false),
            SPACETIMEDB_FIELD("k", ::SpacetimeDB::CoreType::I128, false, false),
            SPACETIMEDB_FIELD("l", ::SpacetimeDB::CoreType::I256, false, false),
            SPACETIMEDB_FIELD("m", ::SpacetimeDB::CoreType::Bool, false, false),
            SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::F32, false, false),
            SPACETIMEDB_FIELD("o", ::SpacetimeDB::CoreType::F64, false, false),
            SPACETIMEDB_FIELD("p", ::SpacetimeDB::CoreType::String, false, false),
            SPACETIMEDB_FIELD_CUSTOM("q", "Identity", false, false),
            SPACETIMEDB_FIELD_CUSTOM("r", "ConnectionId", false, false), // Assuming ConnectionId is a registered struct
            SPACETIMEDB_FIELD_CUSTOM("s", "Timestamp", false, false),
            SPACETIMEDB_FIELD_CUSTOM("t", "TimeDuration", false, false)
            })
    );

    struct EveryVecStruct {
        std::vector<uint8_t> a;
        std::vector<uint16_t> b;
        std::vector<uint32_t> c;
        std::vector<uint64_t> d;
        std::vector<::SpacetimeDB::Types::uint128_t_placeholder> e;
        std::vector<::spacetimedb::sdk::u256_placeholder> f;
        std::vector<int8_t> g;
        std::vector<int16_t> h;
        std::vector<int32_t> i;
        std::vector<int64_t> j;
        std::vector<::SpacetimeDB::Types::int128_t_placeholder> k;
        std::vector<::spacetimedb::sdk::i256_placeholder> l;
        std::vector<bool> m;
        std::vector<float> n;
        std::vector<double> o;
        std::vector<std::string> p;
        std::vector<spacetimedb::sdk::Identity> q;
        std::vector<spacetimedb::sdk::ConnectionId> r;
        std::vector<spacetimedb::sdk::Timestamp> s;
        std::vector<spacetimedb::sdk::TimeDuration> t;
    };

#define EVERY_VEC_STRUCT_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, a, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint16_t, b, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, c, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, d, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::uint128_t_placeholder, e, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::u256_placeholder, f, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int8_t, g, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int16_t, h, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, i, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int64_t, j, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::int128_t_placeholder, k, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::i256_placeholder, l, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, bool, m, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, float, n, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, double, o, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, p, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, q, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::ConnectionId, r, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Timestamp, s, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::TimeDuration, t, false, true)

    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::EveryVecStruct, sdk_test_cpp_EveryVecStruct,
        "EveryVecStruct",
        EVERY_VEC_STRUCT_FIELDS,
        ({ // Schema describes element types. Vector nature is implicit in C++ type + X-Macro IS_VECTOR flag.
            SPACETIMEDB_FIELD("a", ::SpacetimeDB::CoreType::U8, false, false),
            SPACETIMEDB_FIELD("b", ::SpacetimeDB::CoreType::U16, false, false),
            SPACETIMEDB_FIELD("c", ::SpacetimeDB::CoreType::U32, false, false),
            SPACETIMEDB_FIELD("d", ::SpacetimeDB::CoreType::U64, false, false),
            SPACETIMEDB_FIELD("e", ::SpacetimeDB::CoreType::U128, false, false),
            SPACETIMEDB_FIELD("f", ::SpacetimeDB::CoreType::U256, false, false),
            SPACETIMEDB_FIELD("g", ::SpacetimeDB::CoreType::I8, false, false),
            SPACETIMEDB_FIELD("h", ::SpacetimeDB::CoreType::I16, false, false),
            SPACETIMEDB_FIELD("i", ::SpacetimeDB::CoreType::I32, false, false),
            SPACETIMEDB_FIELD("j", ::SpacetimeDB::CoreType::I64, false, false),
            SPACETIMEDB_FIELD("k", ::SpacetimeDB::CoreType::I128, false, false),
            SPACETIMEDB_FIELD("l", ::SpacetimeDB::CoreType::I256, false, false),
            SPACETIMEDB_FIELD("m", ::SpacetimeDB::CoreType::Bool, false, false),
            SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::F32, false, false),
            SPACETIMEDB_FIELD("o", ::SpacetimeDB::CoreType::F64, false, false),
            SPACETIMEDB_FIELD("p", ::SpacetimeDB::CoreType::String, false, false),
            SPACETIMEDB_FIELD_CUSTOM("q", "Identity", false, false),
            SPACETIMEDB_FIELD_CUSTOM("r", "ConnectionId", false, false),
            SPACETIMEDB_FIELD_CUSTOM("s", "Timestamp", false, false),
            SPACETIMEDB_FIELD_CUSTOM("t", "TimeDuration", false, false)
            })
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
    inline void serialize(::bsatn::Writer& writer, const EnumWithPayloadTag& value) {
        writer.write_u8(static_cast<uint8_t>(value));
    }
    inline EnumWithPayloadTag deserialize_EnumWithPayloadTag(::bsatn::Reader& reader) {
        return static_cast<EnumWithPayloadTag>(reader.read_u8());
    }
} // namespace sdk_test_cpp

// Specialization for bsatn::deserialize must be in bsatn namespace
namespace SpacetimeDB::bsatn {
    template<>
    inline sdk_test_cpp::EnumWithPayloadTag deserialize<sdk_test_cpp::EnumWithPayloadTag>(::SpacetimeDB::bsatn::Reader& reader) {
        return sdk_test_cpp::deserialize_EnumWithPayloadTag(reader);
    }
} // namespace SpacetimeDB::bsatn

namespace sdk_test_cpp {
    struct EnumWithPayload {
        sdk_test_cpp::EnumWithPayloadTag tag;
        std::variant<
            uint8_t, uint16_t, uint32_t, uint64_t, ::SpacetimeDB::Types::uint128_t_placeholder, ::spacetimedb::sdk::u256_placeholder,
            int8_t, int16_t, int32_t, int64_t, ::SpacetimeDB::Types::int128_t_placeholder, ::spacetimedb::sdk::i256_placeholder,
            bool, float, double, std::string, spacetimedb::sdk::Identity, spacetimedb::sdk::ConnectionId,
            spacetimedb::sdk::Timestamp, std::vector<uint8_t>, std::vector<int32_t>,
            std::vector<std::string>, std::vector<SimpleEnum>
        > value;

        // Manual BSATN methods (declarations)
        void bsatn_serialize(::bsatn::Writer& writer) const;
        void bsatn_deserialize(::bsatn::Reader& reader);
    };


    // Schema registration for EnumWithPayloadTag (if not already done by being part of another module)
    // For this example, let's assume it needs explicit registration if it's a standalone type used in schema.
    // However, SPACETIMEDB_TYPE_ENUM requires an enum class, not a free function.
    // So, EnumWithPayloadTag will be registered via SPACETIMEDB_FIELD_CUSTOM("tag", "EnumWithPayloadTag", ...)
    // and we assume "EnumWithPayloadTag" corresponds to a schema type that is simply a U8.
    // A full registration for EnumWithPayloadTag like SimpleEnum isn't standard for a tag enum if its only purpose is to be a field.
    // But if it WERE to be registered like SimpleEnum:
    SPACETIMEDB_TYPE_ENUM(sdk_test_cpp::EnumWithPayloadTag, sdk_test_cpp_EnumWithPayloadTag, "EnumWithPayloadTag", ({
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
        }));


#define ENUM_WITH_PAYLOAD_FIELDS(XX) \
    XX(sdk_test_cpp::EnumWithPayloadTag, tag, false, false); \
    XX(std::vector<uint8_t>, payload_bytes_for_schema, false, false) // Placeholder for schema

    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::EnumWithPayload, sdk_test_cpp_EnumWithPayload,
        "EnumWithPayload",
        ENUM_WITH_PAYLOAD_FIELDS,
        ({
            SPACETIMEDB_FIELD_CUSTOM("tag", "EnumWithPayloadTag", false, false),
            SPACETIMEDB_FIELD("payload_bytes_for_schema", ::SpacetimeDB::CoreType::Bytes, false, false)
            })
    );

    //
    // 'One*' Series of Row Structs for Tables
    //

// OneU8
    struct OneU8Row {
        uint8_t n;
    };
#define ONE_U8_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneU8Row, sdk_test_cpp_OneU8Row, "OneU8Row", ONE_U8_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::U8, false, false) })
    );

    // OneU16
    struct OneU16Row {
        uint16_t n;
    };
#define ONE_U16_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint16_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneU16Row, sdk_test_cpp_OneU16Row, "OneU16Row", ONE_U16_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::U16, false, false) })
    );

    // OneU32
    struct OneU32Row {
        uint32_t n;
    };
#define ONE_U32_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneU32Row, sdk_test_cpp_OneU32Row, "OneU32Row", ONE_U32_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::U32, false, false) })
    );

    // OneU64
    struct OneU64Row {
        uint64_t n;
    };
#define ONE_U64_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneU64Row, sdk_test_cpp_OneU64Row, "OneU64Row", ONE_U64_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::U64, false, false) })
    );

    // OneU128
    struct OneU128Row {
        ::SpacetimeDB::Types::uint128_t_placeholder n;
    };
#define ONE_U128_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::uint128_t_placeholder, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneU128Row, sdk_test_cpp_OneU128Row, "OneU128Row", ONE_U128_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::U128, false, false) })
    );

    // OneU256
    struct OneU256Row {
        ::spacetimedb::sdk::u256_placeholder n;
    };
#define ONE_U256_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::u256_placeholder, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneU256Row, sdk_test_cpp_OneU256Row, "OneU256Row", ONE_U256_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::U256, false, false) })
    );

    // OneI8
    struct OneI8Row {
        int8_t n;
    };
#define ONE_I8_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int8_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneI8Row, sdk_test_cpp_OneI8Row, "OneI8Row", ONE_I8_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::I8, false, false) })
    );

    // OneI16
    struct OneI16Row {
        int16_t n;
    };
#define ONE_I16_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int16_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneI16Row, sdk_test_cpp_OneI16Row, "OneI16Row", ONE_I16_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::I16, false, false) })
    );

    // OneI32
    struct OneI32Row {
        int32_t n;
    };
#define ONE_I32_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneI32Row, sdk_test_cpp_OneI32Row, "OneI32Row", ONE_I32_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::I32, false, false) })
    );

    // OneI64
    struct OneI64Row {
        int64_t n;
    };
#define ONE_I64_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int64_t, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneI64Row, sdk_test_cpp_OneI64Row, "OneI64Row", ONE_I64_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::I64, false, false) })
    );

    // OneI128
    struct OneI128Row {
        ::SpacetimeDB::Types::int128_t_placeholder n;
    };
#define ONE_I128_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::int128_t_placeholder, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneI128Row, sdk_test_cpp_OneI128Row, "OneI128Row", ONE_I128_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::I128, false, false) })
    );

    // OneI256
    struct OneI256Row {
        ::spacetimedb::sdk::i256_placeholder n;
    };
#define ONE_I256_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::i256_placeholder, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneI256Row, sdk_test_cpp_OneI256Row, "OneI256Row", ONE_I256_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::I256, false, false) })
    );

    // OneBool
    struct OneBoolRow {
        bool n;
    };
#define ONE_BOOL_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, bool, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneBoolRow, sdk_test_cpp_OneBoolRow, "OneBoolRow", ONE_BOOL_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::Bool, false, false) })
    );

    // OneF32
    struct OneF32Row {
        float n;
    };
#define ONE_F32_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, float, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneF32Row, sdk_test_cpp_OneF32Row, "OneF32Row", ONE_F32_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::F32, false, false) })
    );

    // OneF64
    struct OneF64Row {
        double n;
    };
#define ONE_F64_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, double, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneF64Row, sdk_test_cpp_OneF64Row, "OneF64Row", ONE_F64_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::F64, false, false) })
    );

    // OneString
    struct OneStringRow {
        std::string n;
    };
#define ONE_STRING_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneStringRow, sdk_test_cpp_OneStringRow, "OneStringRow", ONE_STRING_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", ::SpacetimeDB::CoreType::String, false, false) })
    );

    // OneIdentity
    struct OneIdentityRow {
        spacetimedb::sdk::Identity n;
    };
#define ONE_IDENTITY_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneIdentityRow, sdk_test_cpp_OneIdentityRow, "OneIdentityRow", ONE_IDENTITY_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("n", "Identity", false, false) })
    );

    // OneConnectionId
    struct OneConnectionIdRow {
        spacetimedb::sdk::ConnectionId n;
    };
#define ONE_CONNECTION_ID_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::ConnectionId, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneConnectionIdRow, sdk_test_cpp_OneConnectionIdRow, "OneConnectionIdRow", ONE_CONNECTION_ID_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("n", "ConnectionId", false, false) }) // Assuming ConnectionId is registered type
    );

    // OneTimestamp
    struct OneTimestampRow {
        spacetimedb::sdk::Timestamp n;
    };
#define ONE_TIMESTAMP_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Timestamp, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneTimestampRow, sdk_test_cpp_OneTimestampRow, "OneTimestampRow", ONE_TIMESTAMP_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("n", "Timestamp", false, false) })
    );

    // OneSimpleEnum
    struct OneSimpleEnumRow {
        SimpleEnum n;
    };
#define ONE_SIMPLE_ENUM_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::SimpleEnum, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneSimpleEnumRow, sdk_test_cpp_OneSimpleEnumRow, "OneSimpleEnumRow", ONE_SIMPLE_ENUM_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("n", "SimpleEnum", false, false) })
    );

    // OneEnumWithPayload
    struct OneEnumWithPayloadRow {
        EnumWithPayload n;
    };
#define ONE_ENUM_WITH_PAYLOAD_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EnumWithPayload, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneEnumWithPayloadRow, sdk_test_cpp_OneEnumWithPayloadRow, "OneEnumWithPayloadRow", ONE_ENUM_WITH_PAYLOAD_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("n", "EnumWithPayload", false, false) })
    );

    // OneUnitStruct
    struct OneUnitStructRow {
        UnitStruct s; // Field name 's' as in Rust example (though it's a unit struct)
    };
#define ONE_UNIT_STRUCT_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::UnitStruct, s, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneUnitStructRow, sdk_test_cpp_OneUnitStructRow, "OneUnitStructRow", ONE_UNIT_STRUCT_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("s", "UnitStruct", false, false) })
    );

    // OneByteStruct
    struct OneByteStructRow {
        ByteStruct s;
    };
#define ONE_BYTE_STRUCT_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::ByteStruct, s, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneByteStructRow, sdk_test_cpp_OneByteStructRow, "OneByteStructRow", ONE_BYTE_STRUCT_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("s", "ByteStruct", false, false) })
    );

    // OneEveryPrimitiveStruct
    struct OneEveryPrimitiveStructRow {
        EveryPrimitiveStruct s;
    };
#define ONE_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryPrimitiveStruct, s, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneEveryPrimitiveStructRow, sdk_test_cpp_OneEveryPrimitiveStructRow, "OneEveryPrimitiveStructRow", ONE_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("s", "EveryPrimitiveStruct", false, false) })
    );

    // OneEveryVecStruct
    struct OneEveryVecStructRow {
        EveryVecStruct s;
    };
#define ONE_EVERY_VEC_STRUCT_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryVecStruct, s, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::OneEveryVecStructRow, sdk_test_cpp_OneEveryVecStructRow, "OneEveryVecStructRow", ONE_EVERY_VEC_STRUCT_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("s", "EveryVecStruct", false, false) })
    );

    //
    // "Vec*" Series of Row Structs for Tables
    //


// VecU8
    struct VecU8Row { std::vector<uint8_t> n; };
#define VEC_U8_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, n, false, true)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::VecU8Row, sdk_test_cpp_VecU8Row, "VecU8Row", VEC_U8_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", SpacetimeDB::CoreType::U8, false, false) }) // Schema for element type
    );

    // VecString
    struct VecStringRow { std::vector<std::string> n; };
#define VEC_STRING_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, n, false, true)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::VecStringRow, sdk_test_cpp_VecStringRow, "VecStringRow", VEC_STRING_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD("n", SpacetimeDB::CoreType::String, false, false) })
    );

    // VecSimpleEnum
    struct VecSimpleEnumRow { std::vector<SimpleEnum> n; };
#define VEC_SIMPLE_ENUM_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::SimpleEnum, n, false, true)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::VecSimpleEnumRow, sdk_test_cpp_VecSimpleEnumRow, "VecSimpleEnumRow", VEC_SIMPLE_ENUM_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("n", "SimpleEnum", false, false) })
    );

    // VecEveryPrimitiveStruct
    struct VecEveryPrimitiveStructRow { std::vector<EveryPrimitiveStruct> s; };
#define VEC_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryPrimitiveStruct, s, false, true)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::VecEveryPrimitiveStructRow, sdk_test_cpp_VecEveryPrimitiveStructRow, "VecEveryPrimitiveStructRow", VEC_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM("s", "EveryPrimitiveStruct", false, false) })
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
#define UNIQUE_U8_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, n, true, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::UniqueU8Row, sdk_test_cpp_UniqueU8Row, "UniqueU8Row", UNIQUE_U8_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD("n", SpacetimeDB::CoreType::U8, true, false),
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // UniqueStringRow
    struct UniqueStringRow {
        uint64_t row_id;
        std::string s;
        int32_t data;
    };
#define UNIQUE_STRING_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, s, true, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::UniqueStringRow, sdk_test_cpp_UniqueStringRow, "UniqueStringRow", UNIQUE_STRING_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD("s", SpacetimeDB::CoreType::String, true, false),
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // UniqueIdentityRow
    struct UniqueIdentityRow {
        uint64_t row_id;
        spacetimedb::sdk::Identity i;
        int32_t data;
    };
#define UNIQUE_IDENTITY_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, i, true, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::UniqueIdentityRow, sdk_test_cpp_UniqueIdentityRow, "UniqueIdentityRow", UNIQUE_IDENTITY_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD_CUSTOM("i", "Identity", true, false),
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // UniqueConnectionIdRow
    struct UniqueConnectionIdRow {
        uint64_t row_id;
        spacetimedb::sdk::ConnectionId c; // Assuming ConnectionId is a distinct type
        int32_t data;
    };
#define UNIQUE_CONNECTION_ID_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::ConnectionId, c, true, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::UniqueConnectionIdRow, sdk_test_cpp_UniqueConnectionIdRow, "UniqueConnectionIdRow", UNIQUE_CONNECTION_ID_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD_CUSTOM("c", "ConnectionId", true, false), // Assuming ConnectionId is registered as "ConnectionId"
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
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
        ::SpacetimeDB::Types::uint128_t_placeholder e;
        ::spacetimedb::sdk::u256_placeholder f;
        int8_t g;
        int16_t h;
        int32_t i;
        int64_t j;
        ::SpacetimeDB::Types::int128_t_placeholder k;
        ::spacetimedb::sdk::i256_placeholder l;
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

#define LARGE_TABLE_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, a, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint16_t, b, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, c, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, d, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::uint128_t_placeholder, e, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::u256_placeholder, f, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int8_t, g, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int16_t, h, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, i, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int64_t, j, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::SpacetimeDB::Types::int128_t_placeholder, k, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, ::spacetimedb::sdk::i256_placeholder, l, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, bool, m, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, float, n, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, double, o, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, p, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, q, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::ConnectionId, r, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Timestamp, s, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::TimeDuration, t, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, va, false, true); /* Vector of u8 */ \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, vp, false, true); /* Vector of string */ \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryPrimitiveStruct, vs, false, true); /* Vector of EveryPrimitiveStruct */ \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, oi, true, false); /* Optional i32 */ \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, os, true, false); /* Optional string */ \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryPrimitiveStruct, oes, true, false); /* Optional EveryPrimitiveStruct */ \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::SimpleEnum, en, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EnumWithPayload, ewp, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::UnitStruct, us, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::ByteStruct, bs, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryPrimitiveStruct, eps, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryVecStruct, evs, false, false)


    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::LargeTableRow, sdk_test_cpp_LargeTableRow, "LargeTableRow", LARGE_TABLE_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD("a", SpacetimeDB::CoreType::U8, false, false),
            SPACETIMEDB_FIELD("b", SpacetimeDB::CoreType::U16, false, false),
            SPACETIMEDB_FIELD("c", SpacetimeDB::CoreType::U32, false, false),
            SPACETIMEDB_FIELD("d", SpacetimeDB::CoreType::U64, false, false),
            SPACETIMEDB_FIELD("e", SpacetimeDB::CoreType::U128, false, false),
            SPACETIMEDB_FIELD("f", SpacetimeDB::CoreType::U256, false, false),
            SPACETIMEDB_FIELD("g", SpacetimeDB::CoreType::I8, false, false),
            SPACETIMEDB_FIELD("h", SpacetimeDB::CoreType::I16, false, false),
            SPACETIMEDB_FIELD("i", SpacetimeDB::CoreType::I32, false, false),
            SPACETIMEDB_FIELD("j", SpacetimeDB::CoreType::I64, false, false),
            SPACETIMEDB_FIELD("k", SpacetimeDB::CoreType::I128, false, false),
            SPACETIMEDB_FIELD("l", SpacetimeDB::CoreType::I256, false, false),
            SPACETIMEDB_FIELD("m", SpacetimeDB::CoreType::Bool, false, false),
            SPACETIMEDB_FIELD("n", SpacetimeDB::CoreType::F32, false, false),
            SPACETIMEDB_FIELD("o", SpacetimeDB::CoreType::F64, false, false),
            SPACETIMEDB_FIELD("p", SpacetimeDB::CoreType::String, false, false),
            SPACETIMEDB_FIELD_CUSTOM("q", "Identity", false, false),
            SPACETIMEDB_FIELD_CUSTOM("r", "ConnectionId", false, false),
            SPACETIMEDB_FIELD_CUSTOM("s", "Timestamp", false, false),
            SPACETIMEDB_FIELD_CUSTOM("t", "TimeDuration", false, false),
            SPACETIMEDB_FIELD("va", SpacetimeDB::CoreType::U8, false, false), // Schema for element type
            SPACETIMEDB_FIELD("vp", SpacetimeDB::CoreType::String, false, false), // Schema for element type
            SPACETIMEDB_FIELD_CUSTOM("vs", "EveryPrimitiveStruct", false, false), // Schema for element type
            SPACETIMEDB_FIELD_OPTIONAL("oi", SpacetimeDB::CoreType::I32, false, false),
            SPACETIMEDB_FIELD_OPTIONAL("os", SpacetimeDB::CoreType::String, false, false),
            SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("oes", "EveryPrimitiveStruct", false, false),
            SPACETIMEDB_FIELD_CUSTOM("en", "SimpleEnum", false, false),
            SPACETIMEDB_FIELD_CUSTOM("ewp", "EnumWithPayload", false, false),
            SPACETIMEDB_FIELD_CUSTOM("us", "UnitStruct", false, false),
            SPACETIMEDB_FIELD_CUSTOM("bs", "ByteStruct", false, false),
            SPACETIMEDB_FIELD_CUSTOM("eps", "EveryPrimitiveStruct", false, false),
            SPACETIMEDB_FIELD_CUSTOM("evs", "EveryVecStruct", false, false)
            })
    );

    // TableHoldsTableRow
    struct TableHoldsTableRow {
        uint64_t row_id; // PK
        OneU8Row a;
        VecU8Row b;
    };
#define TABLE_HOLDS_TABLE_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::OneU8Row, a, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::VecU8Row, b, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::TableHoldsTableRow, sdk_test_cpp_TableHoldsTableRow, "TableHoldsTableRow", TABLE_HOLDS_TABLE_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD_CUSTOM("a", "OneU8Row", false, false),
            SPACETIMEDB_FIELD_CUSTOM("b", "VecU8Row", false, false)
            })
    );

    // ScheduledTableRow
    struct ScheduledTableRow {
        uint64_t scheduled_id; // PK
        spacetimedb::sdk::ScheduleAt scheduled_at;
        std::string text;
    };
#define SCHEDULED_TABLE_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, scheduled_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::ScheduleAt, scheduled_at, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, text, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::ScheduledTableRow, sdk_test_cpp_ScheduledTableRow, "ScheduledTableRow", SCHEDULED_TABLE_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("scheduled_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD_CUSTOM("scheduled_at", "ScheduleAt", false, false), // Assuming ScheduleAt is registered
            SPACETIMEDB_FIELD("text", SpacetimeDB::CoreType::String, false, false)
            })
    );

    // IndexedTableRow
    struct IndexedTableRow {
        uint64_t row_id; // PK
        uint32_t player_id;
    };
#define INDEXED_TABLE_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, player_id, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::IndexedTableRow, sdk_test_cpp_IndexedTableRow, "IndexedTableRow", INDEXED_TABLE_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD("player_id", SpacetimeDB::CoreType::U32, false, false)
            })
    );

    // IndexedTable2Row
    struct IndexedTable2Row {
        uint64_t row_id; // PK
        uint32_t player_id;
        float player_snazz;
    };
#define INDEXED_TABLE_2_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, player_id, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, float, player_snazz, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::IndexedTable2Row, sdk_test_cpp_IndexedTable2Row, "IndexedTable2Row", INDEXED_TABLE_2_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD("player_id", SpacetimeDB::CoreType::U32, false, false),
            SPACETIMEDB_FIELD("player_snazz", SpacetimeDB::CoreType::F32, false, false)
            })
    );

    // BTreeU32Row
    struct BTreeU32Row {
        uint64_t row_id; // PK
        uint32_t n;
        int32_t data;
    };
#define BTREE_U32_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, n, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::BTreeU32Row, sdk_test_cpp_BTreeU32Row, "BTreeU32Row", BTREE_U32_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDB::CoreType::U64, false, true),
            SPACETIMEDB_FIELD("n", SpacetimeDB::CoreType::U32, false, false), // Not marked unique here, index will be separate
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // UsersRow
    struct UsersRow {
        spacetimedb::sdk::Identity identity; // PK
        std::string name;
    };
#define USERS_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, identity, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, name, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::UsersRow, sdk_test_cpp_UsersRow, "UsersRow", USERS_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD_CUSTOM("identity", "Identity", false, false),
            SPACETIMEDB_FIELD("name", SpacetimeDB::CoreType::String, false, false)
            })
    );

    // IndexedSimpleEnumRow
    struct IndexedSimpleEnumRow {
        uint64_t row_id; // PK
        SimpleEnum n;
    };
#define INDEXED_SIMPLE_ENUM_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint64_t, row_id, false, true); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::SimpleEnum, n, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::IndexedSimpleEnumRow, sdk_test_cpp_IndexedSimpleEnumRow, "IndexedSimpleEnumRow", INDEXED_SIMPLE_ENUM_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("row_id", SpacetimeDb::CoreType::U64, false, true),
            SPACETIMEDB_FIELD_CUSTOM("n", "SimpleEnum", false, false)
            })
    );


    //
    // "Pk*" Series of Row Structs for Tables
    //


// PkU8Row
    struct PkU8Row {
        uint8_t n;
        int32_t data;
    };
#define PK_U8_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint8_t, n, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::PkU8Row, sdk_test_cpp_PkU8Row, "PkU8Row", PK_U8_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("n", SpacetimeDB::CoreType::U8, false, false), // Uniqueness implied by PK usage
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // PkStringRow
    struct PkStringRow {
        std::string s;
        int32_t data;
    };
#define PK_STRING_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, s, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::PkStringRow, sdk_test_cpp_PkStringRow, "PkStringRow", PK_STRING_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("s", SpacetimeDB::CoreType::String, false, false),
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // PkIdentityRow
    struct PkIdentityRow {
        spacetimedb::sdk::Identity i;
        int32_t data;
    };
#define PK_IDENTITY_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, i, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::PkIdentityRow, sdk_test_cpp_PkIdentityRow, "PkIdentityRow", PK_IDENTITY_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD_CUSTOM("i", "Identity", false, false),
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // PkSimpleEnumRow
    struct PkSimpleEnumRow {
        SimpleEnum e;
        int32_t data;
    };
#define PK_SIMPLE_ENUM_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::SimpleEnum, e, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::PkSimpleEnumRow, sdk_test_cpp_PkSimpleEnumRow, "PkSimpleEnumRow", PK_SIMPLE_ENUM_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD_CUSTOM("e", "SimpleEnum", false, false),
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );

    // PkU32TwoRow (Example of a table with two primary key components, though our macro currently only supports one)
    // For now, we'll define it with 'a' as the PK as per simplified approach.
    // A proper composite PK would require changes to SPACETIMEDB_PRIMARY_KEY and ModuleSchema.
    struct PkU32TwoRow {
        uint32_t a;
        uint32_t b;
        int32_t data;
    };
#define PK_U32_TWO_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, a, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, b, false, false); \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, data, false, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::PkU32TwoRow, sdk_test_cpp_PkU32TwoRow, "PkU32TwoRow", PK_U32_TWO_ROW_FIELDS,
        ({
            SPACETIMEDB_FIELD("a", SpacetimeDB::CoreType::U32, false, false),
            SPACETIMEDB_FIELD("b", SpacetimeDB::CoreType::U32, false, false), // Not marked unique here
            SPACETIMEDB_FIELD("data", SpacetimeDB::CoreType::I32, false, false)
            })
    );


    //
    // "Option*" Series of Row Structs for Tables
    //


// OptionI32
    struct OptionI32Row { std::optional<int32_t> n; };
#define OPTION_I32_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, int32_t, n, true, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::OptionI32Row, sdk_test_cpp_OptionI32Row, "OptionI32Row", OPTION_I32_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_OPTIONAL("n", SpacetimeDB::CoreType::I32, false, false) })
    );

    // OptionString
    struct OptionStringRow { std::optional<std::string> n; };
#define OPTION_STRING_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, n, true, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::OptionStringRow, sdk_test_cpp_OptionStringRow, "OptionStringRow", OPTION_STRING_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_OPTIONAL("n", SpacetimeDB::CoreType::String, false, false) })
    );

    // OptionIdentity
    struct OptionIdentityRow { std::optional<spacetimedb::sdk::Identity> n; };
#define OPTION_IDENTITY_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, spacetimedb::sdk::Identity, n, true, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::OptionIdentityRow, sdk_test_cpp_OptionIdentityRow, "OptionIdentityRow", OPTION_IDENTITY_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("n", "Identity", false, false) }) // Custom types don't use ::SpacetimeDB::CoreType directly in schema
    );

    // OptionSimpleEnum
    struct OptionSimpleEnumRow { std::optional<SimpleEnum> n; };
#define OPTION_SIMPLE_ENUM_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::SimpleEnum, n, true, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::OptionSimpleEnumRow, sdk_test_cpp_OptionSimpleEnumRow, "OptionSimpleEnumRow", OPTION_SIMPLE_ENUM_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("n", "SimpleEnum", false, false) }) // Custom types don't use ::SpacetimeDB::CoreType directly in schema
    );

    // OptionEveryPrimitiveStruct
    struct OptionEveryPrimitiveStructRow { std::optional<EveryPrimitiveStruct> s; };
#define OPTION_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::EveryPrimitiveStruct, s, true, false)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(sdk_test_cpp::OptionEveryPrimitiveStructRow, sdk_test_cpp_OptionEveryPrimitiveStructRow, "OptionEveryPrimitiveStructRow", OPTION_EVERY_PRIMITIVE_STRUCT_ROW_FIELDS,
        ({ SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("s", "EveryPrimitiveStruct", false, false) }) // Custom types don't use ::SpacetimeDB::CoreType directly in schema
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
#define VEC_OPTION_I32_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::optional<int32_t>, value, false, true)
    SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
        sdk_test_cpp::VecOptionI32, sdk_test_cpp_VecOptionI32, "VecOptionI32", VEC_OPTION_I32_FIELDS,
        // Schema for the element type of the vector.
        // This assumes that the schema system can infer that "value" is a vector of optional I32s.
        // A more explicit schema might be needed if FieldDefinition supported optionality of vector elements.
        // For now, we register the element type as optional I32.
        ({ SPACETIMEDB_FIELD_OPTIONAL("value_element", ::SpacetimeDB::CoreType::I32, false, false) })
        // The field name in schema "value_element" is a placeholder, as the actual field is "value".
        // This highlights a current limitation in representing schemas for vectors of complex/optional types.
        // A better approach would be to register "std::optional<int32_t>" as a type, then use that.
        // Or, if the schema system implies element type from C++ type in X-Macro:
        // { SPACETIMEDB_FIELD_OPTIONAL("value", SpacetimeDB::CoreType::I32, false, false) } -> This seems more plausible.
})
);


// OptionVecOptionI32Row
struct OptionVecOptionI32Row {
    std::optional<VecOptionI32> v;
};
#define OPTION_VEC_OPTION_I32_ROW_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) ACTION(WRITER_OR_READER, VALUE_OR_OBJ, sdk_test_cpp::VecOptionI32, v, true, false)
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    sdk_test_cpp::OptionVecOptionI32Row, sdk_test_cpp_OptionVecOptionI32Row, "OptionVecOptionI32Row", OPTION_VEC_OPTION_I32_ROW_FIELDS,
    ({ SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("v", "VecOptionI32", false, false) }) // Custom types don't use ::SpacetimeDB::CoreType directly in schema
);


} // namespace sdk_test_cpp
#endif // SDK_TEST_H
