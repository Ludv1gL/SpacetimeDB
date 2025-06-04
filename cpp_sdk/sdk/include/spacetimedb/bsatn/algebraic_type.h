#ifndef SPACETIMEDB_BSATN_ALGEBRAIC_TYPE_H
#define SPACETIMEDB_BSATN_ALGEBRAIC_TYPE_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <optional>

namespace SpacetimeDb::bsatn {

// Forward declarations
struct SumType;
struct ProductType;
struct ArrayType;

/**
 * Represents the tag for different algebraic types in SpacetimeDB's type system.
 * This mirrors the Rust/C# implementation for compatibility.
 */
enum class AlgebraicTypeTag : uint8_t {
    Ref = 0,      // Reference to another type
    Sum = 1,      // Sum type (tagged union/enum)
    Product = 2,  // Product type (struct/tuple)
    Array = 3,    // Array type
    String = 4,   // UTF-8 string
    Bool = 5,     // Boolean
    I8 = 6,       // Signed 8-bit integer
    U8 = 7,       // Unsigned 8-bit integer
    I16 = 8,      // Signed 16-bit integer
    U16 = 9,      // Unsigned 16-bit integer
    I32 = 10,     // Signed 32-bit integer
    U32 = 11,     // Unsigned 32-bit integer
    I64 = 12,     // Signed 64-bit integer
    U64 = 13,     // Unsigned 64-bit integer
    I128 = 14,    // Signed 128-bit integer
    U128 = 15,    // Unsigned 128-bit integer
    I256 = 16,    // Signed 256-bit integer
    U256 = 17,    // Unsigned 256-bit integer
    F32 = 18,     // 32-bit floating point
    F64 = 19      // 64-bit floating point
};

/**
 * Represents an element in a Product or Sum type.
 * Each element has an optional name and a type.
 */
struct AggregateElement {
    std::optional<std::string> name;
    uint32_t algebraic_type;  // Index into type registry
    
    AggregateElement(std::optional<std::string> n, uint32_t type)
        : name(std::move(n)), algebraic_type(type) {}
};

/**
 * Represents a sum type (tagged union/enum).
 * Each variant has a name and can contain data.
 */
struct SumType {
    struct Variant {
        std::string name;
        uint32_t algebraic_type;  // Index into type registry
        
        Variant(std::string n, uint32_t type)
            : name(std::move(n)), algebraic_type(type) {}
    };
    
    std::vector<Variant> variants;
    
    explicit SumType(std::vector<Variant> v) : variants(std::move(v)) {}
};

/**
 * Represents a product type (struct/tuple).
 * Contains ordered elements (fields).
 */
struct ProductType {
    std::vector<AggregateElement> elements;
    
    explicit ProductType(std::vector<AggregateElement> elems) 
        : elements(std::move(elems)) {}
    
    // Helper to create a product type for a C++ struct
    template<typename T>
    static ProductType make();
};

/**
 * Represents an array type.
 * Contains the type of elements in the array.
 */
struct ArrayType {
    uint32_t element_type;  // Index into type registry
    
    explicit ArrayType(uint32_t elem_type) : element_type(elem_type) {}
};

/**
 * The main algebraic type representation.
 * This is a tagged union of all possible types in the SpacetimeDB type system.
 */
class AlgebraicType {
public:
    using DataType = std::variant<
        uint32_t,                      // Ref - type reference
        std::unique_ptr<SumType>,      // Sum type
        std::unique_ptr<ProductType>,  // Product type
        std::unique_ptr<ArrayType>,    // Array type
        std::monostate                 // Primitive types (no additional data)
    >;

private:
    AlgebraicTypeTag tag_;
    DataType data_;

public:
    // Constructors for each type
    static AlgebraicType make_ref(uint32_t type_id) {
        return AlgebraicType(AlgebraicTypeTag::Ref, type_id);
    }
    
    static AlgebraicType make_sum(std::unique_ptr<SumType> sum) {
        return AlgebraicType(AlgebraicTypeTag::Sum, std::move(sum));
    }
    
    static AlgebraicType make_product(std::unique_ptr<ProductType> product) {
        return AlgebraicType(AlgebraicTypeTag::Product, std::move(product));
    }
    
    static AlgebraicType make_array(std::unique_ptr<ArrayType> array) {
        return AlgebraicType(AlgebraicTypeTag::Array, std::move(array));
    }
    
