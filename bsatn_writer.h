#ifndef BSATN_WRITER_H
#define BSATN_WRITER_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept> // For std::runtime_error
#include <optional>
#include <functional>
#include <type_traits> // For std::is_enum
#include "uint128_placeholder.h" // For uint128_t_placeholder etc.

// Forward declaration for user-defined types' serialize functions
// E.g. void serialize(bsatn::Writer& writer, const MyStruct& value);

namespace bsatn {

class Writer {
public:
    Writer() = default;

    void write_bool(bool value);
    void write_u8(uint8_t value);
    void write_u16_le(uint16_t value);
    void write_u32_le(uint32_t value);
    void write_u64_le(uint64_t value);
    void write_u128_le(const SpacetimeDB::Types::uint128_t_placeholder& value);

    void write_i8(int8_t value);
    void write_i16_le(int16_t value);
    void write_i32_le(int32_t value);
    void write_i64_le(int64_t value);
    void write_i128_le(const SpacetimeDB::Types::int128_t_placeholder& value);

    void write_f32_le(float value);
    void write_f64_le(double value);

    void write_string(const std::string& value);
    void write_bytes(const std::vector<std::byte>& value);

    template<typename T, typename Func>
    void write_optional(const std::optional<T>& opt_value, Func write_func) {
        if (opt_value.has_value()) {
            write_u8(1); // Some tag or just 1 for present
            write_func(*this, *opt_value);
        } else {
            write_u8(0); // None tag or 0 for not present
        }
    }

    // Overload for std::nullptr_t to handle optional empty structs or similar if needed
    // void write_optional(const std::optional<std::nullptr_t>& /*opt_value*/);


    template<typename T, typename Func>
    void write_vector(const std::vector<T>& vec, Func write_func) {
        write_u32_le(static_cast<uint32_t>(vec.size()));
        for (const auto& item : vec) {
            write_func(*this, item);
        }
    }

    // Specific version for vector of bytes, as it's a primitive BSATN type
    void write_vector_byte(const std::vector<std::byte>& vec);


    // Generic serialize call for user-defined types or types with global serialize functions
    // This relies on Argument-Dependent Lookup (ADL) or global `serialize` overloads.
    template<typename T>
    void serialize(const T& value) {
        // If T is an enum class, cast it to its underlying type (assumed u8 for now)
        if constexpr (std::is_enum_v<T>) {
            write_u8(static_cast<uint8_t>(value));
        } else {
            // For other types, rely on a global/namespace-level serialize function
            // This function must be declared as:
            // void serialize(bsatn::Writer& writer, const YourType& value);
            // And needs to be found via ADL or be in global/bsatn namespace.
            // This is a common pattern for C++ serialization libraries.
            // For direct calls for primitives within generated struct serializers, this won't be used.
            // This is more for users calling serialize on a top-level object.
            serialize(*this, value); // Note: This calls a free function `serialize`, not a member
        }
    }


    const std::vector<std::byte>& get_buffer() const;
    std::vector<std::byte>&& take_buffer(); // Move semantics

private:
    void write_bytes_raw(const void* data, size_t size);
    std::vector<std::byte> buffer;
};

} // namespace bsatn

#endif // BSATN_WRITER_H
