#ifndef SPACETIMEDB_BSATN_READER_H // Renamed header guard
#define SPACETIMEDB_BSATN_READER_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept> // For std::runtime_error
#include <optional>  // Ensure std::optional is available
#include <functional>
#include <span> // For std::span (C++20)
#include <type_traits> // For std::is_enum, std::is_same_v, std::true_type, std::false_type
#include <cstring> // For memcpy
#include "uint128_placeholder.h" // Assumes this is in the same directory or accessible via include paths
#include "spacetimedb/sdk/spacetimedb_sdk_types.h" // For u256_placeholder, i256_placeholder

// Forward declaration for user-defined types' deserialize functions
// E.g. MyStruct deserialize_MyStruct(bsatn::Reader& reader);
// This is now typically handled by template specializations of bsatn::deserialize<T>

namespace SpacetimeDb::bsatn {

    // Forward declaration of Reader class
    class Reader;

    // Forward declare the generic deserialize template used by Reader's methods
    template<typename T> T deserialize(Reader& r);

    // Helper trait to check if a type is std::optional
    template<typename> struct is_std_optional : std::false_type {};
    template<typename T> struct is_std_optional<std::optional<T>> : std::true_type {};
    template<typename T> constexpr bool is_std_optional_v = is_std_optional<T>::value;

    // Helper trait to check if a type is std::vector
    template<typename> struct is_std_vector : std::false_type {};
    template<typename T> struct is_std_vector<std::vector<T>> : std::true_type {};
    template<typename T> constexpr bool is_std_vector_v = is_std_vector<T>::value;

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
        SpacetimeDb::Types::uint128_t_placeholder read_u128_le(); // Corrected namespace
        SpacetimeDb::sdk::u256_placeholder read_u256_le(); // Declaration

        int8_t read_i8();
        int16_t read_i16_le();
        int32_t read_i32_le();
        int64_t read_i64_le();
        SpacetimeDb::Types::int128_t_placeholder read_i128_le(); // Corrected namespace
        SpacetimeDb::sdk::i256_placeholder read_i256_le(); // Declaration

        float read_f32_le();
        double read_f64_le();

        std::string read_string();
        std::vector<std::byte> read_bytes(); // Reads a length-prefixed byte array
        std::vector<std::byte> read_fixed_bytes(size_t count); // Reads a fixed number of bytes

        template<typename T>
        std::optional<T> read_optional() {
            // This function will now correctly use the modified generic deserialize below
            // which handles std::optional<T> passed to read_vector, for example.
            // Or, if T is not optional, it will call deserialize<T> which might then call
            // the optional handling logic if T itself is std::optional<U>.
            uint8_t tag = read_u8();
            if (tag == 0) {
                return std::nullopt;
            }
            else if (tag == 1) {
                return deserialize<T>(*this);
            }
            throw std::runtime_error("Invalid tag for optional type in Reader::read_optional(): " + std::to_string(tag));
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
                vec.push_back(deserialize<T>(*this)); // This deserialize call is the key
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
    }; // End of class Reader definition

    // Moved inline definitions for 256-bit types
    inline SpacetimeDb::sdk::u256_placeholder Reader::read_u256_le() {
        SpacetimeDb::sdk::u256_placeholder val;
        ensure_bytes(sizeof(val.data)); // Assuming data is std::array<uint64_t, 4>
        memcpy(val.data.data(), current_ptr, sizeof(val.data));
        current_ptr += sizeof(val.data);
        // TODO: Handle endianness if necessary for each uint64_t component
        return val;
    }

    inline SpacetimeDb::sdk::i256_placeholder Reader::read_i256_le() {
        SpacetimeDb::sdk::i256_placeholder val;
        ensure_bytes(sizeof(val.data));
        memcpy(val.data.data(), current_ptr, sizeof(val.data));
        current_ptr += sizeof(val.data);
        // TODO: Handle endianness
        return val;
    }

    // Helper to give better compile errors if no specialization is found (optional)
    // This must be declared before its use in the generic deserialize template.
    template<typename T> T deserialize_specialized(Reader& r);
    
    // Forward declaration for user-defined types
    // User structs will define this as a non-template overload
    template<typename T>
    inline T deserialize_user_defined(Reader& r, T*) {
        // This will be called only if no specific overload exists
        static_assert(sizeof(T) == 0, "No deserialize function found for this type");
        return T{};
    }