    static AlgebraicType make_string() {
        return AlgebraicType(AlgebraicTypeTag::String, std::monostate{});
    }
    
    static AlgebraicType make_bool() {
        return AlgebraicType(AlgebraicTypeTag::Bool, std::monostate{});
    }
    
    static AlgebraicType make_i8() {
        return AlgebraicType(AlgebraicTypeTag::I8, std::monostate{});
    }
    
    static AlgebraicType make_i16() {
        return AlgebraicType(AlgebraicTypeTag::I16, std::monostate{});
    }
    
    static AlgebraicType make_i32() {
        return AlgebraicType(AlgebraicTypeTag::I32, std::monostate{});
    }
    
    static AlgebraicType make_i64() {
        return AlgebraicType(AlgebraicTypeTag::I64, std::monostate{});
    }
    
    static AlgebraicType make_i128() {
        return AlgebraicType(AlgebraicTypeTag::I128, std::monostate{});
    }
    
    static AlgebraicType make_i256() {
        return AlgebraicType(AlgebraicTypeTag::I256, std::monostate{});
    }
    
    static AlgebraicType make_u8() {
        return AlgebraicType(AlgebraicTypeTag::U8, std::monostate{});
    }
    
    static AlgebraicType make_u16() {
        return AlgebraicType(AlgebraicTypeTag::U16, std::monostate{});
    }
    
    static AlgebraicType make_u32() {
        return AlgebraicType(AlgebraicTypeTag::U32, std::monostate{});
    }
    
    static AlgebraicType make_u64() {
        return AlgebraicType(AlgebraicTypeTag::U64, std::monostate{});
    }
    
    static AlgebraicType make_u128() {
        return AlgebraicType(AlgebraicTypeTag::U128, std::monostate{});
    }
    
    static AlgebraicType make_u256() {
        return AlgebraicType(AlgebraicTypeTag::U256, std::monostate{});
    }
    
    static AlgebraicType make_f32() {
        return AlgebraicType(AlgebraicTypeTag::F32, std::monostate{});
    }
    
    static AlgebraicType make_f64() {
        return AlgebraicType(AlgebraicTypeTag::F64, std::monostate{});
    }
    
    // Accessors
    AlgebraicTypeTag tag() const { return tag_; }
    const DataType& data() const { return data_; }
    
    // Type checking helpers
    bool is_ref() const { return tag_ == AlgebraicTypeTag::Ref; }
    bool is_sum() const { return tag_ == AlgebraicTypeTag::Sum; }
    bool is_product() const { return tag_ == AlgebraicTypeTag::Product; }
    bool is_array() const { return tag_ == AlgebraicTypeTag::Array; }
    bool is_primitive() const { 
        return static_cast<uint8_t>(tag_) >= static_cast<uint8_t>(AlgebraicTypeTag::String);
    }
    
    // Data accessors (throw if wrong type)
    uint32_t as_ref() const {
        if (!is_ref()) throw std::runtime_error("Type is not a Ref");
        return std::get<uint32_t>(data_);
    }
    
    const SumType& as_sum() const {
        if (!is_sum()) throw std::runtime_error("Type is not a Sum");
        return *std::get<std::unique_ptr<SumType>>(data_);
    }
    
    const ProductType& as_product() const {
        if (!is_product()) throw std::runtime_error("Type is not a Product");
        return *std::get<std::unique_ptr<ProductType>>(data_);
    }
    
    const ArrayType& as_array() const {
        if (!is_array()) throw std::runtime_error("Type is not an Array");
        return *std::get<std::unique_ptr<ArrayType>>(data_);
    }
    
