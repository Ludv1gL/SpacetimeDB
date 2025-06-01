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
            void bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
                // Assuming Writer has a method like write_bytes_raw or similar for fixed-size arrays
                // If writer.write_bytes expects std::vector<std::byte>, a conversion is needed:
                // const std::byte* bytes_ptr = reinterpret_cast<const std::byte*>(this->value.data());
                // writer.write_bytes(std::vector<std::byte>(bytes_ptr, bytes_ptr + this->value.size()));
                // For now, let's assume a direct write method or that write_bytes can handle it.
                // Based on writer.h, it has write_bytes(const std::vector<std::byte>& value)
                // and write_bytes_raw(const void* data, size_t size). Let's use write_bytes_raw.
                writer.write_bytes_raw(this->value.data(), this->value.size());
            }
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
            void bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
                writer.write_u64_le(this->ms_since_epoch); // Using ms_since_epoch as per struct def
            }
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

            void bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
                // The request was writer.write_bytes(this->value);
                // Assuming this->value refers to this->id which is uint64_t.
                // writer.write_bytes expects const std::vector<std::byte>&.
                // So, we must convert uint64_t to std::vector<std::byte>.
                // This is unusual; writer.write_u64_le(this->id) would be more direct.
                // Adhering to 'write_bytes' requirement:
                std::array<std::byte, sizeof(this->id)> id_bytes;
                uint64_t n = this->id; // Assuming little-endian for now, matching write_u64_le
                for (size_t i = 0; i < sizeof(this->id); ++i) {
                    id_bytes[i] = static_cast<std::byte>(n & 0xFF);
                    n >>= 8;
                }
                writer.write_bytes(std::vector<std::byte>(id_bytes.begin(), id_bytes.end()));
            }
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
