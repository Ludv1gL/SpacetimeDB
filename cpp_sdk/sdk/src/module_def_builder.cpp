#include "spacetimedb/internal/module_def.h"    // Updated path
#include "spacetimedb/internal/module_schema.h" // Updated path, For SpacetimeDb::ModuleSchema etc.
#include "spacetimedb/bsatn_all.h"           // Explicit include for bsatn::Writer, though module_def.h includes it

#include <stdexcept> // For std::runtime_error
#include <algorithm> // For std::transform (not currently used, but can be useful)
#include <vector>    // For std::vector
#include <string>    // For std::string

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
        default:
            throw std::runtime_error("Unsupported SpacetimeDb::CoreType for primitive mapping: " + std::to_string(static_cast<int>(core_type)));
    }
}

// Helper function to convert SpacetimeDb::TypeIdentifier to SpacetimeDB::Internal::InternalType
SpacetimeDB::Internal::InternalType map_type_identifier_to_internal_type(
    const SpacetimeDb::TypeIdentifier& type_id,
    const SpacetimeDb::ModuleSchema& user_schema
) {
    (void)user_schema; // user_schema not strictly needed in this simplified version
    SpacetimeDB::Internal::InternalType internal_ty;
    if (type_id.core_type == SpacetimeDb::CoreType::UserDefined) {
        internal_ty.kind = SpacetimeDB::Internal::InternalType::Kind::UserDefined;
        internal_ty.user_defined_name = type_id.user_defined_name;
    } else {
        internal_ty.kind = SpacetimeDB::Internal::InternalType::Kind::Primitive;
        internal_ty.primitive_type = map_core_type_to_internal_primitive(type_id.core_type);
    }
    return internal_ty;
}

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
    // TODO: Add mapping for Vector kind if FieldDefinition supports it explicitly
    // For now, vectors are not directly representable by FieldDefinition's TypeIdentifier in a distinct way
    // other than a user-defined type named "vector" which is not standard.
    // The SATS schema would define vector<T> and option<T> explicitly.
    // This mapping should ideally handle those if the macro schema also captures them.
    return element_type;
}


SpacetimeDB::Internal::InternalModuleDef SpacetimeDB::Internal::build_internal_module_def(
    const SpacetimeDb::ModuleSchema& user_schema) {
    InternalModuleDef module_def_internal;
    module_def_internal.name = "module";

    for (const auto& pair : user_schema.types) {
        const SpacetimeDb::TypeDefinition& user_type_def = pair.second;
        InternalTypeDef internal_type_def;
        internal_type_def.name = user_type_def.spacetime_db_name;

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

    for (const auto& pair : user_schema.tables) {
        const SpacetimeDb::TableDefinition& table_def_user = pair.second;
        InternalTableDef table_def_internal;
        table_def_internal.name = table_def_user.spacetime_name;

        auto it_type = user_schema.types.find(table_def_user.cpp_row_type_name);
        if (it_type != user_schema.types.end()) {
            table_def_internal.row_type_name = it_type->second.spacetime_db_name;
        } else {
            throw std::runtime_error("Row type '" + table_def_user.cpp_row_type_name + "' not found for table '" + table_def_user.spacetime_name + "'.");
        }

        if (!table_def_user.primary_key_field_name.empty()) {
            table_def_internal.primary_key_field_name = table_def_user.primary_key_field_name;
        }
        module_def_internal.tables.push_back(table_def_internal);
    }

    for (const auto& pair : user_schema.reducers) {
        const SpacetimeDb::ReducerDefinition& reducer_def_user = pair.second;
        InternalReducerDef reducer_def_internal;
        reducer_def_internal.name = reducer_def_user.spacetime_name;

        for (const auto& param_user : reducer_def_user.parameters) {
            InternalReducerParameterDef param_internal;
            param_internal.name = param_user.name;

            // This mapping needs to be robust for optionals/vectors if reducer params can be such.
            // Currently, map_field_type_to_internal_type expects a FieldDefinition.
            // We simulate one here.
            SpacetimeDb::FieldDefinition temp_field_for_param_mapping;
            temp_field_for_param_mapping.name = param_user.name;
            temp_field_for_param_mapping.type = param_user.type;
            // TODO: How are optional parameters for reducers specified in ModuleSchema?
            // Assuming ReducerParameterDefinition::type already correctly reflects if it's an Option<T>
            // by having type.user_defined_name be "Option" and type.core_type be UserDefined,
            // which map_type_identifier_to_internal_type doesn't currently expand to InternalType::Kind::Option.
            // This is a gap if reducer params can be directly optional via this path.
            // For now, assume parameters are not optional unless their TypeIdentifier is already complex.
            temp_field_for_param_mapping.is_optional = false;
            param_internal.ty = map_field_type_to_internal_type(temp_field_for_param_mapping, user_schema);

            reducer_def_internal.parameters.push_back(param_internal);
        }
        module_def_internal.reducers.push_back(reducer_def_internal);
    }

    return module_def_internal;
}

// BSATN Serialization Implementations
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
            writer.write_string(type.user_defined_name);
            break;
        case InternalType::Kind::Option:
        case InternalType::Kind::Vector:
            if (!type.element_type) throw std::runtime_error("Option/Vector element_type is null during serialization.");
            serialize(writer, *type.element_type);
            break;
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
    writer.write_string(def.name);
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
    writer.write_string(def.row_type_name);

    bool has_pk = def.primary_key_field_name.has_value();
    writer.write_u8(static_cast<uint8_t>(has_pk));
    if (has_pk) {
        writer.write_string(def.primary_key_field_name.value());
    }
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
}

std::vector<std::byte> SpacetimeDB::Internal::get_serialized_module_definition_bytes() {
    const SpacetimeDb::ModuleSchema& user_schema = SpacetimeDb::ModuleSchema::instance();
    InternalModuleDef internal_module_def = build_internal_module_def(user_schema);
    bsatn::Writer writer;
    serialize(writer, internal_module_def);
    return writer.get_buffer(); // Use get_buffer() if take_buffer() is not what we want (e.g. if writer is reused)
                                // Assuming getBytes() was a typo for get_buffer() or take_buffer() from bsatn::Writer
}
