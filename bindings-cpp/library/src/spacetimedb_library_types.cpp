#include "spacetimedb/types.h"
#include "spacetimedb/bsatn_all.h"

#include <vector>
#include <array>
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::copy
#include <cstddef>   // For size_t

namespace SpacetimeDb {

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


        // ConnectionId
        void ConnectionId::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // ConnectionId is just a uint64_t, write it directly
            writer.write_u64_le(this->id);
        }
        void ConnectionId::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
            this->id = reader.read_u64_le();
        }


        // u256
        void u256::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            // Assuming writer has a method to write raw bytes for fixed-size arrays,
            // or u256_placeholder has a .data() and .size() like Identity for write_bytes_raw.
            // Based on writer.h, write_u256_le exists. Let's use that if available,
            // otherwise, write_bytes_raw is a fallback.
            writer.write_u256_le(*this);
        }
        void u256::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
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

        // i256
        void i256::bsatn_serialize(::SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_i256_le(*this);
        }
        void i256::bsatn_deserialize(::SpacetimeDb::bsatn::Reader& reader) {
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

} // namespace SpacetimeDb

// Serialize functions are now defined as inline in writer.h to avoid duplicate definitions
