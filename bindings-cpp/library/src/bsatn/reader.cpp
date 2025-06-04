#include "spacetimedb/bsatn/reader.h"
#include <cstring>

namespace SpacetimeDb::bsatn {

    void Reader::check_available(size_t num_bytes) const {
        if (current_ptr + num_bytes > end_ptr) {
            throw std::runtime_error("BSATN Reader: Not enough bytes remaining.");
        }
    }

    void Reader::advance(size_t num_bytes) {
        current_ptr += num_bytes;
    }

    bool Reader::read_bool() {
        check_available(1);
        uint8_t val = *current_ptr;
        advance(1);
        if (val > 1) {
            throw std::runtime_error("Invalid bool value in BSATN deserialization.");
        }
        return val != 0;
    }

    uint8_t Reader::read_u8() {
        check_available(1);
        uint8_t val = *current_ptr;
        advance(1);
        return val;
    }

    uint16_t Reader::read_u16_le() {
        check_available(2);
        uint16_t val;
        std::memcpy(&val, current_ptr, 2);
        advance(2);
        return val;
    }

    uint32_t Reader::read_u32_le() {
        check_available(4);
        uint32_t val;
        std::memcpy(&val, current_ptr, 4);
        advance(4);
        return val;
    }

    uint64_t Reader::read_u64_le() {
        check_available(8);
        uint64_t val;
        std::memcpy(&val, current_ptr, 8);
        advance(8);
        return val;
    }

    SpacetimeDb::Types::uint128_t_placeholder Reader::read_u128_le() {
        uint64_t low = read_u64_le();
        uint64_t high = read_u64_le();
        return SpacetimeDb::Types::uint128_t_placeholder(low, high);
    }

    SpacetimeDb::library::u256_placeholder Reader::read_u256_le() {
        check_available(32);
        SpacetimeDb::library::u256_placeholder val;
        std::memcpy(val.data.data(), current_ptr, 32);
        advance(32);
        return val;
    }

    int8_t Reader::read_i8() {
        return static_cast<int8_t>(read_u8());
    }

    int16_t Reader::read_i16_le() {
        return static_cast<int16_t>(read_u16_le());
    }

    int32_t Reader::read_i32_le() {
        return static_cast<int32_t>(read_u32_le());
    }

    int64_t Reader::read_i64_le() {
        return static_cast<int64_t>(read_u64_le());
    }

    SpacetimeDb::Types::int128_t_placeholder Reader::read_i128_le() {
        uint64_t low = read_u64_le();
        int64_t high = static_cast<int64_t>(read_u64_le());
        return SpacetimeDb::Types::int128_t_placeholder(low, high);
    }

    SpacetimeDb::library::i256_placeholder Reader::read_i256_le() {
        check_available(32);
        SpacetimeDb::library::i256_placeholder val;
        std::memcpy(val.data.data(), current_ptr, 32);
        advance(32);
        return val;
    }

    float Reader::read_f32_le() {
        uint32_t bits = read_u32_le();
        float val;
        std::memcpy(&val, &bits, 4);
        return val;
    }

    double Reader::read_f64_le() {
        uint64_t bits = read_u64_le();
        double val;
        std::memcpy(&val, &bits, 8);
        return val;
    }

    std::string Reader::read_string() {
        uint32_t len = read_u32_le();
        check_available(len);
        std::string result(reinterpret_cast<const char*>(current_ptr), len);
        advance(len);
        return result;
    }

    std::vector<uint8_t> Reader::read_bytes() {
        uint32_t len = read_u32_le();
        check_available(len);
        std::vector<uint8_t> result(current_ptr, current_ptr + len);
        advance(len);
        return result;
    }

    std::vector<uint8_t> Reader::read_fixed_bytes(size_t count) {
        check_available(count);
        std::vector<uint8_t> result(current_ptr, current_ptr + count);
        advance(count);
        return result;
    }


    std::vector<uint8_t> Reader::read_vector_byte() {
        return read_bytes();
    }

    bool Reader::is_eos() const {
        return current_ptr >= end_ptr;
    }

    size_t Reader::remaining_bytes() const {
        return (current_ptr <= end_ptr) ? (end_ptr - current_ptr) : 0;
    }

} // namespace SpacetimeDb::bsatn