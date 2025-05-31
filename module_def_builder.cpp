#include "spacetime_module_def.h"
#include "spacetime_schema.h" // For SpacetimeDb::ModuleSchema and its contained types
#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::transform

// Helper function to convert SpacetimeDb::CoreType to SpacetimeDB::Internal::InternalPrimitiveType
SpacetimeDB::Internal::InternalPrimitiveType map_core_type_to_internal_primitive(SpacetimeDb::CoreType core_type) {
    using InternalPT = SpacetimeDB::Internal::InternalPrimitiveType;
    switch (core_type) {
        case SpacetimeDb::CoreType::Bool: return InternalPT::Bool;
        case SpacetimeDb::CoreType::U8:   return InternalPT::U8;
        case SpacetimeDb::CoreType::U16:  return InternalPT::U16;
        case SpacetimeDb::CoreType::U32:  return InternalPT::U32;
        case SpacetimeDb::CoreType::U64:  return InternalPT::U64;
        case SpacetimeDb::CoreType::U128: return InternalPT::U128;
        case SpacetimeDb::CoreType::I8:   return InternalPT::I8;
        case SpacetimeDb::CoreType::I16:  return InternalPT::I16;
        case SpacetimeDb::CoreType::I32:  return InternalPT::I32;
        case SpacetimeDb::CoreType::I64:  return InternalPT::I64;
        case SpacetimeDb::CoreType::I128: return InternalPT::I128;
        case SpacetimeDb::CoreType::F32:  return InternalPT::F32;
        case SpacetimeDb::CoreType::F64:  return InternalPT::F64;
        case SpacetimeDb::CoreType::String: return InternalPT::String;
        case SpacetimeDb::CoreType::Bytes:  return InternalPT::Bytes;
        // SpacetimeDb::CoreType::UserDefined is not a primitive.
        // SpacetimeDb::CoreType::Unit could be InternalPrimitiveType::Unit if that's added.
        default:
            throw std::runtime_error("Unsupported SpacetimeDb::CoreType for primitive mapping: " + std::to_string(static_cast<int>(core_type)));
    }
}

// Helper function to convert SpacetimeDb::TypeIdentifier to SpacetimeDB::Internal::InternalType
SpacetimeDB::Internal::InternalType map_type_identifier_to_internal_type(
    const SpacetimeDb::TypeIdentifier& type_id,
    const SpacetimeDb::ModuleSchema& user_schema // Needed to check if a user_defined_name is an Option or Vector "alias"
) {
    SpacetimeDB::Internal::InternalType internal_ty;

    // This simplified mapping assumes that if type_id.user_defined_name is "Option" or "Vector",
    // it's a direct reference to a conceptual Option or Vector, not a user-defined struct *named* "Option".
    // The SpacetimeDB SATS schema parser resolves these to specific Type variants (Type::Option, Type::Vector).
    // Our ModuleSchema from macros doesn't explicitly store this distinction yet for user_defined_name.
    // For now, we'll assume user_defined_name is for actual custom structs/enums.
    // Optionals are handled by FieldDefinition::is_optional. Vectors are not explicitly handled by TypeIdentifier alone.
    // This part needs careful alignment with how the schema is meant to be structured.
    // The InternalType struct *does* have Kind::Option and Kind::Vector.

    // Let's refine: The schema definition from macros uses `is_optional` on FieldDefinition.
    // And `SPACETIMEDB_TYPE_VECTOR` would be needed if we were to support vector fields directly in this manner.
    // For now, let's assume TypeIdentifier mostly maps to Primitive or UserDefined.
    // We will construct Option/Vector kinds if the context dictates (e.g. field.is_optional).

    if (type_id.core_type == SpacetimeDb::CoreType::UserDefined) {
        // Check if this "user_defined_name" corresponds to a type that should be treated as Option or Vector
        // This logic is a bit of a placeholder as the macro schema doesn't fully qualify options/vectors
        // in TypeIdentifier itself, but rather uses `is_optional` or would need a specific VectorType.
        // For now, assume user_defined_name directly maps to a type name.
        internal_ty.kind = SpacetimeDB::Internal::InternalType::Kind::UserDefined;
        internal_ty.user_defined_name = type_id.user_defined_name; // This should be the SpacetimeDB name
    } else {
        internal_ty.kind = SpacetimeDB::Internal::InternalType::Kind::Primitive;
        internal_ty.primitive_type = map_core_type_to_internal_primitive(type_id.core_type);
    }
    return internal_ty;
}


