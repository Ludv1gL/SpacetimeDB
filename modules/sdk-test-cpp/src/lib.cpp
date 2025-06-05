#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/macros.h>
#include <spacetimedb/types.h>
#include <string>
#include <vector>
#include <optional>

using namespace SpacetimeDb;

// Define enums
enum class SimpleEnum : uint8_t {
    Zero = 0,
    One = 1,
    Two = 2
};

// Register SimpleEnum with the type system
// TODO: Fix type registration
// SPACETIMEDB_REGISTER_TYPE(SimpleEnum);

// Define EnumWithPayload as a tagged union
struct EnumWithPayload {
    enum class Tag : uint8_t {
        U8, U16, U32, U64, U128, U256,
        I8, I16, I32, I64, I128, I256,
        Bool, F32, F64, Str,
        Identity, ConnectionId, Timestamp,
        Bytes, Ints, Strings, SimpleEnums
    } tag;
    
    // TODO: Properly implement variant storage
    // For now, use simple members instead of union for types with constructors
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    U128 u128;
    U256 u256;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    I128 i128;
    I256 i256;
    bool bool_val;
    float f32;
    double f64;
    std::string str;
    SpacetimeDb::Identity identity;
    SpacetimeDb::ConnectionId connection_id;
    SpacetimeDb::Timestamp timestamp;
    std::vector<uint8_t> bytes;
    std::vector<int32_t> ints;
    std::vector<std::string> strings;
    std::vector<SimpleEnum> simple_enums;
    
    // Constructors for each variant
    static EnumWithPayload from_u8(uint8_t v) { EnumWithPayload e; e.tag = Tag::U8; e.u8 = v; return e; }
    static EnumWithPayload from_u16(uint16_t v) { EnumWithPayload e; e.tag = Tag::U16; e.u16 = v; return e; }
    static EnumWithPayload from_u32(uint32_t v) { EnumWithPayload e; e.tag = Tag::U32; e.u32 = v; return e; }
    static EnumWithPayload from_u64(uint64_t v) { EnumWithPayload e; e.tag = Tag::U64; e.u64 = v; return e; }
    static EnumWithPayload from_u128(const U128& v) { EnumWithPayload e; e.tag = Tag::U128; e.u128 = v; return e; }
    static EnumWithPayload from_u256(const U256& v) { EnumWithPayload e; e.tag = Tag::U256; e.u256 = v; return e; }
    static EnumWithPayload from_i8(int8_t v) { EnumWithPayload e; e.tag = Tag::I8; e.i8 = v; return e; }
    static EnumWithPayload from_i16(int16_t v) { EnumWithPayload e; e.tag = Tag::I16; e.i16 = v; return e; }
    static EnumWithPayload from_i32(int32_t v) { EnumWithPayload e; e.tag = Tag::I32; e.i32 = v; return e; }
    static EnumWithPayload from_i64(int64_t v) { EnumWithPayload e; e.tag = Tag::I64; e.i64 = v; return e; }
    static EnumWithPayload from_i128(const I128& v) { EnumWithPayload e; e.tag = Tag::I128; e.i128 = v; return e; }
    static EnumWithPayload from_i256(const I256& v) { EnumWithPayload e; e.tag = Tag::I256; e.i256 = v; return e; }
    static EnumWithPayload from_bool(bool v) { EnumWithPayload e; e.tag = Tag::Bool; e.bool_val = v; return e; }
    static EnumWithPayload from_f32(float v) { EnumWithPayload e; e.tag = Tag::F32; e.f32 = v; return e; }
    static EnumWithPayload from_f64(double v) { EnumWithPayload e; e.tag = Tag::F64; e.f64 = v; return e; }
    static EnumWithPayload from_str(const std::string& v) { EnumWithPayload e; e.tag = Tag::Str; e.str = v; return e; }
    static EnumWithPayload from_identity(const SpacetimeDb::Identity& v) { EnumWithPayload e; e.tag = Tag::Identity; e.identity = v; return e; }
    static EnumWithPayload from_connection_id(const SpacetimeDb::ConnectionId& v) { EnumWithPayload e; e.tag = Tag::ConnectionId; e.connection_id = v; return e; }
    static EnumWithPayload from_timestamp(const SpacetimeDb::Timestamp& v) { EnumWithPayload e; e.tag = Tag::Timestamp; e.timestamp = v; return e; }
    static EnumWithPayload from_bytes(const std::vector<uint8_t>& v) { EnumWithPayload e; e.tag = Tag::Bytes; e.bytes = v; return e; }
    static EnumWithPayload from_ints(const std::vector<int32_t>& v) { EnumWithPayload e; e.tag = Tag::Ints; e.ints = v; return e; }
    static EnumWithPayload from_strings(const std::vector<std::string>& v) { EnumWithPayload e; e.tag = Tag::Strings; e.strings = v; return e; }
    static EnumWithPayload from_simple_enums(const std::vector<SimpleEnum>& v) { EnumWithPayload e; e.tag = Tag::SimpleEnums; e.simple_enums = v; return e; }
};

