#include "bsatn_writer.h"
#include <cstring> // For std::memcpy
#include <limits>  // For std::numeric_limits
#include <algorithm> // For std::reverse if needed for endianness on some platforms

// Helper for endian conversion if system is big-endian.
// BSATN is little-endian. Most common platforms (x86, ARM) are little-endian.
// For simplicity, this implementation assumes little-endian system or that
// types like uint32_t are handled correctly by direct memory copy.
// A robust solution would use compile-time checks or runtime detection for endianness.
namespace {
template<typename T>
void append_le_bytes(std::vector<std::byte>& buffer, T value) {
    // On a little-endian system, this is a direct copy.
    // On a big-endian system, bytes of `value` would need to be reversed.
    // For now, assuming little-endian for simplicity of example.
    const std::byte* bytes = reinterpret_cast<const std::byte*>(&value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
}
} // anonymous namespace

namespace bsatn {

void Writer::write_bytes_raw(const void* data, size_t size) {
    if (size == 0) return;
    const std::byte* bytes = static_cast<const std::byte*>(data);
    buffer.insert(buffer.end(), bytes, bytes + size);
}

void Writer::write_bool(bool value) {
    buffer.push_back(static_cast<std::byte>(value ? 1 : 0));
}

void Writer::write_u8(uint8_t value) {
    buffer.push_back(static_cast<std::byte>(value));
}

void Writer::write_u16_le(uint16_t value) {
    append_le_bytes(buffer, value);
}

void Writer::write_u32_le(uint32_t value) {
    append_le_bytes(buffer, value);
}

void Writer::write_u64_le(uint64_t value) {
    append_le_bytes(buffer, value);
}

void Writer::write_u128_le(const SpacetimeDB::Types::uint128_t_placeholder& value) {
    write_u64_le(value.low);  // Assuming little-endian: lower part first
    write_u64_le(value.high); // Then higher part
}

void Writer::write_i8(int8_t value) {
    buffer.push_back(static_cast<std::byte>(value));
}

void Writer::write_i16_le(int16_t value) {
    append_le_bytes(buffer, value);
}

void Writer::write_i32_le(int32_t value) {
    append_le_bytes(buffer, value);
}

void Writer::write_i64_le(int64_t value) {
    append_le_bytes(buffer, value);
}

void Writer::write_i128_le(const SpacetimeDB::Types::int128_t_placeholder& value) {
    write_u64_le(value.low);  // Write lower part as uint64_t
    write_i64_le(value.high); // Write higher part as int64_t to preserve sign representation
}

void Writer::write_f32_le(float value) {
    // Union trick for type punning, common but technically UB in some interpretations.
    // std::bit_cast (C++20) is preferred if available.
    // For BSATN, it's about the byte representation being consistent.
    union {
        float f;
        uint32_t u;
    } pun = { .f = value };
    write_u32_le(pun.u);
}

void Writer::write_f64_le(double value) {
    union {
        double d;
        uint64_t u;
    } pun = { .d = value };
    write_u64_le(pun.u);
}

void Writer::write_string(const std::string& value) {
    if (value.length() > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("String length exceeds uint32_t max");
    }
    write_u32_le(static_cast<uint32_t>(value.length()));
    write_bytes_raw(value.data(), value.length());
}

void Writer::write_bytes(const std::vector<std::byte>& value) {
     if (value.size() > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("Byte vector size exceeds uint32_t max");
    }
    write_u32_le(static_cast<uint32_t>(value.size()));
    write_bytes_raw(value.data(), value.size());
}

void Writer::write_vector_byte(const std::vector<std::byte>& vec) {
    // This is identical to write_bytes for std::vector<std::byte>
    write_bytes(vec);
}


const std::vector<std::byte>& Writer::get_buffer() const {
    return buffer;
}

std::vector<std::byte>&& Writer::take_buffer() {
    return std::move(buffer);
}

} // namespace bsatn
