#ifndef SPACETIMEDB_FIELD_REGISTRATION_H
#define SPACETIMEDB_FIELD_REGISTRATION_H

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <type_traits>
#include <functional>
#include <cstring>  // for memcpy
#include <map>
#include <typeinfo>
#include "spacetimedb/types.h"  // For Identity, etc.
#include "spacetimedb/bsatn/uint128_placeholder.h"  // For Types namespace
#include "spacetimedb/timestamp.h"  // For Timestamp class

namespace SpacetimeDb {

// Extended type system for BSATN
namespace bsatn_type {
    constexpr uint8_t REF = 0;
    constexpr uint8_t SUM = 1;
    constexpr uint8_t PRODUCT = 2;
    constexpr uint8_t ARRAY = 3;
    constexpr uint8_t STRING = 4;
    constexpr uint8_t BOOL = 5;
    constexpr uint8_t I8 = 6;
    constexpr uint8_t U8 = 7;
    constexpr uint8_t I16 = 8;
    constexpr uint8_t U16 = 9;
    constexpr uint8_t I32 = 10;
    constexpr uint8_t U32 = 11;
    constexpr uint8_t I64 = 12;
    constexpr uint8_t U64 = 13;
    constexpr uint8_t I128 = 14;
    constexpr uint8_t U128 = 15;
    constexpr uint8_t I256 = 16;
    constexpr uint8_t U256 = 17;
    constexpr uint8_t F32 = 18;
    constexpr uint8_t F64 = 19;
    
    // Note: BYTES is not a primitive type in the AlgebraicType enum
    // It's represented as ARRAY of U8
    constexpr uint8_t BYTES = ARRAY; // Special case: vector<uint8_t> = Array<U8>
}

// Type traits for BSATN type mapping
template<typename T> struct bsatn_type_id { 
    static constexpr bool is_primitive = false;
    static constexpr uint8_t value = 0; 
};

// Primitive types
template<> struct bsatn_type_id<bool> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::BOOL; 
};
template<> struct bsatn_type_id<uint8_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U8; 
};
template<> struct bsatn_type_id<uint16_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U16; 
};
template<> struct bsatn_type_id<uint32_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U32; 
};
template<> struct bsatn_type_id<uint64_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U64; 
};
template<> struct bsatn_type_id<int8_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::I8; 
};
template<> struct bsatn_type_id<int16_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::I16; 
};
template<> struct bsatn_type_id<int32_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::I32; 
};
template<> struct bsatn_type_id<int64_t> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::I64; 
};
template<> struct bsatn_type_id<float> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::F32; 
};
template<> struct bsatn_type_id<double> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::F64; 
};
template<> struct bsatn_type_id<std::string> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::STRING; 
};
template<> struct bsatn_type_id<std::vector<uint8_t>> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::BYTES; 
};

// Complex types need special handling
template<typename T> 
struct bsatn_type_id<std::vector<T>> { 
    static constexpr bool is_primitive = false;
    static constexpr uint8_t value = bsatn_type::ARRAY; 
};

template<typename T> 
struct bsatn_type_id<std::optional<T>> { 
    static constexpr bool is_primitive = false;
    static constexpr uint8_t value = bsatn_type::SUM; 
};

// Special SpacetimeDB types
template<> struct bsatn_type_id<Identity> { 
    static constexpr bool is_primitive = false;
    static constexpr uint8_t value = bsatn_type::PRODUCT; 
};

template<> struct bsatn_type_id<ConnectionId> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U64; 
};

template<> struct bsatn_type_id<Timestamp> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U64; 
};

// Placeholder 128/256-bit types
template<> struct bsatn_type_id<Types::uint128_t_placeholder> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U128; 
};

template<> struct bsatn_type_id<Types::uint256_t_placeholder> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::U256; 
};

template<> struct bsatn_type_id<Types::int128_t_placeholder> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::I128; 
};

template<> struct bsatn_type_id<Types::int256_t_placeholder> { 
    static constexpr bool is_primitive = true;
    static constexpr uint8_t value = bsatn_type::I256; 
};

// Utility functions for BSATN encoding (implemented in spacetimedb.h)
#ifndef SPACETIMEDB_WRITE_UTILS_DEFINED
#define SPACETIMEDB_WRITE_UTILS_DEFINED