// Register EnumWithPayload
// TODO: Fix type registration
// SPACETIMEDB_REGISTER_TYPE(EnumWithPayload);

// Define structs
struct UnitStruct {};
// TODO: Fix type registration
// SPACETIMEDB_REGISTER_TYPE(UnitStruct);

struct ByteStruct {
    uint8_t b;
};
// TODO: Fix type registration
// SPACETIMEDB_REGISTER_TYPE(ByteStruct, SPACETIMEDB_FIELD(ByteStruct, b));

struct EveryPrimitiveStruct {
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint64_t d;
    U128 e;
    U256 f;
    int8_t g;
    int16_t h;
    int32_t i;
    int64_t j;
    I128 k;
    I256 l;
    bool m;
    float n;
    double o;
    std::string p;
    SpacetimeDb::Identity q;
    SpacetimeDb::ConnectionId r;
    SpacetimeDb::Timestamp s;
    SpacetimeDb::TimeDuration t;
};
// TODO: Fix SPACETIMEDB_REGISTER_TYPE macro usage
// SPACETIMEDB_REGISTER_TYPE(EveryPrimitiveStruct);

struct EveryVecStruct {
    std::vector<uint8_t> a;
    std::vector<uint16_t> b;
    std::vector<uint32_t> c;
    std::vector<uint64_t> d;
    std::vector<U128> e;
    std::vector<U256> f;
    std::vector<int8_t> g;
    std::vector<int16_t> h;
    std::vector<int32_t> i;
    std::vector<int64_t> j;
    std::vector<I128> k;
    std::vector<I256> l;
    std::vector<bool> m;
    std::vector<float> n;
    std::vector<double> o;
    std::vector<std::string> p;
    std::vector<SpacetimeDb::Identity> q;
    std::vector<SpacetimeDb::ConnectionId> r;
    std::vector<SpacetimeDb::Timestamp> s;
    std::vector<SpacetimeDb::TimeDuration> t;
};
// TODO: Fix type registration
// SPACETIMEDB_REGISTER_TYPE(EveryVecStruct,
//     SPACETIMEDB_FIELD(EveryVecStruct, a),
//     SPACETIMEDB_FIELD(EveryVecStruct, b),
//     SPACETIMEDB_FIELD(EveryVecStruct, c),
//     SPACETIMEDB_FIELD(EveryVecStruct, d),
//     SPACETIMEDB_FIELD(EveryVecStruct, e),
//     SPACETIMEDB_FIELD(EveryVecStruct, f),
//     SPACETIMEDB_FIELD(EveryVecStruct, g),
//     SPACETIMEDB_FIELD(EveryVecStruct, h),
//     SPACETIMEDB_FIELD(EveryVecStruct, i),
//     SPACETIMEDB_FIELD(EveryVecStruct, j),
//     SPACETIMEDB_FIELD(EveryVecStruct, k),
//     SPACETIMEDB_FIELD(EveryVecStruct, l),
//     SPACETIMEDB_FIELD(EveryVecStruct, m),
//     SPACETIMEDB_FIELD(EveryVecStruct, n),
//     SPACETIMEDB_FIELD(EveryVecStruct, o),
//     SPACETIMEDB_FIELD(EveryVecStruct, p),
//     SPACETIMEDB_FIELD(EveryVecStruct, q),
//     SPACETIMEDB_FIELD(EveryVecStruct, r),
//     SPACETIMEDB_FIELD(EveryVecStruct, s),
//     SPACETIMEDB_FIELD(EveryVecStruct, t)
// );

