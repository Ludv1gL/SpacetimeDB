#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace spacetimedb {

// Forward declarations
struct SumType;
struct ProductType;
struct ArrayType;
struct SumTypeVariant;
struct ProductTypeElement;
class TypeContext;
using AlgebraicTypeRef = uint32_t;

// AlgebraicType tags matching SpacetimeDB's type system
enum class AlgebraicTypeTag : uint8_t {
    Ref = 0,
    Sum = 1,
    Product = 2,
    Array = 3,
    String = 4,
    Bool = 5,
    I8 = 6,
    U8 = 7,
    I16 = 8,
    U16 = 9,
    I32 = 10,
    U32 = 11,
    I64 = 12,
    U64 = 13,
    I128 = 14,
    U128 = 15,
    I256 = 16,
    U256 = 17,
    F32 = 18,
    F64 = 19
};

struct SumTypeVariant {
    std::string name;
    AlgebraicTypeRef algebraic_type;
    
    SumTypeVariant(std::string n, AlgebraicTypeRef type)
        : name(std::move(n)), algebraic_type(type) {}
};

struct SumType {
    std::vector<SumTypeVariant> variants;
    
    explicit SumType(std::vector<SumTypeVariant> v) : variants(std::move(v)) {}
};

struct ProductTypeElement {
    std::optional<std::string> name;
    AlgebraicTypeRef algebraic_type;
    
    ProductTypeElement(std::optional<std::string> n, AlgebraicTypeRef type)
        : name(std::move(n)), algebraic_type(type) {}
};

struct ProductType {
    std::vector<ProductTypeElement> elements;
    
    explicit ProductType(std::vector<ProductTypeElement> elems) 
        : elements(std::move(elems)) {}
};

struct ArrayType {
    AlgebraicTypeRef element_type;
    
    explicit ArrayType(AlgebraicTypeRef elem_type) : element_type(elem_type) {}
};

// Main AlgebraicType class
class AlgebraicType {
public:
    using DataType = std::variant<
        AlgebraicTypeRef,              // Ref
        std::unique_ptr<SumType>,      // Sum
        std::unique_ptr<ProductType>,  // Product
        std::unique_ptr<ArrayType>,    // Array
        std::monostate                 // Primitives
    >;

private:
    AlgebraicTypeTag tag_;
    DataType data_;

public:
    // Constructor
    AlgebraicType(AlgebraicTypeTag tag, DataType data) 
        : tag_(tag), data_(std::move(data)) {}

    // Primitive type constructors
    static AlgebraicType bool_() {
        return AlgebraicType(AlgebraicTypeTag::Bool, std::monostate{});
    }
    
    static AlgebraicType i8() {
        return AlgebraicType(AlgebraicTypeTag::I8, std::monostate{});
    }
    
    static AlgebraicType u8() {
        return AlgebraicType(AlgebraicTypeTag::U8, std::monostate{});
    }
    
    static AlgebraicType i16() {
        return AlgebraicType(AlgebraicTypeTag::I16, std::monostate{});
    }
    
    static AlgebraicType u16() {
        return AlgebraicType(AlgebraicTypeTag::U16, std::monostate{});
    }
    
    static AlgebraicType i32() {
        return AlgebraicType(AlgebraicTypeTag::I32, std::monostate{});
    }
    
    static AlgebraicType u32() {
        return AlgebraicType(AlgebraicTypeTag::U32, std::monostate{});
    }
    
    static AlgebraicType i64() {
        return AlgebraicType(AlgebraicTypeTag::I64, std::monostate{});
    }
    
    static AlgebraicType u64() {
        return AlgebraicType(AlgebraicTypeTag::U64, std::monostate{});
    }
    
    static AlgebraicType i128() {
        return AlgebraicType(AlgebraicTypeTag::I128, std::monostate{});
    }
    
    static AlgebraicType u128() {
        return AlgebraicType(AlgebraicTypeTag::U128, std::monostate{});
    }
    
    static AlgebraicType i256() {
        return AlgebraicType(AlgebraicTypeTag::I256, std::monostate{});
    }
    
    static AlgebraicType u256() {
        return AlgebraicType(AlgebraicTypeTag::U256, std::monostate{});
    }
    
    static AlgebraicType f32() {
        return AlgebraicType(AlgebraicTypeTag::F32, std::monostate{});
    }
    
    static AlgebraicType f64() {
        return AlgebraicType(AlgebraicTypeTag::F64, std::monostate{});
    }
    
    static AlgebraicType string() {
        return AlgebraicType(AlgebraicTypeTag::String, std::monostate{});
    }
    
    // Complex type constructors
    static AlgebraicType ref(AlgebraicTypeRef type_ref) {
        return AlgebraicType(AlgebraicTypeTag::Ref, type_ref);
    }
    
    static AlgebraicType sum(std::unique_ptr<SumType> sum_type) {
        return AlgebraicType(AlgebraicTypeTag::Sum, std::move(sum_type));
    }
    
    static AlgebraicType product(std::unique_ptr<ProductType> product_type) {
        return AlgebraicType(AlgebraicTypeTag::Product, std::move(product_type));
    }
    
    static AlgebraicType array(std::unique_ptr<ArrayType> array_type) {
        return AlgebraicType(AlgebraicTypeTag::Array, std::move(array_type));
    }
    
    // Convenience constructors
    static AlgebraicType sum(std::vector<SumTypeVariant> variants) {
        return sum(std::make_unique<SumType>(std::move(variants)));
    }
    
    static AlgebraicType product(std::vector<ProductTypeElement> elements) {
        return product(std::make_unique<ProductType>(std::move(elements)));
    }
    
    static AlgebraicType array(AlgebraicTypeRef element_type) {
        return array(std::make_unique<ArrayType>(element_type));
    }
    
    // Special types for SpacetimeDB
    static AlgebraicType timestamp() {
        // Timestamp is represented as U64 microseconds since Unix epoch
        return u64();
    }
    
    static AlgebraicType time_duration() {
        // TimeDuration is represented as I64 microseconds
        return i64();
    }
    
    // Getters
    AlgebraicTypeTag tag() const { return tag_; }
    const DataType& data() const { return data_; }
    
    // Type checking
    bool is_primitive() const {
        return static_cast<uint8_t>(tag_) >= static_cast<uint8_t>(AlgebraicTypeTag::String);
    }
    
    bool is_ref() const { return tag_ == AlgebraicTypeTag::Ref; }
    bool is_sum() const { return tag_ == AlgebraicTypeTag::Sum; }
    bool is_product() const { return tag_ == AlgebraicTypeTag::Product; }
    bool is_array() const { return tag_ == AlgebraicTypeTag::Array; }
};

// TypeContext for managing type references
class TypeContext {
private:
    std::vector<AlgebraicType> types_;
    
public:
    AlgebraicTypeRef add(AlgebraicType type) {
        types_.push_back(std::move(type));
        return static_cast<AlgebraicTypeRef>(types_.size() - 1);
    }
    
    const AlgebraicType& get(AlgebraicTypeRef ref) const {
        return types_.at(ref);
    }
    
    size_t size() const { return types_.size(); }
};

} // namespace spacetimedb