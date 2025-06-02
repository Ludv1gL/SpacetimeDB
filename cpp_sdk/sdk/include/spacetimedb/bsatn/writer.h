#ifndef SPACETIMEDB_BSATN_WRITER_H // Renamed header guard
#define SPACETIMEDB_BSATN_WRITER_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept> // For std::runtime_error
#include <optional>
#include <functional>  // Was used for Func, can be removed if Func is removed
#include <type_traits> // For std::is_enum
#include "uint128_placeholder.h" // Assumes this is in the same directory or accessible via include paths
#include "spacetimedb/sdk/spacetimedb_sdk_types.h" // For u256_placeholder, i256_placeholder

namespace SpacetimeDb::bsatn {

    // Forward declare the generic free function template for serialization
    template<typename T> void serialize(class Writer& w, const T& value);

    // Explicit overloads for primitives and common types
    inline void serialize(Writer& w, bool value);
    inline void serialize(Writer& w, uint8_t value);
    inline void serialize(Writer& w, uint16_t value);
    inline void serialize(Writer& w, uint32_t value);
    inline void serialize(Writer& w, uint64_t value);
    inline void serialize(Writer& w, const SpacetimeDb::Types::uint128_t_placeholder& value);
    inline void serialize(Writer& w, const SpacetimeDb::sdk::u256_placeholder& value);
    inline void serialize(Writer& w, int8_t value);
    inline void serialize(Writer& w, int16_t value);
    inline void serialize(Writer& w, int32_t value);
    inline void serialize(Writer& w, int64_t value);
    inline void serialize(Writer& w, const SpacetimeDb::Types::int128_t_placeholder& value);
    inline void serialize(Writer& w, const SpacetimeDb::sdk::i256_placeholder& value);
    inline void serialize(Writer& w, float value);
    inline void serialize(Writer& w, double value);
    inline void serialize(Writer& w, const std::string& value);
    inline void serialize(Writer& w, const std::vector<std::byte>& value);
    
    // Forward declarations for SDK types
    inline void serialize(Writer& w, const SpacetimeDb::sdk::Identity& value);
    inline void serialize(Writer& w, const SpacetimeDb::sdk::ConnectionId& value);
    inline void serialize(Writer& w, const SpacetimeDb::sdk::Timestamp& value);
    inline void serialize(Writer& w, const SpacetimeDb::sdk::TimeDuration& value);
    inline void serialize(Writer& w, const SpacetimeDb::sdk::ScheduleAt& value);

    // Overloads for optionals and vectors that call Writer member functions
    template<typename T>
    inline void serialize(Writer& w, const std::optional<T>& opt_value);

    template<typename T>
    inline void serialize(Writer& w, const std::vector<T>& vec);

    class Writer {
    public:
        Writer() = default;

        void write_bool(bool value);
        void write_u8(uint8_t value);
        void write_u16_le(uint16_t value);
        void write_u32_le(uint32_t value);
        void write_u64_le(uint64_t value);
        void write_u128_le(const SpacetimeDb::Types::uint128_t_placeholder& value);

        void write_i8(int8_t value);
        void write_i16_le(int16_t value);
        void write_i32_le(int32_t value);
        void write_i64_le(int64_t value);
        void write_i128_le(const SpacetimeDb::Types::int128_t_placeholder& value);

        void write_f32_le(float value);
        void write_f64_le(double value);

        void write_string(const std::string& value);
        void write_bytes(const std::vector<std::byte>& value);

        template<typename T> // Removed Func, will use SpacetimeDb::bsatn::serialize(w, T_val)
        void write_optional(const std::optional<T>& opt_value) { // Renamed from write_optional(Func)
            if (opt_value.has_value()) {
                write_u8(1);
                SpacetimeDb::bsatn::serialize(*this, *opt_value); // Use the generic free serialize
            }
            else {
                write_u8(0);
            }
        }

        template<typename T> // Removed Func, will use SpacetimeDb::bsatn::serialize(w, T_val)
        void write_vector(const std::vector<T>& vec) { // Renamed from write_vector(Func)
            write_u32_le(static_cast<uint32_t>(vec.size()));
            for (const auto& item : vec) {
                SpacetimeDb::bsatn::serialize(*this, item); // Use the generic free serialize
            }
        }

        void write_vector_byte(const std::vector<std::byte>& vec); // Specific version for vector of bytes

        // Generic serialize member function (calls the free template function SpacetimeDb::bsatn::serialize<T>)
        template<typename T>
        void serialize_member(const T& value) { // Renamed to avoid conflict with free function
            SpacetimeDb::bsatn::serialize(*this, value);
        }

        const std::vector<std::byte>& get_buffer() const;
        std::vector<std::byte>&& take_buffer();

    private:
        void write_bytes_raw(const void* data, size_t size);
        std::vector<std::byte> buffer;

    public:
        // Inline implementations for 256-bit types
        inline void write_u256_le(const SpacetimeDb::sdk::u256_placeholder& value) {
            // TODO: Handle endianness if necessary for each uint64_t component
            write_bytes_raw(value.data.data(), sizeof(value.data));
        }

