#ifndef SPACETIMEDB_SDK_TYPES_H
#define SPACETIMEDB_SDK_TYPES_H

// Global forward declarations
namespace SpacetimeDb {
    namespace bsatn {
        class Reader;
        class Writer;
    } // namespace bsatn
} // namespace SpacetimeDb

#include "spacetimedb/bsatn/uint128_placeholder.h" // For uint128_t_placeholder
#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <chrono>
#include <stdexcept>

#include "spacetimedb/bsatn/writer.h" // Full definition for Writer
#include "spacetimedb/bsatn/reader.h" // Full definition for Reader

namespace SpacetimeDb {
    namespace sdk {

        const size_t IDENTITY_SIZE = 32;

        class Identity {
        public:
            Identity();
            explicit Identity(const std::array<uint8_t, IDENTITY_SIZE>& bytes);
            // static Identity from_hex_string(const std::string& hex_str); // Implementation can be added if needed

            const std::array<uint8_t, IDENTITY_SIZE>& get_bytes() const;
            std::string to_hex_string() const;

            bool operator==(const Identity& other) const;
            bool operator!=(const Identity& other) const;
            bool operator<(const Identity& other) const; // For std::map keys


            // BSATN Serialization methods (duck-typed, or inherit BsatnSerializable)
            void bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const; // Declaration only
            void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);

        private:
            std::array<uint8_t, IDENTITY_SIZE> value;
        };

        class Timestamp {
        public:
            Timestamp();
            explicit Timestamp(uint64_t milliseconds_since_epoch);

            uint64_t as_milliseconds() const;

            static Timestamp current();

            bool operator==(const Timestamp& other) const;
            bool operator!=(const Timestamp& other) const;
            bool operator<(const Timestamp& other) const;
            bool operator<=(const Timestamp& other) const;
            bool operator>(const Timestamp& other) const;
            bool operator>=(const Timestamp& other) const;

            // BSATN Serialization methods
            void bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const; // Declaration only
            void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);

        private:
            uint64_t ms_since_epoch;
        };

        // Placeholder for a type that might be used with scheduled tables/reducers
        struct ScheduleAt {
            uint64_t timestamp_micros; // Or whatever precision/representation is decided

            ScheduleAt() : timestamp_micros(0) {}
            explicit ScheduleAt(uint64_t ts_micros) : timestamp_micros(ts_micros) {}

            // BSATN Serialization methods
            void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const;
            void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);

            bool operator==(const ScheduleAt& other) const {
                return timestamp_micros == other.timestamp_micros;
            }
            // Add other operators as needed
        };

        // Placeholder for ConnectionId
        struct ConnectionId {
            uint64_t id;

            ConnectionId(uint64_t val = 0) : id(val) {}

            void bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const; // Declaration only
            void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);
            bool operator==(const ConnectionId& other) const { return id == other.id; }
            bool operator<(const ConnectionId& other) const { return id < other.id; } // For map keys
        };

        // Placeholder for TimeDuration
        struct TimeDuration {
            int64_t nanoseconds; // Example: represent as nanoseconds

            TimeDuration(int64_t val = 0) : nanoseconds(val) {}

            void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const;
            void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);
            bool operator==(const TimeDuration& other) const { return nanoseconds == other.nanoseconds; }
            bool operator<(const TimeDuration& other) const { return nanoseconds < other.nanoseconds; } // For map keys
        };

        // Basic Placeholders for u256/i256 - to be fully integrated later
        // Note: The prompt mentions uint128_placeholder.h, but the types here are u256/i256.
        // Assuming these placeholders are what's intended to be used with reader/writer from bsatn.
        struct u256_placeholder {
            std::array<uint64_t, 4> data; // Example internal representation
            u256_placeholder() { data.fill(0); }
            void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const; // Declaration only
            void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);   // Declaration only
            bool operator==(const u256_placeholder& other) const { return data == other.data; }
            bool operator<(const u256_placeholder& other) const { return data < other.data; } // For map keys
        };

        struct i256_placeholder {
            std::array<uint64_t, 4> data; // Example internal representation (sign bit would be in data[3])
            i256_placeholder() { data.fill(0); }
            void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const; // Declaration only
            void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);   // Declaration only
            bool operator==(const i256_placeholder& other) const { return data == other.data; }
            bool operator<(const i256_placeholder& other) const { return data < other.data; } // For map keys
        };
    } // namespace sdk
} // namespace SpacetimeDb

