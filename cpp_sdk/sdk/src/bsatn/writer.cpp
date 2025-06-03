#include "spacetimedb/bsatn/writer.h"
#include <cstring>

namespace SpacetimeDb::bsatn {

    void Writer::write_bytes_raw(const void* data, size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), bytes, bytes + size);
    }

    void Writer::write_bool(bool value) {
        buffer.push_back(value ? 1 : 0);
    }

    void Writer::write_u8(uint8_t value) {
        buffer.push_back(value);
    }

    void Writer::write_u16_le(uint16_t value) {
        write_bytes_raw(&value, 2);
    }

    void Writer::write_u32_le(uint32_t value) {
        write_bytes_raw(&value, 4);
    }

    void Writer::write_u64_le(uint64_t value) {
        write_bytes_raw(&value, 8);
    }

    void Writer::write_u128_le(const SpacetimeDb::Types::uint128_t_placeholder& value) {
        write_u64_le(value.low);
        write_u64_le(value.high);
    }

    void Writer::write_i8(int8_t value) {
        write_u8(static_cast<uint8_t>(value));
    }

    void Writer::write_i16_le(int16_t value) {
        write_u16_le(static_cast<uint16_t>(value));
    }

    void Writer::write_i32_le(int32_t value) {
        write_u32_le(static_cast<uint32_t>(value));
    }

    void Writer::write_i64_le(int64_t value) {
        write_u64_le(static_cast<uint64_t>(value));
    }

    void Writer::write_i128_le(const SpacetimeDb::Types::int128_t_placeholder& value) {
        write_u64_le(value.low);
        write_u64_le(static_cast<uint64_t>(value.high));
    }

    void Writer::write_f32_le(float value) {
        uint32_t bits;
        std::memcpy(&bits, &value, 4);
        write_u32_le(bits);
    }

    void Writer::write_f64_le(double value) {
        uint64_t bits;
        std::memcpy(&bits, &value, 8);
        write_u64_le(bits);
    }

    void Writer::write_string(const std::string& value) {
        write_u32_le(static_cast<uint32_t>(value.size()));
        write_bytes_raw(value.data(), value.size());
    }

    void Writer::write_bytes(const std::vector<uint8_t>& value) {
        write_u32_le(static_cast<uint32_t>(value.size()));
        write_bytes_raw(value.data(), value.size());
    }

    // Compatibility method for std::byte vectors
    void Writer::write_bytes(const std::vector<std::byte>& value) {
        write_u32_le(static_cast<uint32_t>(value.size()));
        write_bytes_raw(value.data(), value.size());
    }

    void Writer::write_vector_byte(const std::vector<uint8_t>& vec) {
        write_bytes(vec);
    }

    // Compatibility methods
    std::vector<std::byte> Writer::get_buffer_as_std_byte() const {
        std::vector<std::byte> result;
        result.reserve(buffer.size());
        for (uint8_t b : buffer) {
            result.push_back(static_cast<std::byte>(b));
        }
        return result;
    }

    std::vector<std::byte> Writer::take_buffer_as_std_byte() {
        std::vector<std::byte> result;
        result.reserve(buffer.size());
        for (uint8_t b : buffer) {
            result.push_back(static_cast<std::byte>(b));
        }
        buffer.clear();
        return result;
    }


} // namespace SpacetimeDb::bsatn