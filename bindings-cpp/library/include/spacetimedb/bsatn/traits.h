#ifndef SPACETIMEDB_BSATN_TRAITS_H
#define SPACETIMEDB_BSATN_TRAITS_H

#include "algebraic_type.h"
#include "reader.h"
#include "writer.h"
#include "ITypeRegistrar.h"
#include <type_traits>
#if __cplusplus >= 202002L
#include <concepts>
#endif

namespace SpacetimeDb::bsatn {

// Forward declaration
class TypeRegistrar;

/**
 * Concept for types that can be serialized to/from BSATN format.
 * Types can satisfy this by:
 * 1. Specializing the bsatn_traits template
 * 2. Implementing member functions bsatn_serialize/bsatn_deserialize
 * 3. Having ADL-findable free functions
 */
#if __cplusplus >= 202002L
template<typename T>
concept BsatnSerializable = requires(T t, Writer& w, Reader& r) {
    { serialize(w, t) } -> std::same_as<void>;
    { deserialize<T>(r) } -> std::same_as<T>;
};
#endif

/**
 * Primary template for BSATN serialization traits.
 * Specialize this for your types to enable serialization.
 */
template<typename T>
struct bsatn_traits {
    static void serialize(Writer& writer, const T& value) {
        if constexpr (requires { value.bsatn_serialize(writer); }) {
            value.bsatn_serialize(writer);
        } else {
            static_assert(false, "Type must implement bsatn_serialize or specialize bsatn_traits");
        }
    }
    
    static T deserialize(Reader& reader) {
        if constexpr (requires { T::bsatn_deserialize(reader); }) {
            return T::bsatn_deserialize(reader);
        } else {
            static_assert(false, "Type must implement bsatn_deserialize or specialize bsatn_traits");
        }
    }
    
    static AlgebraicType algebraic_type() {
        return algebraic_type_of<T>::get();
    }
};

/**
 * Interface for types that can serialize their fields individually.
 * This is similar to C#'s IStructuralReadWrite.
 */
class IStructuralSerializable {
public:
    virtual ~IStructuralSerializable() = default;
    
    virtual void write_fields(Writer& writer) const = 0;
    virtual void read_fields(Reader& reader) = 0;
    virtual AlgebraicType get_algebraic_type() const = 0;
};

/**
 * CRTP base class that provides default BSATN serialization for structs.
 * Derive from this and implement the required methods.
 */
template<typename Derived>
class StructuralSerializable : public IStructuralSerializable {
public:
    // Serialize the entire object
    void bsatn_serialize(Writer& writer) const {
        write_fields(writer);
    }
    
    // Deserialize into this object
    void bsatn_deserialize(Reader& reader) {
        read_fields(reader);
    }
    
    // Static deserialize method
    static Derived deserialize(Reader& reader) {
        Derived result;
        result.read_fields(reader);
        return result;
    }
};

// ITypeRegistrar is defined in ITypeRegistrar.h

/**
 * Helper class for building product types with named fields.
 */
class ProductTypeBuilder {
private:
    std::vector<ProductTypeElement> elements_;
    ITypeRegistrar* registrar_;
    
public:
    explicit ProductTypeBuilder(ITypeRegistrar* registrar = nullptr) 
        : registrar_(registrar) {}
    
    template<typename T>
    ProductTypeBuilder& with_field(const std::string& name) {
        uint32_t type_id = registrar_ ? 
            registrar_->register_type(bsatn_traits<T>::algebraic_type()) : 0;
        elements_.emplace_back(name, type_id);
        return *this;
    }
    
    template<typename T>
    ProductTypeBuilder& with_unnamed_field() {
        uint32_t type_id = registrar_ ? 
            registrar_->register_type(bsatn_traits<T>::algebraic_type()) : 0;
        elements_.emplace_back(std::nullopt, type_id);
        return *this;
    }
    
    std::unique_ptr<ProductType> build() {
        return std::make_unique<ProductType>(std::move(elements_));
    }
};

/**
 * Helper class for building sum types (enums with data).
 */
class SumTypeBuilder {
private:
    std::vector<SumTypeVariant> variants_;
    ITypeRegistrar* registrar_;
    
public:
    explicit SumTypeBuilder(ITypeRegistrar* registrar = nullptr) 
        : registrar_(registrar) {}
    
    template<typename T>
    SumTypeBuilder& with_variant(const std::string& name) {
        uint32_t type_id = registrar_ ? 
            registrar_->register_type(bsatn_traits<T>::algebraic_type()) : 0;
        variants_.emplace_back(name, type_id);
        return *this;
    }
    
    SumTypeBuilder& with_unit_variant(const std::string& name) {
        // Unit variant has no data
        variants_.emplace_back(name, 0);
        return *this;
    }
    