// Overloaded helper for fields which might be optional
SpacetimeDB::Internal::InternalType map_field_type_to_internal_type(
    const SpacetimeDb::FieldDefinition& field_def,
    const SpacetimeDb::ModuleSchema& user_schema
) {
    SpacetimeDB::Internal::InternalType element_type = map_type_identifier_to_internal_type(field_def.type, user_schema);
    if (field_def.is_optional) {
        SpacetimeDB::Internal::InternalType option_type;
        option_type.kind = SpacetimeDB::Internal::InternalType::Kind::Option;
        option_type.element_type = std::make_unique<SpacetimeDB::Internal::InternalType>(std::move(element_type));
        return option_type;
    }
    return element_type;
}


SpacetimeDB::Internal::InternalModuleDef SpacetimeDB::Internal::build_internal_module_def(
    const SpacetimeDb::ModuleSchema& user_schema) {
    InternalModuleDef module_def_internal;
    module_def_internal.name = "module"; // Default module name, can be configured if needed

    // Convert Types
    for (const auto& pair : user_schema.types) {
        const SpacetimeDb::TypeDefinition& user_type_def = pair.second;
        InternalTypeDef internal_type_def;
        internal_type_def.name = user_type_def.spacetime_db_name; // Use SpacetimeDB name

        if (std::holds_alternative<SpacetimeDb::StructDefinition>(user_type_def.definition)) {
            const SpacetimeDb::StructDefinition& struct_def_user = std::get<SpacetimeDb::StructDefinition>(user_type_def.definition);
            internal_type_def.variant_kind = InternalTypeDefVariantKind::Struct;

            for (const auto& field_user : struct_def_user.fields) {
                InternalFieldDef field_internal;
                field_internal.name = field_user.name;
                field_internal.ty = map_field_type_to_internal_type(field_user, user_schema);
                internal_type_def.struct_def.fields.push_back(field_internal);
            }
        } else if (std::holds_alternative<SpacetimeDb::EnumDefinition>(user_type_def.definition)) {
            const SpacetimeDb::EnumDefinition& enum_def_user = std::get<SpacetimeDb::EnumDefinition>(user_type_def.definition);
            internal_type_def.variant_kind = InternalTypeDefVariantKind::Enum;

            for (const auto& variant_user : enum_def_user.variants) {
                InternalEnumVariantDef variant_internal;
                variant_internal.name = variant_user.name;
                internal_type_def.enum_def.variants.push_back(variant_internal);
            }
        }
        module_def_internal.types.push_back(internal_type_def);
    }

    // Convert Tables
    for (const auto& pair : user_schema.tables) {
        const SpacetimeDb::TableDefinition& table_def_user = pair.second;
        InternalTableDef table_def_internal;
        table_def_internal.name = table_def_user.spacetime_name;

        // Find the SpacetimeDB name for the row type
        auto it_type = user_schema.types.find(table_def_user.cpp_row_type_name);
        if (it_type != user_schema.types.end()) {
            table_def_internal.row_type_name = it_type->second.spacetime_db_name;
        } else {
            // This should not happen if schema is consistent
            throw std::runtime_error("Row type '" + table_def_user.cpp_row_type_name + "' not found in schema for table '" + table_def_user.spacetime_name + "'.");
        }

        if (!table_def_user.primary_key_field_name.empty()) {
            table_def_internal.primary_key_field_name = table_def_user.primary_key_field_name;
        }
        module_def_internal.tables.push_back(table_def_internal);
    }

    // Convert Reducers
    for (const auto& pair : user_schema.reducers) {
        const SpacetimeDb::ReducerDefinition& reducer_def_user = pair.second;
        InternalReducerDef reducer_def_internal;
        reducer_def_internal.name = reducer_def_user.spacetime_name;

        for (const auto& param_user : reducer_def_user.parameters) {
            InternalReducerParameterDef param_internal;
            param_internal.name = param_user.name;
            // For reducer parameters, `is_optional` is not directly on ReducerParameterDefinition.
            // If a reducer parameter can be optional, its TypeIdentifier would need to reflect that,
            // or the mapping logic here would need context if options are specified differently for reducers.
            // Assuming ReducerParameterDefinition's type is already final (e.g. an `Option<T>` if applicable)
            // The current map_type_identifier_to_internal_type is basic and doesn't create Option/Vector kinds.
            // This needs to be more robust.
            // Let's assume for now parameters are not optional unless their TypeIdentifier somehow encodes it.
            // For now, use the direct mapping for parameters. If a parameter is optional, its SpacetimeDb::TypeIdentifier
            // would need to be structured to represent an Option<T>, which the current macro setup doesn't fully do.
            // This is a limitation of the current macro schema design's TypeIdentifier for params.
            // A temporary fix: assume reducer params are not optional for this mapping.
            // A better fix: enhance SpacetimeDb::ReducerParameterDefinition to include an is_optional flag or richer type info.
            SpacetimeDb::FieldDefinition temp_field_for_mapping;
            temp_field_for_mapping.name = param_user.name;
            temp_field_for_mapping.type = param_user.type;
            temp_field_for_mapping.is_optional = false; // Explicitly false for now for reducer params
            param_internal.ty = map_field_type_to_internal_type(temp_field_for_mapping, user_schema);

            reducer_def_internal.parameters.push_back(param_internal);
        }
        module_def_internal.reducers.push_back(reducer_def_internal);
    }

    return module_def_internal;
}