// Define table structs for single values
struct OneU8 { uint8_t n; };
struct OneU16 { uint16_t n; };
struct OneU32 { uint32_t n; };
struct OneU64 { uint64_t n; };
struct OneU128 { U128 n; };
struct OneU256 { U256 n; };

// Register tables
SPACETIMEDB_TABLE(OneU8, "one_u8", true);
SPACETIMEDB_TABLE(OneU16, "one_u16", true);
SPACETIMEDB_TABLE(OneU32, "one_u32", true);
SPACETIMEDB_TABLE(OneU64, "one_u64", true);
SPACETIMEDB_TABLE(OneU128, "one_u128", true);
SPACETIMEDB_TABLE(OneU256, "one_u256", true);

struct OneI8 { int8_t n; };
struct OneI16 { int16_t n; };
struct OneI32 { int32_t n; };
struct OneI64 { int64_t n; };
struct OneI128 { I128 n; };
struct OneI256 { I256 n; };

SPACETIMEDB_TABLE(OneI8, "one_i8", true);
SPACETIMEDB_TABLE(OneI16, "one_i16", true);
SPACETIMEDB_TABLE(OneI32, "one_i32", true);
SPACETIMEDB_TABLE(OneI64, "one_i64", true);
SPACETIMEDB_TABLE(OneI128, "one_i128", true);
SPACETIMEDB_TABLE(OneI256, "one_i256", true);

struct OneBool { bool b; };
struct OneF32 { float f; };
struct OneF64 { double f; };
struct OneString { std::string s; };

SPACETIMEDB_TABLE(OneBool, "one_bool", true);
SPACETIMEDB_TABLE(OneF32, "one_f32", true);
SPACETIMEDB_TABLE(OneF64, "one_f64", true);
SPACETIMEDB_TABLE(OneString, "one_string", true);

struct OneIdentity { SpacetimeDb::Identity i; };
struct OneConnectionId { SpacetimeDb::ConnectionId a; };
struct OneTimestamp { SpacetimeDb::Timestamp t; };

SPACETIMEDB_TABLE(OneIdentity, "one_identity", true);
SPACETIMEDB_TABLE(OneConnectionId, "one_connection_id", true);
SPACETIMEDB_TABLE(OneTimestamp, "one_timestamp", true);

struct OneSimpleEnum { SimpleEnum e; };
struct OneEnumWithPayload { EnumWithPayload e; };

SPACETIMEDB_TABLE(OneSimpleEnum, "one_simple_enum", true);
SPACETIMEDB_TABLE(OneEnumWithPayload, "one_enum_with_payload", true);

struct OneUnitStruct { UnitStruct s; };
struct OneByteStruct { ByteStruct s; };
struct OneEveryPrimitiveStruct { EveryPrimitiveStruct s; };
struct OneEveryVecStruct { EveryVecStruct s; };

SPACETIMEDB_TABLE(OneUnitStruct, "one_unit_struct", true);
SPACETIMEDB_TABLE(OneByteStruct, "one_byte_struct", true);
SPACETIMEDB_TABLE(OneEveryPrimitiveStruct, "one_every_primitive_struct", true);
SPACETIMEDB_TABLE(OneEveryVecStruct, "one_every_vec_struct", true);

// Reducers for single value inserts
SPACETIMEDB_REDUCER(insert_one_u8, UserDefined, ctx, uint8_t n) {
    ctx.insert(OneU8{n});
}

SPACETIMEDB_REDUCER(insert_one_u16, UserDefined, ctx, uint16_t n) {
    ctx.insert(OneU16{n});
}

SPACETIMEDB_REDUCER(insert_one_u32, UserDefined, ctx, uint32_t n) {
    ctx.insert(OneU32{n});
}

SPACETIMEDB_REDUCER(insert_one_u64, UserDefined, ctx, uint64_t n) {
    ctx.insert(OneU64{n});
}

SPACETIMEDB_REDUCER(insert_one_u128, UserDefined, ctx, U128 n) {
    ctx.insert(OneU128{n});
}

SPACETIMEDB_REDUCER(insert_one_u256, UserDefined, ctx, U256 n) {
    ctx.insert(OneU256{n});
}

SPACETIMEDB_REDUCER(insert_one_i8, UserDefined, ctx, int8_t n) {
    ctx.insert(OneI8{n});
}

