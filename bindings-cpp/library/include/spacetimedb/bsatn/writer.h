#ifndef SPACETIMEDB_BSATN_WRITER_H
#define SPACETIMEDB_BSATN_WRITER_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <functional>
#include <type_traits>
#include <variant>
#include "uint128_placeholder.h"
#include "spacetimedb/types.h"

namespace SpacetimeDb::bsatn {

    // Forward declarations
    template<typename T> struct bsatn_traits;
    template<typename T> void serialize(class Writer& w, const T& value);

    class Writer {
    private:
        std::vector<uint8_t>* buffer_;
        
    public:
        Writer() : buffer_(nullptr) {}
        explicit Writer(std::vector<uint8_t>& buffer) : buffer_(&buffer) {}

        void write_bool(bool value);
        void write_u8(uint8_t value);
        void write_u16_le(uint16_t value);
        void write_u32_le(uint32_t value);
        void write_u64_le(uint64_t value);
        void write_u128_le(const SpacetimeDb::Types::uint128_t_placeholder& value);
        void write_u256_le(const SpacetimeDb::u256_placeholder& value) {
            write_bytes_raw(value.data.data(), value.data.size());
        }

        void write_i8(int8_t value);
        void write_i16_le(int16_t value);
        void write_i32_le(int32_t value);
        void write_i64_le(int64_t value);
        void write_i128_le(const SpacetimeDb::Types::int128_t_placeholder& value);
        void write_i256_le(const SpacetimeDb::i256_placeholder& value) {
            write_bytes_raw(value.data.data(), value.data.size());
        }

        void write_f32_le(float value);
        void write_f64_le(double value);

        void write_string(const std::string& value);
        void write_bytes(const std::vector<uint8_t>& value);

        template<typename T>
        void write_optional(const std::optional<T>& opt_value) {
            if (opt_value.has_value()) {
                write_u8(1);
                SpacetimeDb::bsatn::serialize(*this, *opt_value);
            }
            else {
                write_u8(0);
            }
        }

        template<typename T>
        void write_vector(const std::vector<T>& vec) {
            write_u32_le(static_cast<uint32_t>(vec.size()));
            for (const auto& item : vec) {
                SpacetimeDb::bsatn::serialize(*this, item);
            }
        }

        void write_vector_byte(const std::vector<uint8_t>& vec);

        // Generic serialize member function
        template<typename T>
        void serialize_member(const T& value) {
            SpacetimeDb::bsatn::serialize(*this, value);
        }
        
        void write_vec_len(size_t len) {
            // Write length as varint
            write_u32_le(static_cast<uint32_t>(len));
        }

        const std::vector<uint8_t>& get_buffer() const { 
            return buffer_ ? *buffer_ : buffer; 
        }
        
        std::vector<uint8_t>&& take_buffer() { 
            return std::move(buffer_ ? *buffer_ : buffer); 
        }

    private:
        void write_bytes_raw(const void* data, size_t size);
        std::vector<uint8_t> buffer;
    };

    // Helper to detect if type has bsatn_serialize method
    template<typename T, typename = void>
    struct has_bsatn_serialize : std::false_type {};
    
    template<typename T>
    struct has_bsatn_serialize<T, std::void_t<decltype(std::declval<const T&>().bsatn_serialize(std::declval<Writer&>()))>> 
        : std::true_type {};

    // Generic serialize implementation for types with bsatn_serialize
    template<typename T>
    inline void serialize(Writer& w, const T& value) {
        if constexpr (has_bsatn_serialize<T>::value) {
            value.bsatn_serialize(w);
        } else {
            // Fall back to bsatn_traits
            bsatn_traits<T>::serialize(w, value);
        }
    }

    // Inline implementations of serialize functions
    inline void serialize(Writer& w, bool value) {
        w.write_bool(value);
    }

    inline void serialize(Writer& w, uint8_t value) {
        w.write_u8(value);
    }

    inline void serialize(Writer& w, uint16_t value) {
        w.write_u16_le(value);
    }

    inline void serialize(Writer& w, uint32_t value) {
        w.write_u32_le(value);
    }

    inline void serialize(Writer& w, uint64_t value) {
        w.write_u64_le(value);
    }

    inline void serialize(Writer& w, const SpacetimeDb::Types::uint128_t_placeholder& value) {
        w.write_u128_le(value);
    }

    inline void serialize(Writer& w, const SpacetimeDb::u256_placeholder& value) {
        w.write_u256_le(value);
    }

    inline void serialize(Writer& w, int8_t value) {
        w.write_i8(value);
    }

    inline void serialize(Writer& w, int16_t value) {
        w.write_i16_le(value);
    }

    inline void serialize(Writer& w, int32_t value) {
        w.write_i32_le(value);
    }

    inline void serialize(Writer& w, int64_t value) {
        w.write_i64_le(value);
    }

    inline void serialize(Writer& w, const SpacetimeDb::Types::int128_t_placeholder& value) {
        w.write_i128_le(value);
    }

    inline void serialize(Writer& w, const SpacetimeDb::i256_placeholder& value) {
        w.write_i256_le(value);
    }

    inline void serialize(Writer& w, float value) {
        w.write_f32_le(value);
    }

    inline void serialize(Writer& w, double value) {
        w.write_f64_le(value);
    }

    inline void serialize(Writer& w, const std::string& value) {
        w.write_string(value);
    }

    inline void serialize(Writer& w, const std::vector<uint8_t>& value) {
        w.write_bytes(value);
    }

    inline void serialize(Writer& w, std::monostate) {
        // Nothing to serialize for monostate
    }

    // SDK type serialization - removed to avoid circular dependency
    // Types with bsatn_serialize methods will be handled by the generic serialize function

    template<typename T>
    inline void serialize(Writer& w, const std::optional<T>& opt_value) {
        w.write_optional(opt_value);
    }

    template<typename T>
    inline void serialize(Writer& w, const std::vector<T>& vec) {
        w.write_vector(vec);
    }

} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_WRITER_H