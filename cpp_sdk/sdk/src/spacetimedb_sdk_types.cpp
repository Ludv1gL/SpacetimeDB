#include "spacetimedb/sdk/spacetimedb_sdk_types.h"
#include "spacetimedb/bsatn_all.h"

#include <vector>
#include <array>
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::copy
#include <cstddef>   // For size_t

namespace SpacetimeDb {
    namespace sdk {

        // Identity constructors
        Identity::Identity() {
            value.fill(0);
        }
        
        Identity::Identity(const std::array<uint8_t, IDENTITY_SIZE>& bytes) : value(bytes) {
        }
        
        const std::array<uint8_t, IDENTITY_SIZE>& Identity::get_bytes() const {
            return value;
        }
        
        std::string Identity::to_hex_string() const {
            // TODO: Implement hex string conversion
            return "placeholder_hex_string";
        }
        
        bool Identity::operator==(const Identity& other) const {
            return value == other.value;
        }
        
        bool Identity::operator!=(const Identity& other) const {
            return !(*this == other);
        }
        
        bool Identity::operator<(const Identity& other) const {
            return value < other.value;
        }

        // Timestamp constructors
        Timestamp::Timestamp() : ms_since_epoch(0) {
        }
        
        Timestamp::Timestamp(uint64_t milliseconds_since_epoch) : ms_since_epoch(milliseconds_since_epoch) {
        }
        
        uint64_t Timestamp::as_milliseconds() const {
            return ms_since_epoch;
        }
        
        Timestamp Timestamp::current() {
            // TODO: Implement actual current timestamp
            return Timestamp(0);
        }
        
        bool Timestamp::operator==(const Timestamp& other) const {
            return ms_since_epoch == other.ms_since_epoch;
        }
        
        bool Timestamp::operator!=(const Timestamp& other) const {
            return !(*this == other);
        }
        
        bool Timestamp::operator<(const Timestamp& other) const {
            return ms_since_epoch < other.ms_since_epoch;
        }
        
        bool Timestamp::operator<=(const Timestamp& other) const {
            return ms_since_epoch <= other.ms_since_epoch;
        }
        
        bool Timestamp::operator>(const Timestamp& other) const {
            return ms_since_epoch > other.ms_since_epoch;
        }
        
        bool Timestamp::operator>=(const Timestamp& other) const {
            return ms_since_epoch >= other.ms_since_epoch;
        }

        // Identity
        void Identity::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // Write raw bytes without length prefix for fixed-size Identity
            for (size_t i = 0; i < this->value.size(); ++i) {
                writer.write_u8(this->value[i]);
            }
        }
        void Identity::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            std::vector<uint8_t> bytes = reader.read_fixed_bytes(IDENTITY_SIZE);
            if (bytes.size() == IDENTITY_SIZE) {
                std::copy(bytes.begin(), bytes.end(), this->value.data());
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
            // ConnectionId is just a uint64_t, write it directly
            writer.write_u64_le(this->id);
        }
        void ConnectionId::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->id = reader.read_u64_le();
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
            std::vector<uint8_t> bytes = reader.read_fixed_bytes(sizeof(this->data));
            if (bytes.size() == sizeof(this->data)) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<uint8_t*>(this->data.data()));
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
            std::vector<uint8_t> bytes = reader.read_fixed_bytes(sizeof(this->data));
            if (bytes.size() == sizeof(this->data)) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<uint8_t*>(this->data.data()));
            }
            else {
                throw std::runtime_error("Failed to read enough bytes for i256_placeholder");
            }
        }

    } // namespace sdk
} // namespace SpacetimeDb

// Serialize functions are now defined as inline in writer.h to avoid duplicate definitions
