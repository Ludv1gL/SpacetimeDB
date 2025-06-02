// Fixed version of sdk_test.h that uses the comprehensive bridge
#ifndef SDK_TEST_FIXED_H
#define SDK_TEST_FIXED_H

// Use our comprehensive bridge instead of problematic SDK headers
#include "comprehensive_sdk_bridge.h"
#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include <optional>

// Test types outside namespace (from original)
enum class GlobalSimpleEnum : uint8_t {
    GlobalZero,
    GlobalOne,
    GlobalTwo,
};

struct GlobalUnitStruct {};

namespace sdk_test_cpp {

    enum class SimpleEnum : uint8_t {
        Zero,
        One,
        Two,
    };

    struct UnitStruct {};

    struct ByteStruct {
        uint8_t b;
    };

    struct EveryPrimitiveStruct {
        uint8_t a;
        uint16_t b;
        uint32_t c;
        uint64_t d;
        ::SpacetimeDb::Types::uint128_t_placeholder e;
        ::SpacetimeDb::sdk::u256_placeholder f;
        int8_t g;
        int16_t h;
        int32_t i;
        int64_t j;
        ::SpacetimeDb::Types::int128_t_placeholder k;
        ::SpacetimeDb::sdk::i256_placeholder l;
        bool m;
        float n;
        double o;
        std::string p;
        ::SpacetimeDb::sdk::Identity q;
        ::SpacetimeDb::sdk::ConnectionId r;
        ::SpacetimeDb::sdk::Timestamp s;
        ::SpacetimeDb::sdk::TimeDuration t;
    };

    struct EveryVecStruct {
        std::vector<uint8_t> a;
        std::vector<uint16_t> b;
        std::vector<uint32_t> c;
        std::vector<uint64_t> d;
        std::vector<::SpacetimeDb::Types::uint128_t_placeholder> e;
        std::vector<::SpacetimeDb::sdk::u256_placeholder> f;
        std::vector<int8_t> g;
        std::vector<int16_t> h;
        std::vector<int32_t> i;
        std::vector<int64_t> j;
        std::vector<::SpacetimeDb::Types::int128_t_placeholder> k;
        std::vector<::SpacetimeDb::sdk::i256_placeholder> l;
        std::vector<bool> m;
        std::vector<float> n;
        std::vector<double> o;
        std::vector<std::string> p;
        std::vector<::SpacetimeDb::sdk::Identity> q;
        std::vector<::SpacetimeDb::sdk::ConnectionId> r;
        std::vector<::SpacetimeDb::sdk::Timestamp> s;
        std::vector<::SpacetimeDb::sdk::TimeDuration> t;
    };

    // Define an enum for the tags
    enum class EnumWithPayloadTag : uint8_t {
        TagU8, TagU16, TagU32, TagU64, TagU128, TagU256,
        TagI8, TagI16, TagI32, TagI64, TagI128, TagI256,
        TagBool, TagF32, TagF64, TagStr, TagIdentity, TagConnectionId,
        TagTimestamp, TagBytes, TagInts, TagStrings, TagSimpleEnums
    };

    struct EnumWithPayload {
        sdk_test_cpp::EnumWithPayloadTag tag;
        std::variant<
            uint8_t, uint16_t, uint32_t, uint64_t, ::SpacetimeDb::Types::uint128_t_placeholder, ::SpacetimeDb::sdk::u256_placeholder,
            int8_t, int16_t, int32_t, int64_t, ::SpacetimeDb::Types::int128_t_placeholder, ::SpacetimeDb::sdk::i256_placeholder,
            bool, float, double, std::string, ::SpacetimeDb::sdk::Identity, ::SpacetimeDb::sdk::ConnectionId,
            ::SpacetimeDb::sdk::Timestamp, std::vector<uint8_t>, std::vector<int32_t>,
            std::vector<std::string>, std::vector<SimpleEnum>
        > value;