    // Convenient factory methods for codegen
    static AlgebraicType Bool() { return make_bool(); }
    static AlgebraicType I8() { return make_i8(); }
    static AlgebraicType U8() { return make_u8(); }
    static AlgebraicType I16() { return make_i16(); }
    static AlgebraicType U16() { return make_u16(); }
    static AlgebraicType I32() { return make_i32(); }
    static AlgebraicType U32() { return make_u32(); }
    static AlgebraicType I64() { return make_i64(); }
    static AlgebraicType U64() { return make_u64(); }
    static AlgebraicType I128() { return make_i128(); }
    static AlgebraicType U128() { return make_u128(); }
    static AlgebraicType I256() { return make_i256(); }
    static AlgebraicType U256() { return make_u256(); }
    static AlgebraicType F32() { return make_f32(); }
    static AlgebraicType F64() { return make_f64(); }
    static AlgebraicType String() { return make_string(); }
    static AlgebraicType Ref(uint32_t type_id) { return make_ref(type_id); }
    static AlgebraicType Array(const AlgebraicType& elem_type) {
        // For now, we'll need to handle this differently since we need to store the type
        // This is a placeholder - we'll need to enhance this
        return make_array(std::make_unique<ArrayType>(0)); // TODO: fix this
    }
    static AlgebraicType Option(const AlgebraicType& some_type) {
        // Create an option type as a sum type with two variants
        std::vector<SumType::Variant> variants;
        variants.emplace_back("some", 0); // TODO: fix type index
        variants.emplace_back("none", 0); // TODO: fix type index (unit type)
        return make_sum(std::make_unique<SumType>(std::move(variants)));
    }
    static AlgebraicType Product(std::vector<std::pair<std::string, AlgebraicType>> fields) {
        // TODO: This needs to be properly implemented with type registry
        std::vector<AggregateElement> elements;
        for (const auto& [name, type] : fields) {
            elements.emplace_back(name, 0); // TODO: fix type index
        }
        return make_product(std::make_unique<ProductType>(std::move(elements)));
    }
    static AlgebraicType Sum(std::vector<std::pair<std::string, AlgebraicType>> variants) {
        // TODO: This needs to be properly implemented with type registry
        std::vector<SumType::Variant> sum_variants;
        for (const auto& [name, type] : variants) {
            sum_variants.emplace_back(name, 0); // TODO: fix type index
        }
        return make_sum(std::make_unique<SumType>(std::move(sum_variants)));
    }

private:
    template<typename T>
    AlgebraicType(AlgebraicTypeTag tag, T&& data) 
        : tag_(tag), data_(std::forward<T>(data)) {}
};

// Type trait to get algebraic type for C++ types
template<typename T>
struct algebraic_type_of {
    static AlgebraicType get();
};

// Specializations for primitive types
template<> struct algebraic_type_of<bool> {
    static AlgebraicType get() { return AlgebraicType::make_bool(); }
};

template<> struct algebraic_type_of<int8_t> {
    static AlgebraicType get() { return AlgebraicType::make_i8(); }
};

template<> struct algebraic_type_of<int16_t> {
    static AlgebraicType get() { return AlgebraicType::make_i16(); }
};

template<> struct algebraic_type_of<int32_t> {
    static AlgebraicType get() { return AlgebraicType::make_i32(); }
};

template<> struct algebraic_type_of<int64_t> {
    static AlgebraicType get() { return AlgebraicType::make_i64(); }
};

template<> struct algebraic_type_of<uint8_t> {
    static AlgebraicType get() { return AlgebraicType::make_u8(); }
};

template<> struct algebraic_type_of<uint16_t> {
    static AlgebraicType get() { return AlgebraicType::make_u16(); }
};

template<> struct algebraic_type_of<uint32_t> {
    static AlgebraicType get() { return AlgebraicType::make_u32(); }
};

template<> struct algebraic_type_of<uint64_t> {
    static AlgebraicType get() { return AlgebraicType::make_u64(); }
};

template<> struct algebraic_type_of<float> {
    static AlgebraicType get() { return AlgebraicType::make_f32(); }
};

template<> struct algebraic_type_of<double> {
    static AlgebraicType get() { return AlgebraicType::make_f64(); }
};

template<> struct algebraic_type_of<std::string> {
    static AlgebraicType get() { return AlgebraicType::make_string(); }
};

// TODO: Add specializations for I128, I256, U128, U256 when those types are properly defined

} // namespace SpacetimeDb::bsatn

// Namespace alias for convenience
namespace spacetimedb {
    using AlgebraicType = SpacetimeDb::bsatn::AlgebraicType;
    using AlgebraicTypeTag = SpacetimeDb::bsatn::AlgebraicTypeTag;
    using SumType = SpacetimeDb::bsatn::SumType;
    using ProductType = SpacetimeDb::bsatn::ProductType;
    using ArrayType = SpacetimeDb::bsatn::ArrayType;
    using AggregateElement = SpacetimeDb::bsatn::AggregateElement;
}

#endif // SPACETIMEDB_BSATN_ALGEBRAIC_TYPE_H