SPACETIMEDB_REDUCER(insert_one_i16, UserDefined, ctx, int16_t n) {
    ctx.insert(OneI16{n});
}

SPACETIMEDB_REDUCER(insert_one_i32, UserDefined, ctx, int32_t n) {
    ctx.insert(OneI32{n});
}

SPACETIMEDB_REDUCER(insert_one_i64, UserDefined, ctx, int64_t n) {
    ctx.insert(OneI64{n});
}

SPACETIMEDB_REDUCER(insert_one_i128, UserDefined, ctx, I128 n) {
    ctx.insert(OneI128{n});
}

SPACETIMEDB_REDUCER(insert_one_i256, UserDefined, ctx, I256 n) {
    ctx.insert(OneI256{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_bool, ctx, bool b) {
    ctx.insert(OneBool{b});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_f32, ctx, float f) {
    ctx.insert(OneF32{f});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_f64, ctx, double f) {
    ctx.insert(OneF64{f});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_string, ctx, std::string s) {
    ctx.insert(OneString{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_identity, ctx, SpacetimeDb::Identity i) {
    ctx.insert(OneIdentity{i});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_connection_id, ctx, SpacetimeDb::ConnectionId a) {
    ctx.insert(OneConnectionId{a});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_timestamp, ctx, SpacetimeDb::Timestamp t) {
    ctx.insert(OneTimestamp{t});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_simple_enum, ctx, SimpleEnum e) {
    ctx.insert(OneSimpleEnum{e});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_enum_with_payload, ctx, EnumWithPayload e) {
    ctx.insert(OneEnumWithPayload{e});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_unit_struct, ctx, UnitStruct s) {
    ctx.insert(OneUnitStruct{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_byte_struct, ctx, ByteStruct s) {
    ctx.insert(OneByteStruct{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_every_primitive_struct, ctx, EveryPrimitiveStruct s) {
    ctx.insert(OneEveryPrimitiveStruct{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_one_every_vec_struct, ctx, EveryVecStruct s) {
    ctx.insert(OneEveryVecStruct{s});
}

// Tables holding vectors of various types
struct VecU8 { std::vector<uint8_t> n; };
struct VecU16 { std::vector<uint16_t> n; };
struct VecU32 { std::vector<uint32_t> n; };
struct VecU64 { std::vector<uint64_t> n; };
struct VecU128 { std::vector<U128> n; };
struct VecU256 { std::vector<U256> n; };

SPACETIMEDB_TABLE(VecU8, "vec_u8", true);
SPACETIMEDB_TABLE(VecU16, "vec_u16", true);
SPACETIMEDB_TABLE(VecU32, "vec_u32", true);
SPACETIMEDB_TABLE(VecU64, "vec_u64", true);
SPACETIMEDB_TABLE(VecU128, "vec_u128", true);
SPACETIMEDB_TABLE(VecU256, "vec_u256", true);

struct VecI8 { std::vector<int8_t> n; };
struct VecI16 { std::vector<int16_t> n; };
struct VecI32 { std::vector<int32_t> n; };
struct VecI64 { std::vector<int64_t> n; };
struct VecI128 { std::vector<I128> n; };
struct VecI256 { std::vector<I256> n; };

SPACETIMEDB_TABLE(VecI8, "vec_i8", true);
SPACETIMEDB_TABLE(VecI16, "vec_i16", true);
SPACETIMEDB_TABLE(VecI32, "vec_i32", true);
SPACETIMEDB_TABLE(VecI64, "vec_i64", true);
SPACETIMEDB_TABLE(VecI128, "vec_i128", true);
SPACETIMEDB_TABLE(VecI256, "vec_i256", true);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecBool, vec_bool, public, std::vector<bool> b);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecF32, vec_f32, public, std::vector<float> f);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecF64, vec_f64, public, std::vector<double> f);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecString, vec_string, public, std::vector<std::string> s);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecIdentity, vec_identity, public, std::vector<SpacetimeDb::Identity> i);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecConnectionId, vec_connection_id, public, std::vector<SpacetimeDb::ConnectionId> a);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecTimestamp, vec_timestamp, public, std::vector<SpacetimeDb::Timestamp> t);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecSimpleEnum, vec_simple_enum, public, std::vector<SimpleEnum> e);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecEnumWithPayload, vec_enum_with_payload, public, std::vector<EnumWithPayload> e);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecUnitStruct, vec_unit_struct, public, std::vector<UnitStruct> s);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecByteStruct, vec_byte_struct, public, std::vector<ByteStruct> s);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecEveryPrimitiveStruct, vec_every_primitive_struct, public, std::vector<EveryPrimitiveStruct> s);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(VecEveryVecStruct, vec_every_vec_struct, public, std::vector<EveryVecStruct> s);

