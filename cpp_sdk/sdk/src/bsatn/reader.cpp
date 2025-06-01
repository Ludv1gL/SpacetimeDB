#include "spacetimedb/bsatn/reader.h" // Updated include path
#include <cstring>   // For std::memcpy
#include <stdexcept> // For std::runtime_error, std::out_of_range
#include <limits>    // For std::numeric_limits
#include <algorithm> // For std::min (used in read_le_bytes via ensure_bytes)

// Helper for endian conversion if system is big-endian.
// For simplicity, this implementation assumes little-endian system.
namespace {
    template<typename T>
    T read_le_bytes(const std::byte*& current_ptr, const std::byte* end_ptr, size_t T_size) {
        // This helper is now slightly different as ensure_bytes is called by public methods.
        // We assume ensure_bytes(T_size) was called before this.
        // if (static_cast<size_t>(end_ptr - current_ptr) < T_size) {
        //     throw std::out_of_range("BSATN Reader: Not enough bytes to read type.");
        // }
        T value;
        // On a little-endian system, this is a direct copy.
        // On a big-endian system, bytes of `value` would need to be reversed after copy.
        std::memcpy(&value, current_ptr, T_size);
        current_ptr += T_size;
        return value;
    }
} // anonymous namespace


namespace SpacetimeDb {
    namespace bsatn {

        // Static constants defined in header, no need to redefine here unless they were not static const.
        // const uint32_t Reader::max_string_length_sanity_check; (already static const in .h)
        // const uint32_t Reader::max_vector_elements_sanity_check;

        // Constructor definitions are inline in reader.h

        void Reader::ensure_bytes(size_t count) {
            if (static_cast<size_t>(end_ptr - current_ptr) < count) {
                throw std::out_of_range(
                    "BSATN Reader: Not enough bytes remaining. Requested: " + std::to_string(count) +
                    ", Available: " + std::to_string(end_ptr - current_ptr));
            }
        }

        bool Reader::read_bool() {
            ensure_bytes(1);
            uint8_t val = static_cast<uint8_t>(*current_ptr);
            current_ptr++;
            if (val > 1) { // Strict bool (0 or 1)
                throw std::runtime_error("BSATN Reader: Invalid boolean value " + std::to_string(val));
            }
            return val == 1;
        }

        uint8_t Reader::read_u8() {
            ensure_bytes(1);
            uint8_t val = static_cast<uint8_t>(*current_ptr);
            current_ptr++;
            return val;
        }

        uint16_t Reader::read_u16_le() {
            ensure_bytes(sizeof(uint16_t));
            return ::read_le_bytes<uint16_t>(current_ptr, end_ptr, sizeof(uint16_t));
        }

        uint32_t Reader::read_u32_le() {
            ensure_bytes(sizeof(uint32_t));
            return ::read_le_bytes<uint32_t>(current_ptr, end_ptr, sizeof(uint32_t));
        }

        uint64_t Reader::read_u64_le() {
            ensure_bytes(sizeof(uint64_t));
            return ::read_le_bytes<uint64_t>(current_ptr, end_ptr, sizeof(uint64_t));
        }

        SpacetimeDb::Types::uint128_t_placeholder Reader::read_u128_le() {
            // ensure_bytes for both parts is handled by individual read_u64_le calls
            SpacetimeDb::Types::uint128_t_placeholder val;
            val.low = read_u64_le();  // Assuming little-endian: lower part first
            val.high = read_u64_le(); // Then higher part
            return val;
        }

        int8_t Reader::read_i8() {
            ensure_bytes(1);
            int8_t val = static_cast<int8_t>(*current_ptr);
            current_ptr++;
            return val;
        }

        int16_t Reader::read_i16_le() {
            ensure_bytes(sizeof(int16_t));
            return ::read_le_bytes<int16_t>(current_ptr, end_ptr, sizeof(int16_t));
        }

        int32_t Reader::read_i32_le() {
            ensure_bytes(sizeof(int32_t));
            return ::read_le_bytes<int32_t>(current_ptr, end_ptr, sizeof(int32_t));
        }

        int64_t Reader::read_i64_le() {
            ensure_bytes(sizeof(int64_t));
            return ::read_le_bytes<int64_t>(current_ptr, end_ptr, sizeof(int64_t));
        }

        SpacetimeDb::Types::int128_t_placeholder Reader::read_i128_le() {
            SpacetimeDb::Types::int128_t_placeholder val;
            val.low = read_u64_le();  // Lower part as uint64_t
            val.high = read_i64_le(); // Higher part as int64_t for sign
            return val;
        }

        float Reader::read_f32_le() {
            union {
                uint32_t u;
                float f;
            } pun;
            pun.u = read_u32_le(); // This already calls ensure_bytes for uint32_t
            return pun.f;
        }

        double Reader::read_f64_le() {
            union {
                uint64_t u;
                double d;
            } pun;
            pun.u = read_u64_le(); // This already calls ensure_bytes for uint64_t
            return pun.d;
        }

        std::string Reader::read_string() {
            uint32_t len = read_u32_le(); // This ensures bytes for length
            if (len > max_string_length_sanity_check) { // max_string_length_sanity_check is static const in Reader class
                throw std::runtime_error(
                    "BSATN Reader: String length " + std::to_string(len) +
                    " exceeds sanity limit " + std::to_string(max_string_length_sanity_check));
            }
            ensure_bytes(len); // Ensure bytes for string content
            std::string str(reinterpret_cast<const char*>(current_ptr), len);
            current_ptr += len;
            return str;
        }

        std::vector<std::byte> Reader::read_bytes() {
            uint32_t len = read_u32_le(); // This ensures bytes for length
            if (len > max_vector_elements_sanity_check) { // A bit arbitrary, but check total bytes. max_vector_elements_sanity_check is static const.
                throw std::runtime_error(
                    "BSATN Reader: Byte vector length " + std::to_string(len) +
                    " exceeds sanity limit for elements (treat as bytes here).");
            }
            ensure_bytes(len); // Ensure bytes for vector content
            std::vector<std::byte> bytes_vec(current_ptr, current_ptr + len); // Renamed to avoid conflict
            current_ptr += len;
            return bytes_vec;
        }

        std::vector<std::byte> Reader::read_fixed_bytes(size_t count) {
            ensure_bytes(count);
            std::vector<std::byte> bytes_vec(current_ptr, current_ptr + count);
            current_ptr += count;
            return bytes_vec;
        }

        std::vector<std::byte> Reader::read_vector_byte() {
            return read_bytes(); // Same implementation
        }


        bool Reader::is_eos() const {
            return current_ptr >= end_ptr;
        }

        size_t Reader::remaining_bytes() const {
            if (current_ptr >= end_ptr) return 0;
            return static_cast<size_t>(end_ptr - current_ptr);
        }

        // The generic bsatn::deserialize<T> template and its specializations
        // are expected to be in headers (e.g., bsatn_reader.h for the primary template,
        // generated type headers for user type specializations, and bsatn_lib.h or similar for primitives).
        // No implementation for it goes in reader.cpp itself, unless it's a non-template helper.

    }
} // namespace SpacetimeDb::bsatn
