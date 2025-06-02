#include "sdk_test.h"

// SDK Headers required for SpacetimeDB module
#include <spacetimedb/sdk/spacetimedb_sdk_reducer.h>
#include <spacetimedb/sdk/database.h>
#include <spacetimedb/sdk/table.h>
#include <spacetimedb/abi/spacetimedb_abi.h>

// The _spacetimedb_sdk_init() function is defined in spacetimedb_sdk_reducer.h
// and will be exported. The host calls it to initialize the SDK.

namespace sdk_test_cpp {

    // ==========================================
    // EnumWithPayload Serialization Implementation
    // ==========================================
    
    void EnumWithPayload::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
        ::SpacetimeDb::bsatn::serialize(writer, this->tag);
        std::visit([&writer](const auto& arg) {
            ::SpacetimeDb::bsatn::serialize(writer, arg);
        }, this->value);
    }

    void EnumWithPayload::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
        this->tag = ::SpacetimeDb::bsatn::deserialize<sdk_test_cpp::EnumWithPayloadTag>(reader);
        switch (this->tag) {
        case sdk_test_cpp::EnumWithPayloadTag::TagU8:
            this->value = reader.read_u8();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagU16:
            this->value = reader.read_u16_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagU32:
            this->value = reader.read_u32_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagU64:
            this->value = reader.read_u64_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagU128:
            this->value = reader.read_u128_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagU256:
            this->value = reader.read_u256_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagI8:
            this->value = reader.read_i8();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagI16:
            this->value = reader.read_i16_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagI32:
            this->value = reader.read_i32_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagI64:
            this->value = reader.read_i64_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagI128:
            this->value = reader.read_i128_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagI256:
            this->value = reader.read_i256_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagBool:
            this->value = reader.read_bool();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagF32:
            this->value = reader.read_f32_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagF64:
            this->value = reader.read_f64_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagStr:
            this->value = reader.read_string();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagIdentity:
            this->value = ::SpacetimeDb::bsatn::deserialize<::SpacetimeDb::sdk::Identity>(reader);
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagConnectionId:
            this->value = ::SpacetimeDb::bsatn::deserialize<::SpacetimeDb::sdk::ConnectionId>(reader);
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagTimestamp:
            this->value = ::SpacetimeDb::bsatn::deserialize<::SpacetimeDb::sdk::Timestamp>(reader);
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagBytes:
            this->value = reader.read_vector<uint8_t>();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagInts:
            this->value = reader.read_vector<int32_t>();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagStrings:
            this->value = reader.read_vector<std::string>();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagSimpleEnums:
            this->value = reader.read_vector<sdk_test_cpp::SimpleEnum>();
            break;
        default:
            throw std::runtime_error("Unknown tag");
        }
    }

} // namespace sdk_test_cpp

// ==========================================
// TABLE REGISTRATIONS
// ==========================================
// Tables are registered in groups by data type for clarity

// --- Primitive Type Tables ---
// Unsigned integers
SPACETIMEDB_TABLE(OneU8Row, "one_u8", true, "");
SPACETIMEDB_TABLE(OneU16Row, "one_u16", true, "");
SPACETIMEDB_TABLE(OneU32Row, "one_u32", true, "");
SPACETIMEDB_TABLE(OneU64Row, "one_u64", true, "");
SPACETIMEDB_TABLE(OneU128Row, "one_u128", true, "");
SPACETIMEDB_TABLE(OneU256Row, "one_u256", true, "");

// Signed integers
SPACETIMEDB_TABLE(OneI8Row, "one_i8", true, "");
SPACETIMEDB_TABLE(OneI16Row, "one_i16", true, "");
SPACETIMEDB_TABLE(OneI32Row, "one_i32", true, "");
SPACETIMEDB_TABLE(OneI64Row, "one_i64", true, "");
SPACETIMEDB_TABLE(OneI128Row, "one_i128", true, "");
SPACETIMEDB_TABLE(OneI256Row, "one_i256", true, "");

// Boolean and floating point
SPACETIMEDB_TABLE(OneBoolRow, "one_bool", true, "");
SPACETIMEDB_TABLE(OneF32Row, "one_f32", true, "");
SPACETIMEDB_TABLE(OneF64Row, "one_f64", true, "");

// String and special types
SPACETIMEDB_TABLE(OneStringRow, "one_string", true, "");
SPACETIMEDB_TABLE(OneIdentityRow, "one_identity", true, "");
SPACETIMEDB_TABLE(OneConnectionIdRow, "one_connection_id", true, "");
SPACETIMEDB_TABLE(OneTimestampRow, "one_timestamp", true, "");

// --- Struct and Complex Type Tables ---
SPACETIMEDB_TABLE(OneByteStructRow, "one_byte_struct", true, "");
SPACETIMEDB_TABLE(OneEnumWithPayloadRow, "one_enum_with_payload", true, "");
SPACETIMEDB_TABLE(OneEveryPrimitiveStructRow, "one_every_primitive_struct", true, "");
SPACETIMEDB_TABLE(OneEveryVecStructRow, "one_every_vec_struct", true, "");

// TODO: Continue adding remaining tables:
// - Vector tables (vec_u8, vec_u16, etc.)
// - Option tables (option_i32, option_string, etc.)
// - Unique constraint tables
// - Primary key tables
// - Indexed tables
// - Large table
// And their corresponding reducer implementations

// ==========================================
// REDUCER IMPLEMENTATIONS
// ==========================================

// Simple reducer to insert a u8 value
void insert_one_u8(spacetimedb::sdk::ReducerContext& ctx, uint8_t n) {
    auto table = ctx.db().get_table<OneU8Row>("one_u8");
    OneU8Row row{n};
    table.insert(row);
}

} // namespace sdk_test_cpp

// Register the reducer (must be outside namespace)
SPACETIMEDB_REDUCER(sdk_test_cpp::insert_one_u8, uint8_t);