// Reducers for vector inserts
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_u8, ctx, std::vector<uint8_t> n) {
    ctx.insert(VecU8{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_u16, ctx, std::vector<uint16_t> n) {
    ctx.insert(VecU16{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_u32, ctx, std::vector<uint32_t> n) {
    ctx.insert(VecU32{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_u64, ctx, std::vector<uint64_t> n) {
    ctx.insert(VecU64{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_u128, ctx, std::vector<U128> n) {
    ctx.insert(VecU128{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_u256, ctx, std::vector<U256> n) {
    ctx.insert(VecU256{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_i8, ctx, std::vector<int8_t> n) {
    ctx.insert(VecI8{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_i16, ctx, std::vector<int16_t> n) {
    ctx.insert(VecI16{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_i32, ctx, std::vector<int32_t> n) {
    ctx.insert(VecI32{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_i64, ctx, std::vector<int64_t> n) {
    ctx.insert(VecI64{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_i128, ctx, std::vector<I128> n) {
    ctx.insert(VecI128{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_i256, ctx, std::vector<I256> n) {
    ctx.insert(VecI256{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_bool, ctx, std::vector<bool> b) {
    ctx.insert(VecBool{b});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_f32, ctx, std::vector<float> f) {
    ctx.insert(VecF32{f});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_f64, ctx, std::vector<double> f) {
    ctx.insert(VecF64{f});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_string, ctx, std::vector<std::string> s) {
    ctx.insert(VecString{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_identity, ctx, std::vector<SpacetimeDb::Identity> i) {
    ctx.insert(VecIdentity{i});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_connection_id, ctx, std::vector<SpacetimeDb::ConnectionId> a) {
    ctx.insert(VecConnectionId{a});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_timestamp, ctx, std::vector<SpacetimeDb::Timestamp> t) {
    ctx.insert(VecTimestamp{t});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_simple_enum, ctx, std::vector<SimpleEnum> e) {
    ctx.insert(VecSimpleEnum{e});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_enum_with_payload, ctx, std::vector<EnumWithPayload> e) {
    ctx.insert(VecEnumWithPayload{e});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_unit_struct, ctx, std::vector<UnitStruct> s) {
    ctx.insert(VecUnitStruct{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_byte_struct, ctx, std::vector<ByteStruct> s) {
    ctx.insert(VecByteStruct{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_every_primitive_struct, ctx, std::vector<EveryPrimitiveStruct> s) {
    ctx.insert(VecEveryPrimitiveStruct{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_vec_every_vec_struct, ctx, std::vector<EveryVecStruct> s) {
    ctx.insert(VecEveryVecStruct{s});
}

// Tables holding optional values
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(OptionI32, option_i32, public, std::optional<int32_t> n);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(OptionString, option_string, public, std::optional<std::string> s);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(OptionIdentity, option_identity, public, std::optional<SpacetimeDb::Identity> i);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(OptionSimpleEnum, option_simple_enum, public, std::optional<SimpleEnum> e);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(OptionEveryPrimitiveStruct, option_every_primitive_struct, public, std::optional<EveryPrimitiveStruct> s);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(OptionVecOptionI32, option_vec_option_i32, public, std::optional<std::vector<std::optional<int32_t>>> v);

// Reducers for optional inserts
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_option_i32, ctx, std::optional<int32_t> n) {
    ctx.insert(OptionI32{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_option_string, ctx, std::optional<std::string> s) {
    ctx.insert(OptionString{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_option_identity, ctx, std::optional<SpacetimeDb::Identity> i) {
    ctx.insert(OptionIdentity{i});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_option_simple_enum, ctx, std::optional<SimpleEnum> e) {
    ctx.insert(OptionSimpleEnum{e});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_option_every_primitive_struct, ctx, std::optional<EveryPrimitiveStruct> s) {
    ctx.insert(OptionEveryPrimitiveStruct{s});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_option_vec_option_i32, ctx, std::optional<std::vector<std::optional<int32_t>>> v) {
    ctx.insert(OptionVecOptionI32{v});
}

// Tables with unique constraints
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueU8, unique_u8, public, unique<uint8_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueU16, unique_u16, public, unique<uint16_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueU32, unique_u32, public, unique<uint32_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueU64, unique_u64, public, unique<uint64_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueU128, unique_u128, public, unique<U128> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueU256, unique_u256, public, unique<U256> n; int32_t data);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueI8, unique_i8, public, unique<int8_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueI16, unique_i16, public, unique<int16_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueI32, unique_i32, public, unique<int32_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueI64, unique_i64, public, unique<int64_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueI128, unique_i128, public, unique<I128> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueI256, unique_i256, public, unique<I256> n; int32_t data);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueBool, unique_bool, public, unique<bool> b; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueString, unique_string, public, unique<std::string> s; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueIdentity, unique_identity, public, unique<SpacetimeDb::Identity> i; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(UniqueConnectionId, unique_connection_id, public, unique<SpacetimeDb::ConnectionId> a; int32_t data);

// Reducers for unique inserts, updates, and deletes
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_unique_u8, ctx, uint8_t n, int32_t data) {
    ctx.insert(UniqueU8{n, data});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(update_unique_u8, ctx, uint8_t n, int32_t data) {
    auto row = ctx.find_by_unique<UniqueU8>("n", n);
    if (row) {
        row->data = data;
        ctx.update(std::move(*row));
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(delete_unique_u8, ctx, uint8_t n) {
    ctx.delete_where<UniqueU8>([n](const UniqueU8& row) { return row.n == n; });
}

// Similar reducers for other unique types...
// (I'll show a few more examples and you can follow the pattern)

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_unique_u32, ctx, uint32_t n, int32_t data) {
    ctx.insert(UniqueU32{n, data});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(update_unique_u32, ctx, uint32_t n, int32_t data) {
    auto row = ctx.find_by_unique<UniqueU32>("n", n);
    if (row) {
        row->data = data;
        ctx.update(std::move(*row));
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(delete_unique_u32, ctx, uint32_t n) {
    ctx.delete_where<UniqueU32>([n](const UniqueU32& row) { return row.n == n; });
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_unique_string, ctx, std::string s, int32_t data) {
    ctx.insert(UniqueString{s, data});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(update_unique_string, ctx, std::string s, int32_t data) {
    auto row = ctx.find_by_unique<UniqueString>("s", s);
    if (row) {
        row->data = data;
        ctx.update(std::move(*row));
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(delete_unique_string, ctx, std::string s) {
    ctx.delete_where<UniqueString>([&s](const UniqueString& row) { return row.s == s; });
}

// Tables with primary keys
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkU8, pk_u8, public, primary_key<uint8_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkU16, pk_u16, public, primary_key<uint16_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkU32, pk_u32, public, primary_key<uint32_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkU32Two, pk_u32_two, public, primary_key<uint32_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkU64, pk_u64, public, primary_key<uint64_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkU128, pk_u128, public, primary_key<U128> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkU256, pk_u256, public, primary_key<U256> n; int32_t data);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkI8, pk_i8, public, primary_key<int8_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkI16, pk_i16, public, primary_key<int16_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkI32, pk_i32, public, primary_key<int32_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkI64, pk_i64, public, primary_key<int64_t> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkI128, pk_i128, public, primary_key<I128> n; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkI256, pk_i256, public, primary_key<I256> n; int32_t data);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkBool, pk_bool, public, primary_key<bool> b; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkString, pk_string, public, primary_key<std::string> s; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkIdentity, pk_identity, public, primary_key<SpacetimeDb::Identity> i; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkConnectionId, pk_connection_id, public, primary_key<SpacetimeDb::ConnectionId> a; int32_t data);
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(PkSimpleEnum, pk_simple_enum, public, primary_key<SimpleEnum> a; int32_t data);

// Reducers for primary key operations
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_pk_u32, ctx, uint32_t n, int32_t data) {
    ctx.insert(PkU32{n, data});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(update_pk_u32, ctx, uint32_t n, int32_t data) {
    auto row = ctx.find<PkU32>(n);
    if (row) {
        row->data = data;
        ctx.update(std::move(*row));
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(delete_pk_u32, ctx, uint32_t n) {
    ctx.delete_by_id<PkU32>(n);
}

// Similar for other PK types...

// Special reducers for caller identity/connection
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_one_identity, ctx) {
    ctx.insert(OneIdentity{ctx.sender()});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_vec_identity, ctx) {
    ctx.insert(VecIdentity{std::vector<SpacetimeDb::Identity>{ctx.sender()}});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_unique_identity, ctx, int32_t data) {
    ctx.insert(UniqueIdentity{ctx.sender(), data});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_pk_identity, ctx, int32_t data) {
    ctx.insert(PkIdentity{ctx.sender(), data});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_one_connection_id, ctx) {
    auto conn_id = ctx.connection_id();
    if (conn_id) {
        ctx.insert(OneConnectionId{*conn_id});
    } else {
        ctx.fail("No connection id in reducer context");
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_vec_connection_id, ctx) {
    auto conn_id = ctx.connection_id();
    if (conn_id) {
        ctx.insert(VecConnectionId{std::vector<SpacetimeDb::ConnectionId>{*conn_id}});
    } else {
        ctx.fail("No connection id in reducer context");
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_unique_connection_id, ctx, int32_t data) {
    auto conn_id = ctx.connection_id();
    if (conn_id) {
        ctx.insert(UniqueConnectionId{*conn_id, data});
    } else {
        ctx.fail("No connection id in reducer context");
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_caller_pk_connection_id, ctx, int32_t data) {
    auto conn_id = ctx.connection_id();
    if (conn_id) {
        ctx.insert(PkConnectionId{*conn_id, data});
    } else {
        ctx.fail("No connection id in reducer context");
    }
}

// Timestamp reducer
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_call_timestamp, ctx) {
    ctx.insert(OneTimestamp{ctx.timestamp()});
}

// Large table with many fields
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(LargeTable, large_table, public,
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint64_t d;
    U128 e;
    U256 f;
    int8_t g;
    int16_t h;
    int32_t i;
    int64_t j;
    I128 k;
    I256 l;
    bool m;
    float n;
    double o;
    std::string p;
    SimpleEnum q;
    EnumWithPayload r;
    UnitStruct s;
    ByteStruct t;
    EveryPrimitiveStruct u;
    EveryVecStruct v
);

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_large_table, ctx,
    uint8_t a, uint16_t b, uint32_t c, uint64_t d, U128 e, U256 f,
    int8_t g, int16_t h, int32_t i, int64_t j, I128 k, I256 l,
    bool m, float n, double o, std::string p, SimpleEnum q, EnumWithPayload r,
    UnitStruct s, ByteStruct t, EveryPrimitiveStruct u, EveryVecStruct v) {
    ctx.insert(LargeTable{a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(delete_large_table, ctx,
    uint8_t a, uint16_t b, uint32_t c, uint64_t d, U128 e, U256 f,
    int8_t g, int16_t h, int32_t i, int64_t j, I128 k, I256 l,
    bool m, float n, double o, std::string p, SimpleEnum q, EnumWithPayload r,
    UnitStruct s, ByteStruct t, EveryPrimitiveStruct u, EveryVecStruct v) {
    ctx.delete_where<LargeTable>([&](const LargeTable& row) {
        return row.a == a && row.b == b && row.c == c;  // Just match on first few fields
    });
}

// Table that holds other tables
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(TableHoldsTable, table_holds_table, public,
    OneU8 a;
    VecU8 b
);

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_table_holds_table, ctx, OneU8 a, VecU8 b) {
    ctx.insert(TableHoldsTable{a, b});
}

// No-op reducer
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(no_op_succeeds, ctx) {
    // Does nothing, just succeeds
}

// BTree indexed table
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(BTreeU32, btree_u32, public,
    btree<uint32_t> n;
    int32_t data
);

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_into_btree_u32, ctx, std::vector<BTreeU32> rows) {
    for (const auto& row : rows) {
        ctx.insert(row);
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(delete_from_btree_u32, ctx, std::vector<BTreeU32> rows) {
    for (const auto& row : rows) {
        ctx.delete_row(row);
    }
}

// Scheduled table
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(ScheduledTable, scheduled_table, public,
    autoinc<uint64_t> scheduled_id;
    SpacetimeDb::ScheduleAt scheduled_at;
    std::string text
);

// TODO: Fix scheduled reducer definitions
// SPACETIMEDB_SCHEDULED_REDUCER(send_scheduled_message, ctx, ScheduledTable arg) {
    // Just access the fields to ensure they work
    auto id = arg.scheduled_id;
    auto at = arg.scheduled_at;
    auto text = arg.text;
    (void)id;
    (void)at;
    (void)text;
}

// Indexed tables
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(IndexedTable, indexed_table, public,
    btree<uint32_t> player_id
);

// TODO: Fix table definitions
// SPACETIMEDB_TABLE(IndexedTable2, indexed_table_2, public,
    btree<uint32_t> player_id;
    btree<float> player_snazz
);

// Users table with client visibility filter
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(Users, users, public,
    primary_key<SpacetimeDb::Identity> identity;
    std::string name
);

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_user, ctx, std::string name, SpacetimeDb::Identity identity) {
    ctx.insert(Users{identity, name});
}

// Client visibility filters
SPACETIMEDB_CLIENT_FILTER(ONE_U8_VISIBLE, "SELECT * FROM one_u8");
SPACETIMEDB_CLIENT_FILTER(USERS_FILTER, "SELECT * FROM users WHERE identity = :sender");

// More complex reducers
// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_primitives_as_strings, ctx, EveryPrimitiveStruct s) {
    std::vector<std::string> strings = {
        std::to_string(s.a),
        std::to_string(s.b),
        std::to_string(s.c),
        std::to_string(s.d),
        s.e.to_string(),
        s.f.to_string(),
        std::to_string(s.g),
        std::to_string(s.h),
        std::to_string(s.i),
        std::to_string(s.j),
        s.k.to_string(),
        s.l.to_string(),
        s.m ? "true" : "false",
        std::to_string(s.n),
        std::to_string(s.o),
        s.p,
        s.q.to_string(),
        s.r.to_string(),
        s.s.to_string(),
        s.t.to_string()
    };
    ctx.insert(VecString{strings});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_unique_u32_update_pk_u32, ctx, uint32_t n, int32_t d_unique, int32_t d_pk) {
    ctx.insert(UniqueU32{n, d_unique});
    auto pk_row = ctx.find<PkU32>(n);
    if (pk_row) {
        pk_row->data = d_pk;
        ctx.update(std::move(*pk_row));
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(delete_pk_u32_insert_pk_u32_two, ctx, uint32_t n, int32_t data) {
    ctx.insert(PkU32Two{n, data});
    ctx.delete_by_id<PkU32>(n);
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_into_pk_btree_u32, ctx, std::vector<PkU32> pk_u32, std::vector<BTreeU32> bt_u32) {
    for (const auto& row : pk_u32) {
        ctx.insert(row);
    }
    for (const auto& row : bt_u32) {
        ctx.insert(row);
    }
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(update_pk_simple_enum, ctx, SimpleEnum a, int32_t data) {
    auto row = ctx.find<PkSimpleEnum>(a);
    if (row) {
        row->data = data;
        ctx.update(std::move(*row));
    } else {
        ctx.fail("row not found");
    }
}

// Indexed simple enum table
// TODO: Fix table definitions
// SPACETIMEDB_TABLE(IndexedSimpleEnum, indexed_simple_enum, public,
    btree<SimpleEnum> n
);

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(insert_into_indexed_simple_enum, ctx, SimpleEnum n) {
    ctx.insert(IndexedSimpleEnum{n});
}

// TODO: Fix reducer definitions
// SPACETIMEDB_REDUCER(update_indexed_simple_enum, ctx, SimpleEnum a, SimpleEnum b) {
    auto rows = ctx.select<IndexedSimpleEnum>([a](const IndexedSimpleEnum& row) {
        return row.n == a;
    });
    if (!rows.empty()) {
        ctx.delete_where<IndexedSimpleEnum>([a](const IndexedSimpleEnum& row) {
            return row.n == a;
        });
        ctx.insert(IndexedSimpleEnum{b});
    }
}