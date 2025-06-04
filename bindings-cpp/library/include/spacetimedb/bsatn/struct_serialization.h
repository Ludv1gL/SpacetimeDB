#ifndef SPACETIMEDB_BSATN_STRUCT_SERIALIZATION_H
#define SPACETIMEDB_BSATN_STRUCT_SERIALIZATION_H

#include "bsatn.h"
#include "traits.h"
#include "algebraic_type.h"

namespace SpacetimeDb::bsatn {

/**
 * Macros for generating BSATN serialization for user-defined structs.
 * This provides automatic serialization similar to C#'s source generators.
 * 
 * Usage:
 *   struct MyStruct {
 *       uint32_t id;
 *       std::string name;
 *       uint8_t age;
 *   };
 *   
 *   SPACETIMEDB_BSATN_STRUCT_BEGIN(MyStruct)
 *       SPACETIMEDB_BSATN_FIELD(id)
 *       SPACETIMEDB_BSATN_FIELD(name)
 *       SPACETIMEDB_BSATN_FIELD(age)
 *   SPACETIMEDB_BSATN_STRUCT_END()
 */

// Helper macro to serialize a single field
#define SPACETIMEDB_BSATN_SERIALIZE_FIELD(field_name) \
    serialize(writer, value.field_name);

// Helper macro to deserialize a single field
#define SPACETIMEDB_BSATN_DESERIALIZE_FIELD(field_name) \
    result.field_name = deserialize<decltype(result.field_name)>(reader);

// Helper macro to add field to algebraic type
#define SPACETIMEDB_BSATN_ADD_FIELD_TYPE(field_name) \
    elements.emplace_back(#field_name, registrar.register_type(algebraic_type_of<decltype(std::declval<StructType>().field_name)>::get()));

// Begin struct serialization definition
#define SPACETIMEDB_BSATN_STRUCT_BEGIN(StructType) \
    template<> \
    struct bsatn_traits<StructType> { \
        static void serialize(Writer& writer, const StructType& value) { \
            /* Serialize each field in order */

// Define a field for serialization
#define SPACETIMEDB_BSATN_FIELD(field_name) \
            SpacetimeDb::bsatn::serialize(writer, value.field_name);

// End struct serialization definition and add deserialize + algebraic_type methods
#define SPACETIMEDB_BSATN_STRUCT_END() \
        } \
        \
        static auto deserialize(Reader& reader) { \
            std::remove_const_t<std::remove_reference_t<decltype(*static_cast<bsatn_traits*>(nullptr))>> result; \
            return deserialize_impl(reader, result); \
        } \
        \
        template<typename T> \
        static T deserialize_impl(Reader& reader, T& result) { \
            return deserialize_fields(reader, result); \
        } \
        \
        static AlgebraicType algebraic_type() { \
            return make_product_type(); \
        } \
        \
    private: \
        template<typename T> \
        static T deserialize_fields(Reader& reader, T& result); \
        \
        static AlgebraicType make_product_type(); \
    };

/**
 * Alternative approach: Use a more flexible field-by-field macro system
 */

// Variadic macro approach for better ergonomics
#define SPACETIMEDB_BSATN_STRUCT(StructType, ...) \
    template<> \
    struct bsatn_traits<StructType> { \
        static void serialize(Writer& writer, const StructType& value) { \
            SPACETIMEDB_BSATN_SERIALIZE_FIELDS(value, __VA_ARGS__) \
        } \
        \
        static StructType deserialize(Reader& reader) { \
            StructType result; \
            SPACETIMEDB_BSATN_DESERIALIZE_FIELDS(result, __VA_ARGS__) \
            return result; \
        } \
        \
        static AlgebraicType algebraic_type() { \
            std::vector<AggregateElement> elements; \
            SPACETIMEDB_BSATN_BUILD_TYPE(StructType, elements, __VA_ARGS__) \
            return AlgebraicType::make_product(std::make_unique<ProductType>(std::move(elements))); \
        } \
    };

// Helper macros for variadic field processing
#define SPACETIMEDB_BSATN_SERIALIZE_FIELDS(value, ...) \
    SPACETIMEDB_BSATN_FOR_EACH(SPACETIMEDB_BSATN_SERIALIZE_FIELD_HELPER, value, __VA_ARGS__)

#define SPACETIMEDB_BSATN_DESERIALIZE_FIELDS(result, ...) \
    SPACETIMEDB_BSATN_FOR_EACH(SPACETIMEDB_BSATN_DESERIALIZE_FIELD_HELPER, result, __VA_ARGS__)

#define SPACETIMEDB_BSATN_BUILD_TYPE(StructType, elements, ...) \
    SPACETIMEDB_BSATN_FOR_EACH(SPACETIMEDB_BSATN_ADD_TYPE_HELPER, StructType, elements, __VA_ARGS__)

#define SPACETIMEDB_BSATN_SERIALIZE_FIELD_HELPER(value, field_name) \
    SpacetimeDb::bsatn::serialize(writer, value.field_name);

#define SPACETIMEDB_BSATN_DESERIALIZE_FIELD_HELPER(result, field_name) \
    result.field_name = SpacetimeDb::bsatn::deserialize<decltype(result.field_name)>(reader);

#define SPACETIMEDB_BSATN_ADD_TYPE_HELPER(StructType, elements, field_name) \
    elements.emplace_back(#field_name, /* type_id will be handled by TypeRegistry */);

// Macro for-each implementation (supports up to 10 fields)
#define SPACETIMEDB_BSATN_FOR_EACH(macro, ...) \
    SPACETIMEDB_BSATN_GET_MACRO(__VA_ARGS__, \
        SPACETIMEDB_BSATN_FOR_EACH_10, SPACETIMEDB_BSATN_FOR_EACH_9, \
        SPACETIMEDB_BSATN_FOR_EACH_8, SPACETIMEDB_BSATN_FOR_EACH_7, \
        SPACETIMEDB_BSATN_FOR_EACH_6, SPACETIMEDB_BSATN_FOR_EACH_5, \
        SPACETIMEDB_BSATN_FOR_EACH_4, SPACETIMEDB_BSATN_FOR_EACH_3, \
        SPACETIMEDB_BSATN_FOR_EACH_2, SPACETIMEDB_BSATN_FOR_EACH_1)(macro, __VA_ARGS__)

#define SPACETIMEDB_BSATN_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME

#define SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, a) macro(ctx, a)
#define SPACETIMEDB_BSATN_FOR_EACH_2(macro, ctx, a, b) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, a) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, b)
#define SPACETIMEDB_BSATN_FOR_EACH_3(macro, ctx, a, b, c) \
    SPACETIMEDB_BSATN_FOR_EACH_2(macro, ctx, a, b) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, c)
#define SPACETIMEDB_BSATN_FOR_EACH_4(macro, ctx, a, b, c, d) \
    SPACETIMEDB_BSATN_FOR_EACH_3(macro, ctx, a, b, c) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, d)
#define SPACETIMEDB_BSATN_FOR_EACH_5(macro, ctx, a, b, c, d, e) \
    SPACETIMEDB_BSATN_FOR_EACH_4(macro, ctx, a, b, c, d) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, e)
