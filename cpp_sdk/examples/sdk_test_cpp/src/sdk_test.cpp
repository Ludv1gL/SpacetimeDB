#include "sdk_test.h"

namespace sdk_test_cpp {

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

// Simplified approach - just register a few key tables to start
SPACETIMEDB_TABLE(sdk_test_cpp::OneU8Row, "one_u8", true, "");
SPACETIMEDB_TABLE(sdk_test_cpp::OneStringRow, "one_string", true, "");
SPACETIMEDB_TABLE(sdk_test_cpp::OneIdentityRow, "one_identity", true, "");
