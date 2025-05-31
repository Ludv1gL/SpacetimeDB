#ifndef SPACETIME_MODULE_DEF_H
#define SPACETIME_MODULE_DEF_H

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <variant>
#include <cstdint>
#include "bsatn_lib.h" // Assumed to provide bsatn::Writer

// Based on spacetimedb-schema/src/def.rs and spacetimedb-schema/src/types.rs

namespace SpacetimeDB {
namespace Internal {

// Corresponds to spacetimedb_schema::identifier::Identifier / ScopedTypeName (simplified)
using Identifier = std::string;
using ScopedTypeName = std::string; // In full schema, this has scope (module name) and name

// Corresponds to spacetimedb_schema::types::PrimitiveType
enum class InternalPrimitiveType : uint8_t {
    Unit = 0,
    Bool = 1,
    U8 = 2,
    U16 = 3,
    U32 = 4,
    U64 = 5,
    U128 = 6,
    I8 = 7,
    I16 = 8,
    I32 = 9,
    I64 = 10,
    I128 = 11,
    F32 = 12,
    F64 = 13,
    String = 14,
    Bytes = 15,
    // U256, I256, Address, Identity, Timestamp are extensions or aliases often built on these
};

void serialize(bsatn::Writer& writer, const InternalPrimitiveType& value);

// Corresponds to spacetimedb_schema::types::Type
struct InternalType {
    // Simplified: directly embed primitive or user-defined name.
    // Real schema has Type::Primitive(PrimitiveType) or Type::UserDefined(ScopedTypeName)
    // and also option/vector wrappers.
    // For ModuleDef, types are usually referenced by ScopedTypeName.
    // Let's define it more like the schema's `Type` enum.

    enum class Kind : uint8_t {
        Primitive = 0,
        UserDefined = 1,
        Option = 2,
        Vector = 3,
        // Map = 4, // If maps are supported directly in schema def
    };

    Kind kind;
    // Only one of these will be valid based on `kind`
    InternalPrimitiveType primitive_type; // Valid if kind == Primitive
    ScopedTypeName user_defined_name;    // Valid if kind == UserDefined
    std::unique_ptr<InternalType> element_type; // Valid if kind == Option or Vector

    // Default constructor to avoid uninitialized members
    InternalType() : kind(Kind::Primitive), primitive_type(InternalPrimitiveType::Unit), element_type(nullptr) {}
};

void serialize(bsatn::Writer& writer, const InternalType& type);


// Corresponds to spacetimedb_schema::def::FieldDef
struct InternalFieldDef {
    Identifier name;
    InternalType ty;
    // bool is_optional; // This is now handled by InternalType::Option
};

void serialize(bsatn::Writer& writer, const InternalFieldDef& def);

// Corresponds to spacetimedb_schema::def::EnumVariantDef
struct InternalEnumVariantDef {
    Identifier name;
    // uint32_t value; // If explicit discriminant values are part of schema
    // SpacetimeDB enums in schema typically don't have explicit values, index is used.
    // If variants can hold types (like Rust enums), that's more complex (SumTypeDef::Union)
    // For simple C-style enums (SumTypeDef::Enum), just the name is needed.
};

void serialize(bsatn::Writer& writer, const InternalEnumVariantDef& def);

// Corresponds to spacetimedb_schema::def::ProductTypeDef::Struct
struct InternalStructDef {
    std::vector<InternalFieldDef> fields;
};

void serialize(bsatn::Writer& writer, const InternalStructDef& def);

// Corresponds to spacetimedb_schema::def::SumTypeDef::Enum (simple enum)
struct InternalEnumDef {
    std::vector<InternalEnumVariantDef> variants;
};

void serialize(bsatn::Writer& writer, const InternalEnumDef& def);


// Corresponds to spacetimedb_schema::def::TypeDefVariant (simplified)
// In schema, TypeDefVariant has Product(ProductTypeDef) and Sum(SumTypeDef)
// ProductTypeDef can be Struct or Tuple. SumTypeDef can be Enum or Union.
// We'll simplify here: a TypeDef is either a Struct or an Enum.
// Tuples can be represented as structs with generic field names.
// Unions (sum types with data) are more complex and would need a different structure.
// For ModuleDef generation from macros, we primarily deal with structs and simple enums.
enum class InternalTypeDefVariantKind : uint8_t {
    Struct = 0,
    Enum = 1,
    // Union = 2, // For sum types with data per variant
    // Tuple = 3, // Can be represented as struct
};

struct InternalTypeDef {
    ScopedTypeName name; // The C++ name, for ModuleDef it's the SpacetimeDB name
    InternalTypeDefVariantKind variant_kind;
    // Only one of these is valid based on variant_kind
    InternalStructDef struct_def;
    InternalEnumDef enum_def;
};

void serialize(bsatn::Writer& writer, const InternalTypeDef& def);

// Corresponds to spacetimedb_schema::def::TableDef
struct InternalTableDef {
    Identifier name; // SpacetimeDB name of the table
    ScopedTypeName row_type_name; // Name of the C++ struct used for rows (must be a registered TypeDef)
    std::optional<Identifier> primary_key_field_name;
    // std::vector<IndexDef> secondary_indexes; // If supported
};

void serialize(bsatn::Writer& writer, const InternalTableDef& def);

// Corresponds to spacetimedb_schema::def::ReducerDef's parameter
struct InternalReducerParameterDef {
    Identifier name;
    InternalType ty;
};

void serialize(bsatn::Writer& writer, const InternalReducerParameterDef& def);

// Corresponds to spacetimedb_schema::def::ReducerDef
struct InternalReducerDef {
    Identifier name; // SpacetimeDB name of the reducer
    std::vector<InternalReducerParameterDef> parameters;
    // ScopedTypeName return_type; // If reducers have return types in schema
};

void serialize(bsatn::Writer& writer, const InternalReducerDef& def);

// Corresponds to spacetimedb_schema::def::ModuleDef
struct InternalModuleDef {
    Identifier name; // Typically "module" or the crate name
    std::vector<InternalTypeDef> types;
    std::vector<InternalTableDef> tables;
    std::vector<InternalReducerDef> reducers;
    // Checksum or version info could be added
};

void serialize(bsatn::Writer& writer, const InternalModuleDef& def);


// Function to build the InternalModuleDef from the user-facing ModuleSchema
// Declaration
InternalModuleDef build_internal_module_def(const SpacetimeDb::ModuleSchema& user_schema);

// Function to get the fully serialized module definition
// Declaration
std::vector<std::byte> get_serialized_module_definition_bytes();

} // namespace Internal
} // namespace SpacetimeDB

#endif // SPACETIME_MODULE_DEF_H
