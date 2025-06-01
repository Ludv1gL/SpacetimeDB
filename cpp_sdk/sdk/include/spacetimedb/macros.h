#ifndef SPACETIMEDB_MACROS_H
#define SPACETIMEDB_MACROS_H

#include "spacetimedb/internal/module_schema.h"
#include "spacetimedb/bsatn/reader.h"
#include "spacetimedb/bsatn/writer.h"

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits> // For std::is_same_v

// Helper macro to stringify its argument
#define SPACETIMEDB_STRINGIFY_IMPL(x) #x
#define SPACETIMEDB_STRINGIFY(x) SPACETIMEDB_STRINGIFY_IMPL(x)

// --- Type Definition Macros ---

/** @internal Basic helper to construct a FieldDefinition for schema registration. */
inline ::SpacetimeDb::FieldDefinition SPACETIMEDB_FIELD_INTERNAL(const char* name, ::SpacetimeDb::CoreType core_type, const char* user_defined_name, bool is_optional, bool is_unique_field, bool is_auto_inc_field) {
    ::SpacetimeDb::TypeIdentifier type_id;
    type_id.core_type = core_type;
    if (user_defined_name) {
        type_id.user_defined_name = user_defined_name;
    }
    ::SpacetimeDb::FieldDefinition field_def;
    field_def.name = name;
    field_def.type = type_id;
    field_def.is_optional = is_optional;
    field_def.is_unique = is_unique_field;
    field_def.is_auto_increment = is_auto_inc_field;
    return field_def;
}

#define SPACETIMEDB_FIELD(FieldNameStr, FieldCoreType, IsUniqueBool, IsAutoIncBool) \
    ::SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, false, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_OPTIONAL(FieldNameStr, FieldCoreType, IsUniqueBool, IsAutoIncBool) \
    ::SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, true, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_CUSTOM(FieldNameStr, UserDefinedTypeNameStr, IsUniqueBool, IsAutoIncBool) \
    ::SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, ::SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr, false, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_CUSTOM_OPTIONAL(FieldNameStr, UserDefinedTypeNameStr, IsUniqueBool, IsAutoIncBool) \
    ::SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, ::SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr, true, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_TYPE_STRUCT(CppTypeName, CppCleanName, SpacetimeDbNameStr, FieldsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##CppCleanName { \
            Register##CppCleanName() { \
                ::SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbNameStr, \
                    std::vector< ::SpacetimeDb::FieldDefinition> FieldsInitializerList \
                ); \
            } \
        }; \
        static Register##CppCleanName register_##CppCleanName##_instance; \
    }}

#define SPACETIMEDB_ENUM_VARIANT(VariantNameStr) \
    ::SpacetimeDb::EnumVariantDefinition{VariantNameStr}

#define SPACETIMEDB_TYPE_ENUM(CppEnumName, CppCleanName, SpacetimeDbNameStr, VariantsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##CppCleanName { \
            Register##CppCleanName() { \
                ::SpacetimeDb::ModuleSchema::instance().register_enum_type( \
                    SPACETIMEDB_STRINGIFY(CppEnumName), \
                    SpacetimeDbNameStr, \
                    std::vector< ::SpacetimeDb::EnumVariantDefinition> VariantsInitializerList \
                ); \
            } \
        }; \
        static Register##CppCleanName register_##CppCleanName##_instance; \
    }} \
    namespace SpacetimeDb { namespace bsatn { \
        inline void serialize(::bsatn::Writer& writer, const CppEnumName& value) { \
            writer.write_u8(static_cast<uint8_t>(value)); \
        } \
        inline CppEnumName deserialize_##CppCleanName(::bsatn::Reader& reader) { \
            uint8_t val = reader.read_u8(); \
            return static_cast<CppEnumName>(val); \
        } \
    }}

#define SPACETIMEDB_TABLE(CppRowTypeName, SpacetimeDbTableNameStr, IsPublicBool, ScheduledReducerNameStr) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct RegisterTable_##CppRowTypeName##_##SpacetimedbNameStr { \
            RegisterTable_##CppRowTypeName##_##SpacetimedbNameStr() { \
                ::SpacetimeDb::ModuleSchema::instance().register_table( \
                    SPACETIMEDB_STRINGIFY(CppRowTypeName), \
                    SpacetimeDbTableNameStr, \
                    IsPublicBool, \
                    ScheduledReducerNameStr \
                ); \
            } \
        }; \
        static RegisterTable_##CppRowTypeName##_##SpacetimedbNameStr register_table_##CppRowTypeName##_##SpacetimedbNameStr##_instance; \
    }}

#define SPACETIMEDB_PRIMARY_KEY(SpacetimeDbTableNameStr, FieldNameStr) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct SetPrimaryKey_Default_##FieldNameStr { \
             SetPrimaryKey_Default_##FieldNameStr() { \
                ::SpacetimeDb::ModuleSchema::instance().set_primary_key( \
                    SpacetimeDbTableNameStr, \
                    FieldNameStr \
                ); \
            } \
        }; \
        static SetPrimaryKey_Default_##FieldNameStr set_pk_##FieldNameStr##_instance; \
    }}

