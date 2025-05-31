#include "bsatn_reader.h"
#include <cstring>   // For std::memcpy
#include <stdexcept> // For std::runtime_error, std::out_of_range
#include <limits>    // For std::numeric_limits

// Helper for endian conversion if system is big-endian.
// For simplicity, this implementation assumes little-endian system.
namespace {
template<typename T>
T read_le_bytes(const std::byte*& current_ptr, const std::byte* end_ptr) {
    if (static_cast<size_t>(end_ptr - current_ptr) < sizeof(T)) {
        throw std::out_of_range("BSATN Reader: Not enough bytes to read type.");
    }
    T value;
    // On a little-endian system, this is a direct copy.
    // On a big-endian system, bytes of `value` would need to be reversed after copy.
    std::memcpy(&value, current_ptr, sizeof(T));
    current_ptr += sizeof(T);
    return value;
}
} // anonymous namespace


namespace bsatn {

Reader::Reader(std::span<const std::byte> data_span)
    : current_ptr(data_span.data()), end_ptr(data_span.data() + data_span.size()) {}

Reader::Reader(const std::byte* data_ptr, size_t length)
    : current_ptr(data_ptr), end_ptr(data_ptr + length) {
    if (!data_ptr && length > 0) {
        throw std::invalid_argument("BSATN Reader: Null data_ptr with non-zero length.");
    }
}

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
        // Or, could just return val != 0; depending on strictness desired
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
    return read_le_bytes<uint16_t>(current_ptr, end_ptr);
}

uint32_t Reader::read_u32_le() {
    return read_le_bytes<uint32_t>(current_ptr, end_ptr);
}

uint64_t Reader::read_u64_le() {
    return read_le_bytes<uint64_t>(current_ptr, end_ptr);
}

SpacetimeDB::Types::uint128_t_placeholder Reader::read_u128_le() {
    SpacetimeDB::Types::uint128_t_placeholder val;
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
    return read_le_bytes<int16_t>(current_ptr, end_ptr);
}

int32_t Reader::read_i32_le() {
    return read_le_bytes<int32_t>(current_ptr, end_ptr);
}

int64_t Reader::read_i64_le() {
    return read_le_bytes<int64_t>(current_ptr, end_ptr);
}

SpacetimeDB::Types::int128_t_placeholder Reader::read_i128_le() {
    SpacetimeDB::Types::int128_t_placeholder val;
    val.low = read_u64_le();  // Lower part as uint64_t
    val.high = read_i64_le(); // Higher part as int64_t for sign
    return val;
}

float Reader::read_f32_le() {
    union {
        uint32_t u;
        float f;
    } pun;
    pun.u = read_u32_le();
    return pun.f;
}

double Reader::read_f64_le() {
    union {
        uint64_t u;
        double d;
    } pun;
    pun.u = read_u64_le();
    return pun.d;
}

std::string Reader::read_string() {
    uint32_t len = read_u32_le();
    if (len > max_string_length_sanity_check) {
        throw std::runtime_error(
            "BSATN Reader: String length " + std::to_string(len) +
            " exceeds sanity limit " + std::to_string(max_string_length_sanity_check));
    }
    ensure_bytes(len);
    std::string str(reinterpret_cast<const char*>(current_ptr), len);
    current_ptr += len;
    return str;
}

std::vector<std::byte> Reader::read_bytes() {
    uint32_t len = read_u32_le();
     if (len > max_vector_elements_sanity_check * sizeof(std::byte)) { // A bit arbitrary, but check total bytes
        throw std::runtime_error(
            "BSATN Reader: Byte vector length " + std::to_string(len) +
            " exceeds sanity limit.");
    }
    ensure_bytes(len);
    std::vector<std::byte> bytes(current_ptr, current_ptr + len);
    current_ptr += len;
    return bytes;
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

} // namespace bsatn