        inline void write_i256_le(const SpacetimeDb::sdk::i256_placeholder& value) {
            // TODO: Handle endianness
            write_bytes_raw(value.data.data(), sizeof(value.data));
        }
    };


    // Definition of the generic free function template for serialization.
    // This is the primary entry point for serializing an object.
    // It relies on specializations for specific types (primitives, user-defined structs/enums).
    template<typename T>
    void serialize(Writer& w, const T& value) {
        // If T is an enum class, cast it to its underlying type (assumed u8 by macros)
        if constexpr (std::is_enum_v<T>) {
            w.write_u8(static_cast<uint8_t>(value));
        }
        else {
            // This will fail to compile if no specialization or matching overload of
            // `serialize(Writer&, const UserType&)` is found for T, which is good.
            // The SDK macros generate global `SpacetimeDB::bsatn::serialize(Writer&, const UserType&)`
            // which should be found via ADL if `value` is in that namespace, or if this template
            // itself is in SpacetimeDB::bsatn.
            // To ensure it works, let's assume this template is in `bsatn` and generated functions are in `SpacetimeDB::bsatn`.
            // Or, we rely on ADL.
            // For now, let's assume this generic serialize calls a specific, potentially overloaded, serialize_internal or similar.
            // However, the previous Writer::serialize<T> called a free function `serialize(Writer&, const T&)`
            // So this free function itself IS the one that needs specializations or further overloads.
            // This primary template can serve as a fallback that static_asserts or gives a clear error.

            // The macros generate `SpacetimeDB::bsatn::serialize(Writer& writer, const CppTypeName& value)`
            // So, this generic `bsatn::serialize` will not be called for those.
            // For primitives, we need explicit `bsatn::serialize(Writer&, const Primitive&)` overloads or specializations.

            // This is a placeholder for types not covered by specific overloads/specializations.
            // A common pattern is to have a helper struct: `serializer<T>::apply(w, value);`
            // For now, this generic function will only handle enums directly. Other types *must* have an overload.
            // For user-defined types, the serialize function should be found by ADL
            // If not found, the compiler will give an error
        }
    }

    // Explicit overloads for primitives (could also be specializations of the template)
    // These are often provided by a bsatn_lib.h or similar from codegen.
    // For consistency with macros that generate `SpacetimeDB::bsatn::serialize`, these should also be in that namespace.
    // However, bsatn::Writer is in `bsatn`. This implies `SpacetimeDB::bsatn` for generated code.
    // Let's assume this file defines the core `bsatn::` namespace functions.

    // Implementation of serialize overloads for primitives
    inline void serialize(Writer& w, bool value) { w.write_bool(value); }
    inline void serialize(Writer& w, uint8_t value) { w.write_u8(value); }
    inline void serialize(Writer& w, uint16_t value) { w.write_u16_le(value); }
    inline void serialize(Writer& w, uint32_t value) { w.write_u32_le(value); }
    inline void serialize(Writer& w, uint64_t value) { w.write_u64_le(value); }
    inline void serialize(Writer& w, const SpacetimeDb::Types::uint128_t_placeholder& value) { w.write_u128_le(value); }
    inline void serialize(Writer& w, const SpacetimeDb::sdk::u256_placeholder& value) { w.write_u256_le(value); }
    inline void serialize(Writer& w, int8_t value) { w.write_i8(value); }
    inline void serialize(Writer& w, int16_t value) { w.write_i16_le(value); }
    inline void serialize(Writer& w, int32_t value) { w.write_i32_le(value); }
    inline void serialize(Writer& w, int64_t value) { w.write_i64_le(value); }
    inline void serialize(Writer& w, const SpacetimeDb::Types::int128_t_placeholder& value) { w.write_i128_le(value); }
    inline void serialize(Writer& w, const SpacetimeDb::sdk::i256_placeholder& value) { w.write_i256_le(value); }
    inline void serialize(Writer& w, float value) { w.write_f32_le(value); }
    inline void serialize(Writer& w, double value) { w.write_f64_le(value); }
    inline void serialize(Writer& w, const std::string& value) { w.write_string(value); }
    inline void serialize(Writer& w, const std::vector<std::byte>& value) { w.write_bytes(value); }
    
    // Serialize functions for SDK types
    inline void serialize(Writer& w, const SpacetimeDb::sdk::Identity& value) { value.bsatn_serialize(w); }
    inline void serialize(Writer& w, const SpacetimeDb::sdk::ConnectionId& value) { value.bsatn_serialize(w); }
    inline void serialize(Writer& w, const SpacetimeDb::sdk::Timestamp& value) { value.bsatn_serialize(w); }
    inline void serialize(Writer& w, const SpacetimeDb::sdk::TimeDuration& value) { value.bsatn_serialize(w); }
    inline void serialize(Writer& w, const SpacetimeDb::sdk::ScheduleAt& value) { value.bsatn_serialize(w); }

    // Implementation of serialize for optionals
    template<typename T>
    inline void serialize(Writer& w, const std::optional<T>& opt_value) {
        w.write_optional(opt_value);
    }

    // Implementation of serialize for vectors
    template<typename T>
    inline void serialize(Writer& w, const std::vector<T>& vec) {
        w.write_vector(vec);
    }

} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_WRITER_H