inline void write_u32(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

inline void write_string(std::vector<uint8_t>& buf, const std::string& str) {
    write_u32(buf, static_cast<uint32_t>(str.length()));
    for (char c : str) {
        buf.push_back(static_cast<uint8_t>(c));
    }
}

#endif // SPACETIMEDB_WRITE_UTILS_DEFINED

template<typename T>
void write_field_type(std::vector<uint8_t>& buf);

// Field descriptor for runtime reflection
struct FieldDescriptor {
    std::string name;
    size_t offset;
    size_t size;
    std::function<void(std::vector<uint8_t>&)> write_type;  // Writes AlgebraicType
    std::function<void(std::vector<uint8_t>&, const void*)> serialize;  // Serializes value
};

// Table descriptor
struct TableDescriptor {
    std::vector<FieldDescriptor> fields;
};

// Global registry for table descriptors
inline std::map<const std::type_info*, TableDescriptor>& get_table_descriptors() {
    static std::map<const std::type_info*, TableDescriptor> descriptors;
    return descriptors;
}

// Helper to write AlgebraicType for primitive types
template<typename T>
typename std::enable_if<bsatn_type_id<T>::is_primitive, void>::type
write_algebraic_type(std::vector<uint8_t>& buf) {
    buf.push_back(bsatn_type_id<T>::value);
}

// Helper to write AlgebraicType for vectors
template<typename T>
void write_algebraic_type(std::vector<uint8_t>& buf, const std::vector<T>*) {
    buf.push_back(bsatn_type::ARRAY);  // Array type
    write_field_type<T>(buf);  // Element type - use unified writer
}

// Helper to write AlgebraicType for optionals
template<typename T>
void write_algebraic_type(std::vector<uint8_t>& buf, const std::optional<T>*) {
    buf.push_back(bsatn_type::SUM);  // Sum type
    write_u32(buf, 2);  // 2 variants (must be u32!)
    
    // Variant 0: Some
    // name: Option<Box<str>> = Some("some")
    buf.push_back(0);  // Some tag for Option<Box<str>>
    write_string(buf, "some");
    // algebraic_type: inner type T
    write_field_type<T>(buf);  // Inner type - use unified writer
    
    // Variant 1: None  
    // name: Option<Box<str>> = Some("none")
    buf.push_back(0);  // Some tag for Option<Box<str>>
    write_string(buf, "none");
    // algebraic_type: unit type (empty product)
    buf.push_back(bsatn_type::PRODUCT);  // Unit type
    write_u32(buf, 0);  // 0 fields
}

// Helper to write AlgebraicType for Identity (array of 32 bytes)
inline void write_algebraic_type(std::vector<uint8_t>& buf, const Identity*) {
    buf.push_back(bsatn_type::ARRAY);  // Array type
    write_field_type<uint8_t>(buf);    // Element type: u8 (properly encoded)
    // TODO: Should also encode length = 32, but current format doesn't support fixed-size arrays
}

// Generic helper for custom struct types
template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value && !bsatn_type_id<T>::is_primitive, void>::type
write_algebraic_type_struct(std::vector<uint8_t>& buf) {
    auto& descriptors = get_table_descriptors();
    auto it = descriptors.find(&typeid(T));
    
    if (it != descriptors.end()) {
        // Write as Product type
        buf.push_back(bsatn_type::PRODUCT);
        write_u32(buf, it->second.fields.size());
        
        for (const auto& field : it->second.fields) {
            buf.push_back(0);  // Some (field name present) - BSATN Option::Some = 0
            write_string(buf, field.name);
            field.write_type(buf);
        }
    } else {
        // Fallback - write as opaque type
        buf.push_back(bsatn_type::PRODUCT);
        write_u32(buf, 0);  // No fields
    }
}

// Template to detect vector types
template<typename T>
struct is_vector : std::false_type {};

template<typename T>
struct is_vector<std::vector<T>> : std::true_type {};

// Template to detect optional types
template<typename T>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

// Template to detect enum types (we can just use std::is_enum directly)

// Unified type writer that handles all cases
template<typename T>
void write_field_type(std::vector<uint8_t>& buf) {
    if constexpr (std::is_enum_v<T>) {
        // Enums are serialized as their underlying type (usually u32 for Sum types)
        buf.push_back(bsatn_type::U32);  // Most enums map to u32
    } else if constexpr (bsatn_type_id<T>::is_primitive) {
        write_algebraic_type<T>(buf);
    } else if constexpr (std::is_same_v<T, Identity>) {
        write_algebraic_type(buf, static_cast<Identity*>(nullptr));
    } else if constexpr (std::is_same_v<T, ConnectionId>) {
        buf.push_back(bsatn_type::U64);  // ConnectionId is serialized as u64
    } else if constexpr (std::is_same_v<T, Timestamp>) {
        buf.push_back(bsatn_type::U64);  // Timestamp is serialized as u64
    } else if constexpr (is_vector<T>::value) {
        write_algebraic_type(buf, static_cast<T*>(nullptr));
    } else if constexpr (is_optional<T>::value) {
        write_algebraic_type(buf, static_cast<T*>(nullptr));
    } else {
        // Custom struct type
        write_algebraic_type_struct<T>(buf);
    }
}

// Forward declarations for all serialize_value functions
// Primitive types - defined in field_registration.cpp
void serialize_value(std::vector<uint8_t>& buf, const bool& val);
void serialize_value(std::vector<uint8_t>& buf, const uint8_t& val);
void serialize_value(std::vector<uint8_t>& buf, const uint16_t& val);
void serialize_value(std::vector<uint8_t>& buf, const uint32_t& val);
void serialize_value(std::vector<uint8_t>& buf, const uint64_t& val);
void serialize_value(std::vector<uint8_t>& buf, const int8_t& val);
void serialize_value(std::vector<uint8_t>& buf, const int16_t& val);
void serialize_value(std::vector<uint8_t>& buf, const int32_t& val);
void serialize_value(std::vector<uint8_t>& buf, const int64_t& val);
void serialize_value(std::vector<uint8_t>& buf, const float& val);
void serialize_value(std::vector<uint8_t>& buf, const double& val);

template<typename T>
typename std::enable_if<std::is_enum<T>::value, void>::type
serialize_value(std::vector<uint8_t>& buf, const T& val);

template<typename T>
void serialize_value(std::vector<uint8_t>& buf, const std::vector<T>& val);

template<typename T>
void serialize_value(std::vector<uint8_t>& buf, const std::optional<T>& val);

void serialize_value(std::vector<uint8_t>& buf, const std::string& val);
void serialize_value(std::vector<uint8_t>& buf, const std::vector<uint8_t>& val);
void serialize_value(std::vector<uint8_t>& buf, const Identity& val);
void serialize_value(std::vector<uint8_t>& buf, const ConnectionId& val);
void serialize_value(std::vector<uint8_t>& buf, const Timestamp& val);
void serialize_value(std::vector<uint8_t>& buf, const Types::uint128_t_placeholder& val);
void serialize_value(std::vector<uint8_t>& buf, const Types::uint256_t_placeholder& val);
void serialize_value(std::vector<uint8_t>& buf, const Types::int128_t_placeholder& val);
void serialize_value(std::vector<uint8_t>& buf, const Types::int256_t_placeholder& val);

template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value && !std::is_enum<T>::value && !bsatn_type_id<T>::is_primitive, void>::type
serialize_value(std::vector<uint8_t>& buf, const T& val);

// Include inline implementations for header-only usage

inline void serialize_value(std::vector<uint8_t>& buf, const std::string& val) {
    write_u32(buf, val.length());
    for (char c : val) {
        buf.push_back(static_cast<uint8_t>(c));
    }
}

inline void serialize_value(std::vector<uint8_t>& buf, const std::vector<uint8_t>& val) {
    write_u32(buf, val.size());
    for (uint8_t b : val) {
        buf.push_back(b);
    }
}

template<typename T>
void serialize_value(std::vector<uint8_t>& buf, const std::optional<T>& val) {
    if (val.has_value()) {
        buf.push_back(0);  // Some tag
        serialize_value(buf, *val);
    } else {
        buf.push_back(1);  // None tag
    }
}

// Serialization for enums
template<typename T>
typename std::enable_if<std::is_enum<T>::value, void>::type
serialize_value(std::vector<uint8_t>& buf, const T& val) {
    using underlying = typename std::underlying_type<T>::type;
    serialize_value(buf, static_cast<underlying>(val));
}

// Generic serialization for custom structs - uses registered field descriptors
template<typename T>
typename std::enable_if<!std::is_arithmetic<T>::value && !std::is_enum<T>::value && !bsatn_type_id<T>::is_primitive, void>::type
serialize_value(std::vector<uint8_t>& buf, const T& val) {
    auto& descriptors = get_table_descriptors();
    auto it = descriptors.find(&typeid(T));
    
    if (it != descriptors.end()) {
        // Serialize each field using registered descriptors
        for (const auto& field : it->second.fields) {
            field.serialize(buf, &val);
        }
    }
}

// Serialization for special types
inline void serialize_value(std::vector<uint8_t>& buf, const SpacetimeDb::Identity& val) {
    const auto& bytes = val.get_bytes();
    for (uint8_t b : bytes) {
        buf.push_back(b);
    }
}

inline void serialize_value(std::vector<uint8_t>& buf, const SpacetimeDb::ConnectionId& val) {
    serialize_value(buf, val.id);
}

inline void serialize_value(std::vector<uint8_t>& buf, const SpacetimeDb::Timestamp& val) {
    serialize_value(buf, val.millis_since_epoch());
}

// Serialization for placeholder 128/256-bit types
inline void serialize_value(std::vector<uint8_t>& buf, const SpacetimeDb::Types::uint128_t_placeholder& val) {
    serialize_value(buf, val.low);
    serialize_value(buf, val.high);
}

inline void serialize_value(std::vector<uint8_t>& buf, const SpacetimeDb::Types::uint256_t_placeholder& val) {
    serialize_value(buf, val.d0);
    serialize_value(buf, val.d1);
    serialize_value(buf, val.d2);
    serialize_value(buf, val.d3);
}

inline void serialize_value(std::vector<uint8_t>& buf, const SpacetimeDb::Types::int128_t_placeholder& val) {
    serialize_value(buf, val.low);
    serialize_value(buf, static_cast<uint64_t>(val.high));
}

inline void serialize_value(std::vector<uint8_t>& buf, const SpacetimeDb::Types::int256_t_placeholder& val) {
    serialize_value(buf, val.d0);
    serialize_value(buf, val.d1);
    serialize_value(buf, val.d2);
    serialize_value(buf, static_cast<uint64_t>(val.d3));
}

// Template implementation for vectors - must come after all specific type implementations
template<typename T>
void serialize_value(std::vector<uint8_t>& buf, const std::vector<T>& val) {
    write_u32(buf, val.size());
    for (const auto& elem : val) {
        serialize_value(buf, elem);
    }
}

// Macro to register a single field
#define SPACETIMEDB_FIELD(struct_type, field_name, field_type) \
    { \
        SpacetimeDb::FieldDescriptor desc; \
        desc.name = #field_name; \
        desc.offset = offsetof(struct_type, field_name); \
        desc.size = sizeof(field_type); \
        desc.write_type = [](std::vector<uint8_t>& buf) { \
            SpacetimeDb::write_field_type<field_type>(buf); \
        }; \
        desc.serialize = [](std::vector<uint8_t>& buf, const void* obj) { \
            const struct_type* typed_obj = static_cast<const struct_type*>(obj); \
            SpacetimeDb::serialize_value(buf, typed_obj->field_name); \
        }; \
        SpacetimeDb::get_table_descriptors()[&typeid(struct_type)].fields.push_back(desc); \
    }

// Macro to register all fields of a struct
#define SPACETIMEDB_REGISTER_FIELDS(struct_type, ...) \
    namespace { \
        struct struct_type##_field_registrar { \
            struct_type##_field_registrar() { \
                __VA_ARGS__ \
            } \
        }; \
        static struct_type##_field_registrar struct_type##_field_registrar_instance; \
    }

} // namespace SpacetimeDb

#endif // SPACETIMEDB_FIELD_REGISTRATION_H