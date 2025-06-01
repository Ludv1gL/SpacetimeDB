#include "sdk_test.h" // Also includes spacetimedb_sdk_types.h, uint128_placeholder.h indirectly
#include <spacetimedb/bsatn/writer.h>
#include <spacetimedb/bsatn/reader.h>
#include <variant> // For std::visit
#include <stdexcept> // For std::runtime_error

// Ensure SpacetimeDb::bsatn::serialize and deserialize are available for all variant types
// For SDK types, they are already in spacetimedb_sdk_types.h or bsatn/reader.h/writer.h
// For std::vector<SimpleEnum>, ensure SimpleEnum's bsatn functions are available.
// SimpleEnum is already handled by the generic bsatn::deserialize/serialize templates for enums.

namespace sdk_test_cpp {

    void EnumWithPayload::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
        // Serialize the tag
        ::SpacetimeDb::bsatn::serialize(writer, this->tag);

        // Serialize the value using std::visit
        std::visit([&writer](const auto& arg) {
            ::SpacetimeDb::bsatn::serialize(writer, arg);
            }, this->value);
    }

    void EnumWithPayload::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
        // Deserialize the tag
        this->tag = ::SpacetimeDb::bsatn::deserialize<sdk_test_cpp::EnumWithPayloadTag>(reader);

        // Deserialize the value based on the tag
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
            // Note: EnumWithPayload in sdk_test.h uses ::SpacetimeDb::bsatn::uint128_placeholder,
            // which was changed to ::SpacetimeDb::Types::uint128_t_placeholder.
            // Reader::read_u128_le() returns ::SpacetimeDb::Types::uint128_t_placeholder.
            this->value = reader.read_u128_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagU256:
            // Note: EnumWithPayload in sdk_test.h uses ::SpacetimeDb::sdk::u256_placeholder.
            // Reader::read_u256_le() returns ::SpacetimeDb::sdk::u256_placeholder.
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
            // Similar to U128, type is ::SpacetimeDb::Types::int128_t_placeholder
            this->value = reader.read_i128_le();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagI256:
            // Similar to U256, type is ::SpacetimeDb::sdk::i256_placeholder
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
        case sdk_test_cpp::EnumWithPayloadTag::TagBytes: // std::vector<uint8_t>
            // EnumWithPayload variant type is std::vector<uint8_t>
            this->value = reader.read_vector<uint8_t>();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagInts: // std::vector<int32_t>
            this->value = reader.read_vector<int32_t>();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagStrings: // std::vector<std::string>
            this->value = reader.read_vector<std::string>();
            break;
        case sdk_test_cpp::EnumWithPayloadTag::TagSimpleEnums: // std::vector<SimpleEnum>
            this->value = reader.read_vector<sdk_test_cpp::SimpleEnum>();
            break;
        default:
            // Handle unknown tag, e.g., by throwing an exception
            throw std::runtime_error("Unknown tag encountered during EnumWithPayload deserialization: " + std::to_string(static_cast<int>(this->tag)));
        }
    }

} // namespace sdk_test_cpp