        // Manual BSATN methods (stub implementations)
        void bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // Stub implementation
        }
        void bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            // Stub implementation
        }
    };

    // 'One*' Series of Row Structs for Tables
    struct OneU8Row { uint8_t n; };
    struct OneU16Row { uint16_t n; };
    struct OneU32Row { uint32_t n; };
    struct OneU64Row { uint64_t n; };
    struct OneU128Row { ::SpacetimeDb::Types::uint128_t_placeholder n; };
    struct OneU256Row { ::SpacetimeDb::sdk::u256_placeholder n; };

    struct OneI8Row { int8_t n; };
    struct OneI16Row { int16_t n; };
    struct OneI32Row { int32_t n; };
    struct OneI64Row { int64_t n; };
    struct OneI128Row { ::SpacetimeDb::Types::int128_t_placeholder n; };
    struct OneI256Row { ::SpacetimeDb::sdk::i256_placeholder n; };

    struct OneBoolRow { bool n; };
    struct OneF32Row { float n; };
    struct OneF64Row { double n; };
    struct OneStringRow { std::string n; };
    struct OneIdentityRow { ::SpacetimeDb::sdk::Identity n; };
    struct OneConnectionIdRow { ::SpacetimeDb::sdk::ConnectionId n; };
    struct OneTimestampRow { ::SpacetimeDb::sdk::Timestamp n; };
    struct OneSimpleEnumRow { SimpleEnum n; };
    struct OneEnumWithPayloadRow { EnumWithPayload n; };

    struct OneUnitStructRow { UnitStruct s; };
    struct OneByteStructRow { ByteStruct s; };
    struct OneEveryPrimitiveStructRow { EveryPrimitiveStruct s; };
    struct OneEveryVecStructRow { EveryVecStruct s; };

    // "Vec*" Series of Row Structs for Tables
    struct VecU8Row { std::vector<uint8_t> n; };
    struct VecStringRow { std::vector<std::string> n; };
    struct VecSimpleEnumRow { std::vector<SimpleEnum> n; };
    struct VecEveryPrimitiveStructRow { std::vector<EveryPrimitiveStruct> s; };

    // "Unique*" Series of Row Structs for Tables
    struct UniqueU8Row {
        uint64_t row_id;
        uint8_t n;
        int32_t data;
    };

    struct UniqueStringRow {
        uint64_t row_id;
        std::string s;
        int32_t data;
    };

    struct UniqueIdentityRow {
        uint64_t row_id;
        ::SpacetimeDb::sdk::Identity i;
        int32_t data;
    };

    struct UniqueConnectionIdRow {
        uint64_t row_id;
        ::SpacetimeDb::sdk::ConnectionId c;
        int32_t data;
    };

    // Large table combining many types
    struct LargeTableRow {
        uint64_t row_id; // PK
        uint8_t a;
        uint16_t b;
        uint32_t c;
        uint64_t d;
        ::SpacetimeDb::Types::uint128_t_placeholder e;
        ::SpacetimeDb::sdk::u256_placeholder f;
        int8_t g;
        int16_t h;
        int32_t i;
        int64_t j;
        ::SpacetimeDb::Types::int128_t_placeholder k;
        ::SpacetimeDb::sdk::i256_placeholder l;
        bool m;
        float n;
        double o;
        std::string p;
        ::SpacetimeDb::sdk::Identity q;
        ::SpacetimeDb::sdk::ConnectionId r;
        ::SpacetimeDb::sdk::Timestamp s;
        ::SpacetimeDb::sdk::TimeDuration t;
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

    // Table that holds other table types
    struct TableHoldsTableRow {
        uint64_t row_id; // PK
        OneU8Row a;
        VecU8Row b;
    };

    // Scheduled table
    struct ScheduledTableRow {
        uint64_t scheduled_id; // PK
        ::SpacetimeDb::sdk::ScheduleAt scheduled_at;
        std::string text;
    };

    // Indexed tables
    struct IndexedTableRow {
        uint64_t row_id; // PK
        uint32_t player_id;
    };

    struct IndexedTable2Row {
        uint64_t row_id; // PK
        uint32_t player_id;
        float player_snazz;
    };

    struct BTreeU32Row {
        uint64_t row_id; // PK
        uint32_t n;
        int32_t data;
    };

    struct UsersRow {
        ::SpacetimeDb::sdk::Identity identity; // PK
        std::string name;
    };

    struct IndexedSimpleEnumRow {
        uint64_t row_id; // PK
        SimpleEnum n;
    };

    // "Pk*" Series of Row Structs for Tables
    struct PkU8Row {
        uint8_t n;
        int32_t data;
    };

    struct PkStringRow {
        std::string s;
        int32_t data;
    };

    struct PkIdentityRow {
        ::SpacetimeDb::sdk::Identity i;
        int32_t data;
    };

    struct PkSimpleEnumRow {
        SimpleEnum e;
        int32_t data;
    };

    struct PkU32TwoRow {
        uint32_t a;
        uint32_t b;
        int32_t data;
    };

    // "Option*" Series of Row Structs for Tables
    struct OptionI32Row { std::optional<int32_t> n; };
    struct OptionStringRow { std::optional<std::string> n; };
    struct OptionIdentityRow { std::optional<::SpacetimeDb::sdk::Identity> n; };
    struct OptionSimpleEnumRow { std::optional<SimpleEnum> n; };
    struct OptionEveryPrimitiveStructRow { std::optional<EveryPrimitiveStruct> s; };

    // Complex vector option type
    struct VecOptionI32 {
        std::vector<std::optional<int32_t>> value;
    };

    struct OptionVecOptionI32Row {
        std::optional<VecOptionI32> v;
    };

} // namespace sdk_test_cpp

#endif // SDK_TEST_FIXED_H