namespace SpacetimeDb {
    namespace sdk {

        // Identity
        inline void Identity::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_bytes(std::vector<std::byte>(reinterpret_cast<const std::byte*>(this->value.data()), reinterpret_cast<const std::byte*>(this->value.data() + this->value.size())));
        }
        inline void Identity::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            std::vector<std::byte> bytes = reader.read_bytes(IDENTITY_SIZE);
            if (bytes.size() == IDENTITY_SIZE) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<std::byte*>(this->value.data()));
            }
            else {
                // Consider a more robust error handling strategy for your application
                throw std::runtime_error("Failed to read enough bytes for Identity");
            }
        }

        // Timestamp
        inline void Timestamp::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_u64_le(this->ms_since_epoch);
        }
        inline void Timestamp::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->ms_since_epoch = reader.read_u64_le();
        }

        // ScheduleAt
        inline void ScheduleAt::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_u64_le(this->timestamp_micros);
        }
        inline void ScheduleAt::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->timestamp_micros = reader.read_u64_le();
        }

        // ConnectionId
        inline void ConnectionId::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // Converting uint64_t to a byte array for write_bytes, little-endian.
            std::array<std::byte, sizeof(this->id)> id_bytes;
            uint64_t n = this->id;
            for (size_t i = 0; i < sizeof(this->id); ++i) {
                id_bytes[i] = static_cast<std::byte>(n & 0xFF);
                n >>= 8;
            }
            writer.write_bytes(std::vector<std::byte>(id_bytes.begin(), id_bytes.end()));
        }
        inline void ConnectionId::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            std::vector<std::byte> id_bytes_vec = reader.read_bytes(sizeof(this->id));
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
        inline void TimeDuration::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_i64_le(this->nanoseconds);
        }
        inline void TimeDuration::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->nanoseconds = reader.read_i64_le();
        }

        // u256_placeholder
        inline void u256_placeholder::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // Assuming writer has a method to write raw bytes for fixed-size arrays,
            // or u256_placeholder has a .data() and .size() like Identity for write_bytes_raw.
            // Based on writer.h, write_u256_le exists. Let's use that if available,
            // otherwise, write_bytes_raw is a fallback.
            writer.write_u256_le(*this);
        }
        inline void u256_placeholder::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            // Similar to serialize, assuming reader.read_u256_le() or fallback
            // *this = reader.read_u256_le(); // If Reader has this method
            // Fallback:
            std::vector<std::byte> bytes = reader.read_bytes(sizeof(this->data));
            if (bytes.size() == sizeof(this->data)) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<std::byte*>(this->data.data()));
            }
            else {
                throw std::runtime_error("Failed to read enough bytes for u256_placeholder");
            }
        }

        // i256_placeholder
        inline void i256_placeholder::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_i256_le(*this);
        }
        inline void i256_placeholder::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            // *this = reader.read_i256_le(); // If Reader has this method
            // Fallback:
            std::vector<std::byte> bytes = reader.read_bytes(sizeof(this->data));
            if (bytes.size() == sizeof(this->data)) {
                std::copy(bytes.begin(), bytes.end(), reinterpret_cast<std::byte*>(this->data.data()));
            }
            else {
                throw std::runtime_error("Failed to read enough bytes for i256_placeholder");
            }
        }

    }
} // namespace SpacetimeDb::sdk

namespace SpacetimeDb {
    namespace bsatn {

        inline void serialize(Writer& writer, const ::SpacetimeDb::sdk::Identity& value) {
            value.bsatn_serialize(writer);
        }

        inline void serialize(Writer& writer, const ::SpacetimeDb::sdk::ConnectionId& value) {
            value.bsatn_serialize(writer);
        }

        inline void serialize(Writer& writer, const ::SpacetimeDb::sdk::Timestamp& value) {
            value.bsatn_serialize(writer);
        }

    }
} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_SDK_TYPES_H