// BSATN Serialization Implementations
// These need to match spacetimedb-schema/src/def.rs format

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalPrimitiveType& value) {
    writer.write_u8(static_cast<uint8_t>(value));
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalType& type) {
    writer.write_u8(static_cast<uint8_t>(type.kind));
    switch (type.kind) {
        case InternalType::Kind::Primitive:
            serialize(writer, type.primitive_type);
            break;
        case InternalType::Kind::UserDefined:
            writer.write_string(type.user_defined_name); // Assuming ScopedTypeName is string
            break;
        case InternalType::Kind::Option:
        case InternalType::Kind::Vector:
            if (!type.element_type) throw std::runtime_error("Option/Vector element_type is null");
            serialize(writer, *type.element_type);
            break;
        // case InternalType::Kind::Map: // If maps are supported
            // serialize(writer, *type.key_type);   // Assuming key_type field exists
            // serialize(writer, *type.value_type); // Assuming value_type field exists
            // break;
        default:
            throw std::runtime_error("Unknown InternalType::Kind for serialization");
    }
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalFieldDef& def) {
    writer.write_string(def.name);
    serialize(writer, def.ty);
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalEnumVariantDef& def) {
    writer.write_string(def.name);
    // If variants had explicit discriminant values: writer.write_u32_le(def.value);
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalStructDef& def) {
    writer.write_u32_le(static_cast<uint32_t>(def.fields.size()));
    for (const auto& field : def.fields) {
        serialize(writer, field);
    }
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalEnumDef& def) {
    writer.write_u32_le(static_cast<uint32_t>(def.variants.size()));
    for (const auto& variant : def.variants) {
        serialize(writer, variant);
    }
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalTypeDef& def) {
    writer.write_string(def.name); // ScopedTypeName
    writer.write_u8(static_cast<uint8_t>(def.variant_kind));
    switch (def.variant_kind) {
        case InternalTypeDefVariantKind::Struct:
            serialize(writer, def.struct_def);
            break;
        case InternalTypeDefVariantKind::Enum:
            serialize(writer, def.enum_def);
            break;
        default:
            throw std::runtime_error("Unknown InternalTypeDefVariantKind for serialization");
    }
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalTableDef& def) {
    writer.write_string(def.name);
    writer.write_string(def.row_type_name); // ScopedTypeName

    bool has_pk = def.primary_key_field_name.has_value();
    writer.write_u8(static_cast<uint8_t>(has_pk));
    if (has_pk) {
        writer.write_string(def.primary_key_field_name.value());
    }
    // writer.write_u32_le(0); // Placeholder for secondary_indexes.len() if supported
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalReducerParameterDef& def) {
    writer.write_string(def.name);
    serialize(writer, def.ty);
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalReducerDef& def) {
    writer.write_string(def.name);
    writer.write_u32_le(static_cast<uint32_t>(def.parameters.size()));
    for (const auto& param : def.parameters) {
        serialize(writer, param);
    }
    // serialize(writer, def.return_type); // If return types are supported
}

void SpacetimeDB::Internal::serialize(bsatn::Writer& writer, const InternalModuleDef& def) {
    writer.write_string(def.name);

    writer.write_u32_le(static_cast<uint32_t>(def.types.size()));
    for (const auto& type_def : def.types) {
        serialize(writer, type_def);
    }

    writer.write_u32_le(static_cast<uint32_t>(def.tables.size()));
    for (const auto& table_def : def.tables) {
        serialize(writer, table_def);
    }

    writer.write_u32_le(static_cast<uint32_t>(def.reducers.size()));
    for (const auto& reducer_def : def.reducers) {
        serialize(writer, reducer_def);
    }
    // writer.write_bytes(def.checksum); // If checksum is part of schema
}


// Public accessor function implementation
std::vector<std::byte> SpacetimeDB::Internal::get_serialized_module_definition_bytes() {
    // 1. Get the ModuleSchema instance (populated by macros)
    const SpacetimeDb::ModuleSchema& user_schema = SpacetimeDb::ModuleSchema::instance();

    // 2. Build InternalModuleDef from ModuleSchema
    InternalModuleDef internal_module_def = build_internal_module_def(user_schema);

    // 3. Create a bsatn::Writer
    bsatn::Writer writer; // Assuming default constructor exists and manages a std::vector<std::byte>

    // 4. Serialize InternalModuleDef
    serialize(writer, internal_module_def);

    // 5. Return the bytes
    return writer.getBytes(); // Assuming getBytes() returns std::vector<std::byte> or similar
}