#define SPACETIMEDB_INDEX(SpacetimeDbTableNameStr, IndexNameStr, ColumnFieldNamesInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct RegisterIndex_##SpacetimeDbTableNameStr##_##IndexNameStr { \
            RegisterIndex_##SpacetimeDbTableNameStr##_##IndexNameStr() { \
                ::SpacetimeDb::IndexDefinition index_def; \
                index_def.index_name = IndexNameStr; \
                std::vector<std::string> cols = ColumnFieldNamesInitializerList; \
                index_def.column_field_names = std::move(cols); \
                ::SpacetimeDb::ModuleSchema::instance().add_index( \
                    SpacetimeDbTableNameStr, \
                    index_def \
                ); \
            } \
        }; \
        static RegisterIndex_##SpacetimeDbTableNameStr##_##IndexNameStr register_index_##SpacetimeDbTableNameStr##_##IndexNameStr##_instance; \
    }}

inline ::SpacetimeDb::ReducerParameterDefinition SPACETIMEDB_REDUCER_PARAM_INTERNAL(const char* name, ::SpacetimeDb::CoreType core_type, const char* user_defined_name) {
    ::SpacetimeDb::TypeIdentifier type_id;
    type_id.core_type = core_type;
    if (user_defined_name) {
        type_id.user_defined_name = user_defined_name;
    }
    ::SpacetimeDb::ReducerParameterDefinition param_def;
    param_def.name = name;
    param_def.type = type_id;
    return param_def;
}

#define SPACETIMEDB_REDUCER_PARAM(ParamNameStr, ParamCoreType) \
    ::SpacetimeDb::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, ParamCoreType, nullptr)

#define SPACETIMEDB_REDUCER_PARAM_CUSTOM(ParamNameStr, UserDefinedTypeNameStr) \
    ::SpacetimeDb::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, ::SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr)

#define SPACETIMEDB_REGISTER_REDUCER_SCHEMA(SpacetimedbNameStr, CppFunctionName, Kind, RegParamsInitializerList, ...) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct RegisterReducer_##CppFunctionName { \
            RegisterReducer_##CppFunctionName() { \
                auto invoker_lambda = [](::bsatn::Reader& reader_param) { \
                    std::tuple<__VA_ARGS__> args_tuple = \
                        std::make_tuple(::bsatn::deserialize<__VA_ARGS__>(reader_param)...); \
                    std::apply(CppFunctionName, args_tuple); \
                }; \
                ::SpacetimeDb::ModuleSchema::instance().register_reducer( \
                    SpacetimedbNameStr, \
                    SPACETIMEDB_STRINGIFY(CppFunctionName), \
                    std::vector< ::SpacetimeDb::ReducerParameterDefinition> RegParamsInitializerList, \
                    invoker_lambda, \
                    Kind \
                ); \
            } \
        }; \
        static RegisterReducer_##CppFunctionName register_reducer_##CppFunctionName##_instance; \
    }}

#ifndef SPACETIMEDB_EXPORT_REDUCER
#define SPACETIMEDB_EXPORT_REDUCER(SpacetimedbNameStr, CppFunctionName, ...) \
    static_assert(true, "SPACETIMEDB_EXPORT_REDUCER placeholder");
#endif

#define SPACETIMEDB_REDUCER_INIT(CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA("init", CppFunctionName, ::SpacetimeDb::ReducerKind::Init, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER("init", CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_CLIENT_CONNECTED(CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA("client_connected", CppFunctionName, ::SpacetimeDb::ReducerKind::ClientConnected, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER("client_connected", CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_CLIENT_DISCONNECTED(CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA("client_disconnected", CppFunctionName, ::SpacetimeDb::ReducerKind::ClientDisconnected, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER("client_disconnected", CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_SCHEDULED(SpacetimedbNameStr, CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA(SpacetimedbNameStr, CppFunctionName, ::SpacetimeDb::ReducerKind::Scheduled, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER(SpacetimedbNameStr, CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_NAMED(SpacetimedbNameStr, CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA(SpacetimedbNameStr, CppFunctionName, ::SpacetimeDb::ReducerKind::None, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER(SpacetimedbNameStr, CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_CLIENT_VISIBILITY_FILTER(FilterNameConstStr, SqlString) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct RegisterFilter_##FilterNameConstStr { \
            RegisterFilter_##FilterNameConstStr() { \
                ::SpacetimeDb::ModuleSchema::instance().register_filter( \
                    #FilterNameConstStr, \
                    SqlString \
                ); \
            } \
        }; \
        static RegisterFilter_##FilterNameConstStr register_filter_##FilterNameConstStr##_instance; \
    }}