#define SPACETIMEDB_BSATN_FOR_EACH_6(macro, ctx, a, b, c, d, e, f) \
    SPACETIMEDB_BSATN_FOR_EACH_5(macro, ctx, a, b, c, d, e) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, f)
#define SPACETIMEDB_BSATN_FOR_EACH_7(macro, ctx, a, b, c, d, e, f, g) \
    SPACETIMEDB_BSATN_FOR_EACH_6(macro, ctx, a, b, c, d, e, f) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, g)
#define SPACETIMEDB_BSATN_FOR_EACH_8(macro, ctx, a, b, c, d, e, f, g, h) \
    SPACETIMEDB_BSATN_FOR_EACH_7(macro, ctx, a, b, c, d, e, f, g) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, h)
#define SPACETIMEDB_BSATN_FOR_EACH_9(macro, ctx, a, b, c, d, e, f, g, h, i) \
    SPACETIMEDB_BSATN_FOR_EACH_8(macro, ctx, a, b, c, d, e, f, g, h) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, i)
#define SPACETIMEDB_BSATN_FOR_EACH_10(macro, ctx, a, b, c, d, e, f, g, h, i, j) \
    SPACETIMEDB_BSATN_FOR_EACH_9(macro, ctx, a, b, c, d, e, f, g, h, i) \
    SPACETIMEDB_BSATN_FOR_EACH_1(macro, ctx, j)

/**
 * Simpler manual approach for small structs (recommended for now)
 */
#define SPACETIMEDB_BSATN_STRUCT_SIMPLE(StructType) \
    namespace SpacetimeDb::bsatn { \
        template<> \
        struct bsatn_traits<StructType>; \
    } \
    template<> \
    struct SpacetimeDb::bsatn::bsatn_traits<StructType> { \
        static void serialize(Writer& writer, const StructType& value); \
        static StructType deserialize(Reader& reader); \
        static AlgebraicType algebraic_type(); \
    };

} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_STRUCT_SERIALIZATION_H