    std::unique_ptr<SumTypeSchema> build() {
        return std::make_unique<SumTypeSchema>(std::move(variants_));
    }
};

/**
 * Macro to generate BSATN serialization for a simple struct.
 * Usage:
 *   struct MyStruct {
 *       int32_t x;
 *       std::string name;
 *   };
 *   SPACETIMEDB_BSATN_STRUCT(MyStruct, x, name)
 */
#define SPACETIMEDB_BSATN_STRUCT(Type, ...) \
    template<> \
    struct SpacetimeDb::bsatn::bsatn_traits<Type> { \
        static void serialize(Writer& w, const Type& v) { \
            SPACETIMEDB_BSATN_SERIALIZE_FIELDS(v, w, __VA_ARGS__) \
        } \
        static Type deserialize(Reader& r) { \
            Type v; \
            SPACETIMEDB_BSATN_DESERIALIZE_FIELDS(v, r, __VA_ARGS__) \
            return v; \
        } \
        static AlgebraicType algebraic_type() { \
            ProductTypeBuilder builder; \
            SPACETIMEDB_BSATN_REGISTER_FIELDS(Type, builder, __VA_ARGS__) \
            return AlgebraicType::make_product(builder.build()); \
        } \
    };

// Helper macros for field serialization
#define SPACETIMEDB_BSATN_SERIALIZE_FIELD(obj, writer, field) \
    SpacetimeDb::bsatn::serialize(writer, obj.field);

#define SPACETIMEDB_BSATN_DESERIALIZE_FIELD(obj, reader, field) \
    obj.field = SpacetimeDb::bsatn::deserialize<decltype(obj.field)>(reader);

#define SPACETIMEDB_BSATN_REGISTER_FIELD(Type, builder, field) \
    builder.with_field<decltype(Type::field)>(#field);

// Expand macros for each field
#define SPACETIMEDB_BSATN_SERIALIZE_FIELDS(obj, writer, ...) \
    SPACETIMEDB_FOR_EACH_ARG(SPACETIMEDB_BSATN_SERIALIZE_FIELD, obj, writer, __VA_ARGS__)

#define SPACETIMEDB_BSATN_DESERIALIZE_FIELDS(obj, reader, ...) \
    SPACETIMEDB_FOR_EACH_ARG(SPACETIMEDB_BSATN_DESERIALIZE_FIELD, obj, reader, __VA_ARGS__)

#define SPACETIMEDB_BSATN_REGISTER_FIELDS(Type, builder, ...) \
    SPACETIMEDB_FOR_EACH_ARG(SPACETIMEDB_BSATN_REGISTER_FIELD, Type, builder, __VA_ARGS__)

// Macro utilities for variadic expansion
#define SPACETIMEDB_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define SPACETIMEDB_FOR_EACH_ARG(MACRO, obj, extra, ...) \
    SPACETIMEDB_GET_MACRO(__VA_ARGS__, \
        SPACETIMEDB_FE_8, SPACETIMEDB_FE_7, SPACETIMEDB_FE_6, SPACETIMEDB_FE_5, \
        SPACETIMEDB_FE_4, SPACETIMEDB_FE_3, SPACETIMEDB_FE_2, SPACETIMEDB_FE_1) \
    (MACRO, obj, extra, __VA_ARGS__)

#define SPACETIMEDB_FE_1(MACRO, obj, extra, X) MACRO(obj, extra, X)
#define SPACETIMEDB_FE_2(MACRO, obj, extra, X, ...) MACRO(obj, extra, X) SPACETIMEDB_FE_1(MACRO, obj, extra, __VA_ARGS__)
#define SPACETIMEDB_FE_3(MACRO, obj, extra, X, ...) MACRO(obj, extra, X) SPACETIMEDB_FE_2(MACRO, obj, extra, __VA_ARGS__)
#define SPACETIMEDB_FE_4(MACRO, obj, extra, X, ...) MACRO(obj, extra, X) SPACETIMEDB_FE_3(MACRO, obj, extra, __VA_ARGS__)
#define SPACETIMEDB_FE_5(MACRO, obj, extra, X, ...) MACRO(obj, extra, X) SPACETIMEDB_FE_4(MACRO, obj, extra, __VA_ARGS__)
#define SPACETIMEDB_FE_6(MACRO, obj, extra, X, ...) MACRO(obj, extra, X) SPACETIMEDB_FE_5(MACRO, obj, extra, __VA_ARGS__)
#define SPACETIMEDB_FE_7(MACRO, obj, extra, X, ...) MACRO(obj, extra, X) SPACETIMEDB_FE_6(MACRO, obj, extra, __VA_ARGS__)
#define SPACETIMEDB_FE_8(MACRO, obj, extra, X, ...) MACRO(obj, extra, X) SPACETIMEDB_FE_7(MACRO, obj, extra, __VA_ARGS__)

} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_TRAITS_H