    // Generic deserialize template - only handles special cases
    // All other types must have explicit specializations
    template<typename T>
    T deserialize(Reader& r) {
        if constexpr (std::is_enum_v<T>) {
            return static_cast<T>(r.read_u8()); // Assumes underlying type is compatible with u8 or cast is valid
        }
        else if constexpr (is_std_optional_v<T>) {
            using InnerType = typename T::value_type; // T is std::optional<InnerType>
            uint8_t tag = r.read_u8();
            if (tag == 0) {
                return T(std::nullopt); // Construct std::optional<InnerType> from nullopt
            }
            else if (tag == 1) {
                return T(deserialize<InnerType>(r)); // Construct std::optional<InnerType> from deserialized InnerType
            }
            throw std::runtime_error("Invalid tag for optional type in deserialize: " + std::to_string(tag));
        }
        else if constexpr (is_std_vector_v<T>) {
            using InnerType = typename T::value_type;
            return r.read_vector<InnerType>();
        }
        else {
            // Try ADL lookup for spacetimedb_deserialize function
            return spacetimedb_deserialize(r, static_cast<T*>(nullptr));
        }
    }

    // Definition for deserialize_specialized (if needed, or could be specialized elsewhere)
    // For now, this remains a declaration as in the original file.
    // template<typename T> T deserialize_specialized(Reader& r) {
    //    static_assert(sizeof(T) == 0, "Missing specialization for bsatn::deserialize_specialized or direct bsatn::deserialize overload.");
    //    return T{}; // Should not be reached if static_assert works.
    // }

    // Explicit specializations for primitive types
    template<> inline bool deserialize<bool>(Reader& r) { return r.read_bool(); }
    template<> inline uint8_t deserialize<uint8_t>(Reader& r) { return r.read_u8(); }
    template<> inline uint16_t deserialize<uint16_t>(Reader& r) { return r.read_u16_le(); }
    template<> inline uint32_t deserialize<uint32_t>(Reader& r) { return r.read_u32_le(); }
    template<> inline uint64_t deserialize<uint64_t>(Reader& r) { return r.read_u64_le(); }
    template<> inline SpacetimeDb::Types::uint128_t_placeholder deserialize<SpacetimeDb::Types::uint128_t_placeholder>(Reader& r) { return r.read_u128_le(); }
    template<> inline SpacetimeDb::sdk::u256_placeholder deserialize<SpacetimeDb::sdk::u256_placeholder>(Reader& r) { return r.read_u256_le(); }
    template<> inline int8_t deserialize<int8_t>(Reader& r) { return r.read_i8(); }
    template<> inline int16_t deserialize<int16_t>(Reader& r) { return r.read_i16_le(); }
    template<> inline int32_t deserialize<int32_t>(Reader& r) { return r.read_i32_le(); }
    template<> inline int64_t deserialize<int64_t>(Reader& r) { return r.read_i64_le(); }
    template<> inline SpacetimeDb::Types::int128_t_placeholder deserialize<SpacetimeDb::Types::int128_t_placeholder>(Reader& r) { return r.read_i128_le(); }
    template<> inline SpacetimeDb::sdk::i256_placeholder deserialize<SpacetimeDb::sdk::i256_placeholder>(Reader& r) { return r.read_i256_le(); }
    template<> inline float deserialize<float>(Reader& r) { return r.read_f32_le(); }
    template<> inline double deserialize<double>(Reader& r) { return r.read_f64_le(); }
    template<> inline std::string deserialize<std::string>(Reader& r) { return r.read_string(); }
    template<> inline std::vector<std::byte> deserialize<std::vector<std::byte>>(Reader& r) { return r.read_bytes(); }
    
    // Specializations for SDK types
    template<> inline SpacetimeDb::sdk::Identity deserialize<SpacetimeDb::sdk::Identity>(Reader& r) { 
        SpacetimeDb::sdk::Identity val;
        val.bsatn_deserialize(r);
        return val;
    }
    template<> inline SpacetimeDb::sdk::ConnectionId deserialize<SpacetimeDb::sdk::ConnectionId>(Reader& r) { 
        SpacetimeDb::sdk::ConnectionId val;
        val.bsatn_deserialize(r);
        return val;
    }
    template<> inline SpacetimeDb::sdk::Timestamp deserialize<SpacetimeDb::sdk::Timestamp>(Reader& r) { 
        SpacetimeDb::sdk::Timestamp val;
        val.bsatn_deserialize(r);
        return val;
    }
    template<> inline SpacetimeDb::sdk::TimeDuration deserialize<SpacetimeDb::sdk::TimeDuration>(Reader& r) { 
        SpacetimeDb::sdk::TimeDuration val;
        val.bsatn_deserialize(r);
        return val;
    }
    template<> inline SpacetimeDb::sdk::ScheduleAt deserialize<SpacetimeDb::sdk::ScheduleAt>(Reader& r) { 
        SpacetimeDb::sdk::ScheduleAt val;
        val.bsatn_deserialize(r);
        return val;
    }


} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_READER_H
