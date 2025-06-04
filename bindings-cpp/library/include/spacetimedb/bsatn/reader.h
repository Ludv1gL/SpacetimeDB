#ifndef SPACETIMEDB_BSATN_READER_H
#define SPACETIMEDB_BSATN_READER_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <functional>
#if __cplusplus >= 202002L
#include <span>
#endif
#include <type_traits>
#include <cstring>
#include <variant>
#include "uint128_placeholder.h"
#include "spacetimedb/library/spacetimedb_library_types.h"

namespace SpacetimeDb::bsatn {

    class Reader;
    template<typename T> T deserialize(Reader& r);
    template<typename T> struct bsatn_traits;

    // Helper traits
    template<typename> struct is_std_optional : std::false_type {};
    template<typename T> struct is_std_optional<std::optional<T>> : std::true_type {};
    template<typename T> constexpr bool is_std_optional_v = is_std_optional<T>::value;

    template<typename> struct is_std_vector : std::false_type {};
    template<typename T> struct is_std_vector<std::vector<T>> : std::true_type {};
    template<typename T> constexpr bool is_std_vector_v = is_std_vector<T>::value;

    class Reader {
    public:
        // Constructors - using uint8_t consistently
        Reader(const uint8_t* data, size_t size) : current_ptr(data), end_ptr(data + size) {}
#if __cplusplus >= 202002L
        Reader(std::span<const uint8_t> data) : current_ptr(data.data()), end_ptr(data.data() + data.size()) {}
#endif
        Reader(const std::vector<uint8_t>& data) : current_ptr(data.data()), end_ptr(data.data() + data.size()) {}
        

        bool read_bool();
        uint8_t read_u8();
        uint16_t read_u16_le();
        uint32_t read_u32_le();
        uint64_t read_u64_le();
        SpacetimeDb::Types::uint128_t_placeholder read_u128_le();
        SpacetimeDb::library::u256_placeholder read_u256_le();

        int8_t read_i8();
        int16_t read_i16_le();
        int32_t read_i32_le();
        int64_t read_i64_le();
        SpacetimeDb::Types::int128_t_placeholder read_i128_le();
        SpacetimeDb::library::i256_placeholder read_i256_le();

        float read_f32_le();
        double read_f64_le();

        std::string read_string();
        std::vector<uint8_t> read_bytes();
        std::vector<uint8_t> read_fixed_bytes(size_t count);

        template<typename T>
        std::optional<T> read_optional() {
            uint8_t tag = read_u8();
            if (tag == 0) {
                return std::nullopt;
            } else if (tag == 1) {
                return SpacetimeDb::bsatn::deserialize<T>(*this);
            } else {
                throw std::runtime_error("Invalid optional tag in BSATN deserialization.");
            }
        }

        template<typename T>
        std::vector<T> read_vector() {
            uint32_t size = read_u32_le();
            std::vector<T> result;
            result.reserve(size);
            for (uint32_t i = 0; i < size; ++i) {
                result.push_back(SpacetimeDb::bsatn::deserialize<T>(*this));
            }
            return result;
        }

        std::vector<uint8_t> read_vector_byte();

        template<typename T>
        T deserialize_type() {
            return SpacetimeDb::bsatn::deserialize<T>(*this);
        }

        bool is_eos() const;
        size_t remaining_bytes() const;

    private:
        void check_available(size_t num_bytes) const;
        void advance(size_t num_bytes);

        const uint8_t* current_ptr;
        const uint8_t* end_ptr;
    };

    // Generic deserialize function implementation
    template<typename T>
    inline T deserialize(Reader& r) {
        if constexpr (std::is_same_v<T, bool>) {
            return r.read_bool();
        } else if constexpr (std::is_same_v<T, uint8_t>) {
            return r.read_u8();
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            return r.read_u16_le();
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return r.read_u32_le();
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return r.read_u64_le();
        } else if constexpr (std::is_same_v<T, int8_t>) {
            return r.read_i8();
        } else if constexpr (std::is_same_v<T, int16_t>) {
            return r.read_i16_le();
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return r.read_i32_le();
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return r.read_i64_le();
        } else if constexpr (std::is_same_v<T, float>) {
            return r.read_f32_le();
        } else if constexpr (std::is_same_v<T, double>) {
            return r.read_f64_le();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return r.read_string();
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            return r.read_bytes();
        } else if constexpr (is_std_optional_v<T>) {
            return r.read_optional<typename T::value_type>();
        } else if constexpr (is_std_vector_v<T>) {
            return r.read_vector<typename T::value_type>();
        } else if constexpr (std::is_same_v<T, SpacetimeDb::library::Identity>) {
            SpacetimeDb::library::Identity result;
            result.bsatn_deserialize(r);
            return result;
        } else if constexpr (std::is_same_v<T, SpacetimeDb::library::Timestamp>) {
            SpacetimeDb::library::Timestamp result;
            result.bsatn_deserialize(r);
            return result;
        } else if constexpr (std::is_same_v<T, SpacetimeDb::library::ConnectionId>) {
            SpacetimeDb::library::ConnectionId result;
            result.bsatn_deserialize(r);
            return result;
        } else if constexpr (std::is_same_v<T, SpacetimeDb::library::TimeDuration>) {
            SpacetimeDb::library::TimeDuration result;
            result.bsatn_deserialize(r);
            return result;
        } else if constexpr (std::is_same_v<T, SpacetimeDb::library::ScheduleAt>) {
            SpacetimeDb::library::ScheduleAt result;
            result.bsatn_deserialize(r);
            return result;
        } else {
            // For other types, try to use bsatn_traits
            return bsatn_traits<T>::deserialize(r);
        }
    }

} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_READER_H