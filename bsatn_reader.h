#ifndef BSATN_READER_H
#define BSATN_READER_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept> // For std::runtime_error
#include <optional>
#include <functional>
#include <span> // For std::span (C++20)
#include <type_traits> // For std::is_enum
#include "uint128_placeholder.h" // For uint128_t_placeholder etc.

// Forward declaration for user-defined types' deserialize functions
// E.g. MyStruct deserialize_MyStruct(bsatn::Reader& reader);

namespace bsatn {

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

    template<typename T, typename Func>
    std::optional<T> read_optional(Func read_func) {
        uint8_t tag = read_u8();
        if (tag == 0) {
            return std::nullopt;
        } else if (tag == 1) { // Assuming 1 means present
            return read_func(*this);
        }
        // Potentially handle other tag values or throw an error for unexpected tags
        throw std::runtime_error("Invalid tag for optional type: " + std::to_string(tag));
    }

    template<typename T, typename Func>
    std::vector<T> read_vector(Func read_func) {
        uint32_t count = read_u32_le();
        std::vector<T> vec;
        if (count > max_vector_elements_sanity_check) { // Protection against huge allocations
             throw std::runtime_error("Vector element count " + std::to_string(count) + " exceeds sanity limit " + std::to_string(max_vector_elements_sanity_check));
        }
        vec.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            vec.push_back(read_func(*this));
        }
        return vec;
    }

    // Specific version for vector of bytes
    std::vector<std::byte> read_vector_byte();

    // Generic deserialize call for user-defined types or types with global deserialize functions
    template<typename T>
    T deserialize() {
         // If T is an enum class, read u8 and cast
        if constexpr (std::is_enum_v<T>) {
            // Assuming underlying type is uint8_t. Add check or make more generic if needed.
            return static_cast<T>(read_u8());
        } else {
            // For other types, rely on a global/namespace-level deserialize function
            // This function must be declared as:
            // YourType deserialize_YourType(bsatn::Reader& reader);
            // And needs to be found via ADL or be in global/bsatn namespace.
            // This is a common pattern for C++ serialization libraries.
            // This will call, e.g. deserialize_MyStruct(*this).
            return deserialize_adl_helper<T>(*this);
        }
    }


    bool is_eos() const; // End Of Stream
    size_t remaining_bytes() const;
    void ensure_bytes(size_t count); // Throws if not enough bytes remain

private:
    // Helper for ADL to find deserialize_TypeName functions
    template<typename T>
    static T deserialize_adl_helper(bsatn::Reader& r) {
        // This relies on the user providing deserialize_TypeName(Reader&) functions
        // in the same namespace as TypeName, or globally.
        return deserialize_TypeName(r); // This won't compile directly, placeholder for concept
                                        // Actual solution is usually to have deserialize<T> specializations
                                        // or have generated code call deserialize_TypeName directly.
                                        // For this subtask, we assume generated code will call deserialize_TypeName.
                                        // This generic deserialize<T>() is for convenience.
                                        // To make it work, we'd need to specialize it for each T.
                                        // Or, each type T needs a static member T deserialize(Reader&).
                                        // For now, this is a conceptual placeholder.
                                        // The template functions in bsatn_lib.h (codegen) handle this better
                                        // by calling a generic `deserialize<T>` which is then specialized.
        // The macros will generate `deserialize_MyType(reader)` so this specific function is less critical
        // for macro-generated code but useful for manual deserialization.
        // Let's remove this complex ADL helper for now, as the macros will generate specific calls.
        // The read_optional and read_vector will take function pointers to these specific deserialize_TypeName calls.
        static_assert(sizeof(T) == 0, "Generic bsatn::Reader::deserialize<T> not implemented. Use specific read_xxx or generated deserialize_TypeName.");
        return T{}; // Should not be reached
    }


    const std::byte* current_ptr = nullptr;
    const std::byte* end_ptr = nullptr;

    // To prevent excessive allocations from malformed/malicious data
    static const uint32_t max_string_length_sanity_check = 1024 * 1024 * 10; // 10MB
    static const uint32_t max_vector_elements_sanity_check = 1024 * 1024; // 1M elements
};

} // namespace bsatn

#endif // BSATN_READER_H
