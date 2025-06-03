#ifndef SPACETIMEDB_INTERNAL_MODULE_DEF_H // Renamed header guard
#define SPACETIMEDB_INTERNAL_MODULE_DEF_H

#include "spacetimedb/bsatn_all.h" // For bsatn::Writer
#include "spacetimedb/internal/module_schema.h" // For SpacetimeDb::ModuleSchema (used in build_internal_module_def)

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <variant>
#include <cstdint>
#include <memory> // For std::unique_ptr

// Forward declare bsatn::Writer if only used in function signatures here
// However, since we define serialize functions here, it's better to include the full header.
// namespace bsatn { class Writer; }

namespace SpacetimeDb {
    namespace Internal {

        // Corresponds to spacetimedb_schema::identifier::Identifier / ScopedTypeName (simplified)
        using Identifier = std::string;
        using ScopedTypeName = std::string;

        // Corresponds to spacetimedb_schema::types::PrimitiveType
        enum class InternalPrimitiveType : uint8_t {
            Unit = 0, Bool = 1, U8 = 2, U16 = 3, U32 = 4, U64 = 5, U128 = 6,
            I8 = 7, I16 = 8, I32 = 9, I64 = 10, I128 = 11,
            F32 = 12, F64 = 13, String = 14, Bytes = 15,
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalPrimitiveType& value);

        struct InternalType {
            enum class Kind : uint8_t {
                Primitive = 0, UserDefined = 1, Option = 2, Vector = 3,
            };

            Kind kind;
            InternalPrimitiveType primitive_type;
            ScopedTypeName user_defined_name;
            std::unique_ptr<InternalType> element_type;

            InternalType() : kind(Kind::Primitive), primitive_type(InternalPrimitiveType::Unit), element_type(nullptr) {}
            // Copy constructor and assignment for unique_ptr handling
            InternalType(const InternalType& other);
            InternalType& operator=(const InternalType& other);
            // Move constructor and assignment (defaulted is fine for unique_ptr)
            InternalType(InternalType&& other) = default;
            InternalType& operator=(InternalType&& other) = default;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalType& type);

        struct InternalFieldDef {
            Identifier name;
            InternalType ty;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalFieldDef& def);

        struct InternalEnumVariantDef {
            Identifier name;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalEnumVariantDef& def);

        struct InternalStructDef {
            std::vector<InternalFieldDef> fields;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalStructDef& def);

        struct InternalEnumDef {
            std::vector<InternalEnumVariantDef> variants;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalEnumDef& def);

        enum class InternalTypeDefVariantKind : uint8_t {
            Struct = 0, Enum = 1,
        };

        struct InternalTypeDef {
            ScopedTypeName name;
            InternalTypeDefVariantKind variant_kind;
            InternalStructDef struct_def; // Valid if Struct
            InternalEnumDef enum_def;     // Valid if Enum
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalTypeDef& def);

        struct InternalTableDef {
            Identifier name;
            ScopedTypeName row_type_name;
            std::optional<Identifier> primary_key_field_name;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalTableDef& def);

        struct InternalReducerParameterDef {
            Identifier name;
            InternalType ty;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalReducerParameterDef& def);

        struct InternalReducerDef {
            Identifier name;
            std::vector<InternalReducerParameterDef> parameters;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalReducerDef& def);

        struct InternalModuleDef {
            Identifier name;
            std::vector<InternalTypeDef> types;
            std::vector<InternalTableDef> tables;
            std::vector<InternalReducerDef> reducers;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const InternalModuleDef& def);

        // Declaration for the builder function (implementation in module_def_builder.cpp)
        InternalModuleDef build_internal_module_def(const SpacetimeDb::ModuleSchema& user_schema);

        // Declaration for the top-level serializer (implementation in module_def_builder.cpp)
        // This is what get_serialized_module_definition_bytes will call.
        // void serialize_module_def(bsatn::Writer& writer, const InternalModuleDef& def); // This is an alternative to free serialize()

        // Declaration for getting the final bytes (implementation in module_def_builder.cpp)
        std::vector<std::byte> get_serialized_module_definition_bytes();

        // Implementation for InternalType copy constructor/assignment
        inline InternalType::InternalType(const InternalType& other) :
            kind(other.kind),
            primitive_type(other.primitive_type),
            user_defined_name(other.user_defined_name) {
            if (other.element_type) {
                element_type = std::make_unique<InternalType>(*other.element_type);
            }
        }

        inline InternalType& InternalType::operator=(const InternalType& other) {
            if (this == &other) return *this;
            kind = other.kind;
            primitive_type = other.primitive_type;
            user_defined_name = other.user_defined_name;
            if (other.element_type) {
                element_type = std::make_unique<InternalType>(*other.element_type);
            }
            else {
                element_type.reset();
            }
            return *this;
        }


    } // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_MODULE_DEF_H