// Updated X-Macros to use writer/value/reader/obj from the surrounding scope
// and to start basic type dispatch.
#define SPACETIMEDB_XX_SERIALIZE_FIELD(CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        writer.write_optional(value.FIELD_NAME); \
    } else if constexpr (IS_VECTOR) { \
        writer.write_vector(value.FIELD_NAME); \
    } else { \
        if constexpr (std::is_same_v<CPP_TYPE, uint8_t>) { writer.write_u8(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, uint16_t>) { writer.write_u16_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, uint32_t>) { writer.write_u32_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, uint64_t>) { writer.write_u64_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int8_t>) { writer.write_i8(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int16_t>) { writer.write_i16_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int32_t>) { writer.write_i32_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int64_t>) { writer.write_i64_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, float>) { writer.write_f32_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, double>) { writer.write_f64_le(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, bool>) { writer.write_bool(value.FIELD_NAME); } \
        else if constexpr (std::is_same_v<CPP_TYPE, std::string>) { writer.write_string(value.FIELD_NAME); } \
        /* For SpacetimeDB specific SDK types that have bsatn_serialize methods */ \
        else if constexpr (std::is_same_v<CPP_TYPE, ::spacetimedb::sdk::Identity>) { value.FIELD_NAME.bsatn_serialize(writer); } \
        else if constexpr (std::is_same_v<CPP_TYPE, ::spacetimedb::sdk::Timestamp>) { value.FIELD_NAME.bsatn_serialize(writer); } \
        /* Add other spacetimedb::sdk types here if they have direct bsatn_serialize methods */ \
        /* Fallback for enums (which have global serialize) or other custom structs */ \
        else { \
            ::SpacetimeDb::bsatn::serialize(writer, value.FIELD_NAME); \
        } \
    }

#define SPACETIMEDB_XX_DESERIALIZE_FIELD(CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        obj.FIELD_NAME = reader.read_optional<CPP_TYPE>(); \
    } else if constexpr (IS_VECTOR) { \
        obj.FIELD_NAME = reader.read_vector<CPP_TYPE>(); \
    } else { \
        if constexpr (std::is_same_v<CPP_TYPE, uint8_t>) { obj.FIELD_NAME = reader.read_u8(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, uint16_t>) { obj.FIELD_NAME = reader.read_u16_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, uint32_t>) { obj.FIELD_NAME = reader.read_u32_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, uint64_t>) { obj.FIELD_NAME = reader.read_u64_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int8_t>) { obj.FIELD_NAME = reader.read_i8(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int16_t>) { obj.FIELD_NAME = reader.read_i16_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int32_t>) { obj.FIELD_NAME = reader.read_i32_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, int64_t>) { obj.FIELD_NAME = reader.read_i64_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, float>) { obj.FIELD_NAME = reader.read_f32_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, double>) { obj.FIELD_NAME = reader.read_f64_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, bool>) { obj.FIELD_NAME = reader.read_bool(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, std::string>) { obj.FIELD_NAME = reader.read_string(); } \
        /* For SpacetimeDB specific SDK types that have bsatn_deserialize methods */ \
        else if constexpr (std::is_same_v<CPP_TYPE, ::spacetimedb::sdk::Identity>) { obj.FIELD_NAME.bsatn_deserialize(reader); } \
        else if constexpr (std::is_same_v<CPP_TYPE, ::spacetimedb::sdk::Timestamp>) { obj.FIELD_NAME.bsatn_deserialize(reader); } \
        /* Add other spacetimedb::sdk types here if they have direct bsatn_deserialize methods */ \
        /* Fallback for enums or other custom structs that have global deserialize specializations */ \
        else { \
            obj.FIELD_NAME = ::bsatn::deserialize<CPP_TYPE>(reader); \
        } \
    }

#define SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(CppTypeName, CppCleanName, SpacetimeDbNameStr, FIELDS_MACRO, RegFieldsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##CppCleanName { \
            Register##CppCleanName() { \
                ::SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbNameStr, \
                    std::vector< ::SpacetimeDb::FieldDefinition> RegFieldsInitializerList \
                ); \
            } \
        }; \
        static Register##CppCleanName register_##CppCleanName##_instance; \
    }} \
    namespace SpacetimeDb { namespace bsatn { \
        /* Forward declare deserialize for potential recursive types */ \
        CppTypeName deserialize_##CppCleanName(::bsatn::Reader& reader); \
        \
        inline void serialize(::bsatn::Writer& writer, const CppTypeName& value) { \
            FIELDS_MACRO(SPACETIMEDB_XX_SERIALIZE_FIELD) \
        } \
        \
        inline CppTypeName deserialize_##CppCleanName(::bsatn::Reader& reader) { \
            CppTypeName obj; \
            FIELDS_MACRO(SPACETIMEDB_XX_DESERIALIZE_FIELD) \
            return obj; \
        } \
        \
    }} \
    /* Specialization for bsatn::deserialize<CppTypeName> must be in bsatn namespace */ \
    namespace bsatn { \
        template<> \
        inline CppTypeName deserialize<CppTypeName>(::bsatn::Reader& reader) { \
            return ::SpacetimeDb::bsatn::deserialize_##CppCleanName(reader); \
        } \
    }

#endif // SPACETIMEDB_MACROS_H
