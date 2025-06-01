#ifndef SPACETIMEDB_BSATN_READER_H // Renamed header guard
#define SPACETIMEDB_BSATN_READER_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept> // For std::runtime_error
#include <optional>
#include <functional>
#include <span> // For std::span (C++20)
#include <type_traits> // For std::is_enum
#include "uint128_placeholder.h" // Assumes this is in the same directory or accessible via include paths

// Forward declaration for user-defined types' deserialize functions
// E.g. MyStruct deserialize_MyStruct(bsatn::Reader& reader);
// This is now typically handled by template specializations of bsatn::deserialize<T>

namespace bsatn {

// Forward declare the generic deserialize template used by Reader's methods
template<typename T> T deserialize(Reader& r);

class Reader {
public:
    // Construct with a span (non-owning view)
    explicit Reader(std::span<const std::byte> data_span);
    // Construct with raw pointer and length (non-owning)
    Reader(const std::byte* data_ptr, size_t length);

    bool read_bool();
    uint8_t read_u8();
    uint16_t read_u16_le();
    uint32_t read_u32_le();
    uint64_t read_u64_le();
    SpacetimeDB::Types::uint128_t_placeholder read_u128_le();

    int8_t read_i8();
    int16_t read_i16_le();
    int32_t read_i32_le();
    int64_t read_i64_le();
    SpacetimeDB::Types::int128_t_placeholder read_i128_le();

    float read_f32_le();
    double read_f64_le();

    std::string read_string();
    std::vector<std::byte> read_bytes();

    template<typename T> // Removed Func, will use bsatn::deserialize<T>
    std::optional<T> read_optional() { // Renamed from read_optional(Func)
        uint8_t tag = read_u8();
        if (tag == 0) {
            return std::nullopt;
        } else if (tag == 1) { // Assuming 1 means present
            return bsatn::deserialize<T>(*this); // Use the generic deserialize
        }
        throw std::runtime_error("Invalid tag for optional type: " + std::to_string(tag));
    }

    template<typename T> // Removed Func, will use bsatn::deserialize<T>
    std::vector<T> read_vector() { // Renamed from read_vector(Func)
        uint32_t count = read_u32_le();
        std::vector<T> vec;
        if (count > max_vector_elements_sanity_check) {
             throw std::runtime_error("Vector element count " + std::to_string(count) + " exceeds sanity limit " + std::to_string(max_vector_elements_sanity_check));
        }
        vec.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            vec.push_back(bsatn::deserialize<T>(*this)); // Use the generic deserialize
        }
        return vec;
    }

    std::vector<std::byte> read_vector_byte(); // Specific version for vector of bytes

    // Generic deserialize member function (calls the free template function bsatn::deserialize<T>)
    template<typename T>
    T deserialize_member() { // Renamed to avoid conflict if free function is in global/bsatn
         return bsatn::deserialize<T>(*this);
    }

    bool is_eos() const;
    size_t remaining_bytes() const;
    void ensure_bytes(size_t count);

private:
    const std::byte* current_ptr = nullptr;
    const std::byte* end_ptr = nullptr;

    static const uint32_t max_string_length_sanity_check = 1024 * 1024 * 10;
    static const uint32_t max_vector_elements_sanity_check = 1024 * 1024;
};

// Definition of the generic deserialize template (relies on specializations)
// This should be in bsatn_lib.h from codegen, or a similar central place.
// For now, to make this header self-contained for what it needs:
template<typename T>
T deserialize(Reader& r) {
    // This generic version should ideally static_assert(false, "No bsatn::deserialize specialization for this type T")
    // or rely on specific deserialize_TypeName functions being specialized for bsatn::deserialize<TypeName>.
    // The SDK macros will generate:
    //   TypeName deserialize_TypeName(bsatn::Reader&);
    //   template<> inline TypeName bsatn::deserialize<TypeName>(bsatn::Reader& r) { return deserialize_TypeName(r); }
    // Primitives also get specializations.

    // If T is an enum class, read u8 and cast (this is a common case)
    if constexpr (std::is_enum_v<T>) {
        return static_cast<T>(r.read_u8()); // Assumes underlying type is compatible with u8 or cast is valid
    } else {
        // This will fail to compile if no specialization is found for T, which is good.
        // To provide a better error, a static_assert can be used with a helper trait.
        // For now, rely on linker/compiler errors if a specialization is missing.
        return deserialize_specialized<T>(r); // Placeholder for actual mechanism if not direct specialization
    }
}
// Helper to give better compile errors if no specialization is found (optional)
template<typename T> T deserialize_specialized(Reader& r);

} // namespace bsatn

#endif // SPACETIMEDB_BSATN_READER_H
