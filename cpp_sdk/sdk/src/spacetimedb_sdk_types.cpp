#include "spacetimedb/sdk/spacetimedb_sdk_types.h"
#include "spacetimedb/bsatn/reader.h"
#include "spacetimedb/bsatn/writer.h"

#include <vector>
#include <array>
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::copy
#include <cstddef>   // For std::byte

namespace SpacetimeDb {
    namespace sdk {

        // Identity
        void Identity::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_bytes(std::vector<std::byte>(reinterpret_cast<const std::byte*>(this->value.data()), reinterpret_cast<const std::byte*>(this->value.data() + this->value.size())));
        }
        void Identity::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            std::vector<std::byte> bytes = reader.read_fixed_bytes(IDENTITY_SIZE);
            if (bytes.size() == IDENTITY_SIZE) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<std::byte*>(this->value.data()));
            }
            else {
                // Consider a more robust error handling strategy for your application
                throw std::runtime_error("Failed to read enough bytes for Identity");
            }
        }

        // Timestamp
        void Timestamp::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_u64_le(this->ms_since_epoch);
        }
        void Timestamp::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->ms_since_epoch = reader.read_u64_le();
        }

        // ScheduleAt
        void ScheduleAt::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_u64_le(this->timestamp_micros);
        }
        void ScheduleAt::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->timestamp_micros = reader.read_u64_le();
        }

        // ConnectionId
        void ConnectionId::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // Converting uint64_t to a byte array for write_bytes, little-endian.
            std::array<std::byte, sizeof(this->id)> id_bytes;
            uint64_t n = this->id;
            for (size_t i = 0; i < sizeof(this->id); ++i) {
                id_bytes[i] = static_cast<std::byte>(n & 0xFF);
                n >>= 8;
            }
            writer.write_bytes(std::vector<std::byte>(id_bytes.begin(), id_bytes.end()));
        }
        void ConnectionId::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            std::vector<std::byte> id_bytes_vec = reader.read_fixed_bytes(sizeof(this->id));
            if (id_bytes_vec.size() == sizeof(this->id)) {
                this->id = 0; // Assuming little-endian
                for (size_t i = 0; i < sizeof(this->id); ++i) {
                    this->id |= static_cast<uint64_t>(static_cast<unsigned char>(id_bytes_vec[i])) << (i * 8);
                }
            }
            else {
                throw std::runtime_error("Failed to read enough bytes for ConnectionId");
            }
        }

        // TimeDuration
        void TimeDuration::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_i64_le(this->nanoseconds);
        }
        void TimeDuration::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->nanoseconds = reader.read_i64_le();
        }

        // u256_placeholder
        void u256_placeholder::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // Assuming writer has a method to write raw bytes for fixed-size arrays,
            // or u256_placeholder has a .data() and .size() like Identity for write_bytes_raw.
            // Based on writer.h, write_u256_le exists. Let's use that if available,
            // otherwise, write_bytes_raw is a fallback.
            writer.write_u256_le(*this);
        }
        void u256_placeholder::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            // Similar to serialize, assuming reader.read_u256_le() or fallback
            // *this = reader.read_u256_le(); // If Reader has this method
            // Fallback:
            std::vector<std::byte> bytes = reader.read_fixed_bytes(sizeof(this->data));
            if (bytes.size() == sizeof(this->data)) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<std::byte*>(this->data.data()));
            }
            else {
                throw std::runtime_error("Failed to read enough bytes for u256_placeholder");
            }
        }

        // i256_placeholder
        void i256_placeholder::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_i256_le(*this);
        }
        void i256_placeholder::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            // *this = reader.read_i256_le(); // If Reader has this method
            // Fallback:
            std::vector<std::byte> bytes = reader.read_fixed_bytes(sizeof(this->data));
            if (bytes.size() == sizeof(this->data)) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<std::byte*>(this->data.data()));
            }
            else {
                throw std::runtime_error("Failed to read enough bytes for i256_placeholder");
            }
        }

    } // namespace sdk
} // namespace SpacetimeDb

namespace SpacetimeDb {
    namespace bsatn {

        void serialize(Writer& writer, const ::SpacetimeDb::sdk::Identity& value) {
            value.bsatn_serialize(writer);
        }

        void serialize(Writer& writer, const ::SpacetimeDb::sdk::ConnectionId& value) {
            value.bsatn_serialize(writer);
        }

        void serialize(Writer& writer, const ::SpacetimeDb::sdk::Timestamp& value) {
            value.bsatn_serialize(writer);
        }

    } // namespace bsatn
} // namespace SpacetimeDb
