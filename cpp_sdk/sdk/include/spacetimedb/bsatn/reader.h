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
#include <cstring> // For memcpy
#include "uint128_placeholder.h" // Assumes this is in the same directory or accessible via include paths
#include "spacetimedb/sdk/spacetimedb_sdk_types.h" // For u256_placeholder, i256_placeholder

// Forward declaration for user-defined types' deserialize functions
// E.g. MyStruct deserialize_MyStruct(bsatn::Reader& reader);
// This is now typically handled by template specializations of bsatn::deserialize<T>

namespace bsatn {

    // Forward declaration of Reader class
    class Reader;

    // Forward declare the generic deserialize template used by Reader's methods
    template<typename T> T deserialize(Reader& r);

    class Reader {
    public:
        // Constructors
        Reader(const std::byte* data, size_t size) : current_ptr(data), end_ptr(data + size) {}
        Reader(std::span<const std::byte> data) : current_ptr(data.data()), end_ptr(data.data() + data.size()) {}

        bool read_bool();
        uint8_t read_u8();
        uint16_t read_u16_le();
        uint32_t read_u32_le();
        uint64_t read_u64_le();
        SpacetimeDB::Types::uint128_t_placeholder read_u128_le();
        spacetimedb::sdk::u256_placeholder read_u256_le();

        int8_t read_i8();
        int16_t read_i16_le();
        int32_t read_i32_le();
        int64_t read_i64_le();
        SpacetimeDB::Types::int128_t_placeholder read_i128_le();
        spacetimedb::sdk::i256_placeholder read_i256_le();

        float read_f32_le();
        double read_f64_le();

        std::string read_string();
        std::vector<std::byte> read_bytes();

        template<typename T>
        std::optional<T> read_optional() {
            uint8_t tag = read_u8();
            if (tag == 0) {
                return std::nullopt;
            }
            else if (tag == 1) {
                return deserialize<T>(*this);
            }
            throw std::runtime_error("Invalid tag for optional type: " + std::to_string(tag));
        }

        template<typename T>
        std::vector<T> read_vector() {
            uint32_t count = read_u32_le();
            std::vector<T> vec;
            if (count > max_vector_elements_sanity_check) {
                throw std::runtime_error("Vector element count " + std::to_string(count) + " exceeds sanity limit " + std::to_string(max_vector_elements_sanity_check));
            }
            vec.reserve(count);
            for (uint32_t i = 0; i < count; ++i) {
                vec.push_back(deserialize<T>(*this));
            }
            return vec;
        }

        std::vector<std::byte> read_vector_byte();

        bool is_eos() const;
        size_t remaining_bytes() const;
        void ensure_bytes(size_t count);

    private:
        const std::byte* current_ptr = nullptr;
        const std::byte* end_ptr = nullptr;

        static const uint32_t max_string_length_sanity_check = 1024 * 1024 * 10;
        static const uint32_t max_vector_elements_sanity_check = 1024 * 1024;

    public:
        void ensure_bytes(size_t count) {
            if (static_cast<size_t>(end_ptr - current_ptr) < count) {
                throw std::runtime_error("Attempt to read past end of buffer.");
            }
        }

    public:
        // Inline implementations for 256-bit types
        inline spacetimedb::sdk::u256_placeholder read_u256_le() {
            spacetimedb::sdk::u256_placeholder val;
            ensure_bytes(sizeof(val.data)); // Assuming data is std::array<uint64_t, 4>
            memcpy(val.data.data(), current_ptr, sizeof(val.data));
            current_ptr += sizeof(val.data);
            // TODO: Handle endianness if necessary for each uint64_t component
            return val;
        }

        inline spacetimedb::sdk::i256_placeholder read_i256_le() {
            spacetimedb::sdk::i256_placeholder val;
            ensure_bytes(sizeof(val.data));
            memcpy(val.data.data(), current_ptr, sizeof(val.data));
            current_ptr += sizeof(val.data);
            // TODO: Handle endianness
            return val;
        }
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
        }
        else {
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
