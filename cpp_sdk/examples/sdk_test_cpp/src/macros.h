#ifndef SPACETIMEDB_MACROS_H
#define SPACETIMEDB_MACROS_H

#include "module_schema.h"
#include "reader.h"
#include "writer.h"

#include <string>
#include <vector>
#include <utility> // For std::make_pair for potential future use, not directly now
#include <tuple>   // For std::tuple, std::apply (used in SPACETIMEDB_REDUCER)
#include <functional> // For std::function (used in ReducerDefinition)


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

// Schema-only struct registration (no BSATN generation)
#define SPACETIMEDB_TYPE_STRUCT(CppTypeName, SanitizedCppTypeName, SpacetimeDbTypeNameStr, FieldsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##SanitizedCppTypeName { \
            Register##SanitizedCppTypeName() { \
                ::SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbTypeNameStr, \
                    std::vector< ::SpacetimeDb::FieldDefinition> FieldsInitializerList \
                ); \
            } \
        }; \
        static Register##SanitizedCppTypeName register_##SanitizedCppTypeName##_instance; \
    }}

#define SPACETIMEDB_ENUM_VARIANT(VariantNameStr) \
    ::SpacetimeDb::EnumVariantDefinition{VariantNameStr}

#define SPACETIMEDB_TYPE_ENUM(CppTypeName, SanitizedCppTypeName, SpacetimeDbEnumNameStr, VariantsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##SanitizedCppTypeName { \
            Register##SanitizedCppTypeName() { \
                ::SpacetimeDb::ModuleSchema::instance().register_enum_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbEnumNameStr, \
                    VariantsInitializerList \
                ); \
            } \
        }; \
        static Register##SanitizedCppTypeName register_##SanitizedCppTypeName##_instance; \
    }} \
    namespace bsatn { /* Functions in global bsatn namespace */ \
        /* Forward declaration of the concrete deserialization function */ \
        CppTypeName deserialize_##SanitizedCppTypeName(::bsatn::Reader& reader); \
        \
        inline void serialize(::bsatn::Writer& writer, const CppTypeName& value) { \
            writer.write_u8(static_cast<uint8_t>(value)); \
        } \
        /* Specialization calling the forward-declared function */ \
        template<> \
        inline CppTypeName deserialize<CppTypeName>(::bsatn::Reader& reader) { \
            return deserialize_##SanitizedCppTypeName(reader); \
        } \
        /* Definition of the concrete deserialization function */ \
        inline CppTypeName deserialize_##SanitizedCppTypeName(::bsatn::Reader& reader) { \
            uint8_t val = reader.read_u8(); \
            return static_cast<CppTypeName>(val); \
        } \
    }


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


#define SPACETIMEDB_REDUCER_PARAM(ParamNameStr, ParamCoreType) \
    ::SpacetimeDb::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, ParamCoreType, nullptr)

#define SPACETIMEDB_REDUCER_PARAM_CUSTOM(ParamNameStr, UserDefinedTypeNameStr) \
    ::SpacetimeDb::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, ::SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr)

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

#define SPACETIMEDB_XX_DESERIALIZE_REDUCER_ARG_AND_PASS(ParamCppType, ParamName, ReaderName, ArgsVectorName) \
    ArgsVectorName.push_back(std::make_any<ParamCppType>(::bsatn::deserialize<ParamCppType>(ReaderName)));

#define SPACETIMEDB_REDUCER_ARG_DECLARE_HELPER(ParamCppType, ParamName, reader_instance) \
    ParamCppType ParamName = ::bsatn::deserialize<ParamCppType>(reader_instance);

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


#define SPACETIMEDB_XX_SERIALIZE_FIELD(WRITER, VALUE_OBJ, CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        (WRITER).write_optional((VALUE_OBJ).FIELD_NAME); \
    } else if constexpr (IS_VECTOR) { \
        (WRITER).write_vector((VALUE_OBJ).FIELD_NAME); \
    } else { \
        ::bsatn::serialize((WRITER), (VALUE_OBJ).FIELD_NAME); \
    }

#define SPACETIMEDB_XX_DESERIALIZE_FIELD(READER, VALUE_OBJ, CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        (VALUE_OBJ).FIELD_NAME = (READER).read_optional<CPP_TYPE>(); \
    } else if constexpr (IS_VECTOR) { \
        (VALUE_OBJ).FIELD_NAME = (READER).read_vector<CPP_TYPE>(); \
    } else { \
        (VALUE_OBJ).FIELD_NAME = ::bsatn::deserialize<CPP_TYPE>((READER)); \
    }

#define SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(CppTypeName, SanitizedCppTypeName, SpacetimeDbNameStr, FIELDS_MACRO, RegFieldsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##SanitizedCppTypeName { \
            Register##SanitizedCppTypeName() { \
                ::SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbNameStr, \
                    std::vector< ::SpacetimeDb::FieldDefinition> RegFieldsInitializerList \
                ); \
            } \
        }; \
        static Register##SanitizedCppTypeName register_##SanitizedCppTypeName##_instance; \
    }} \
    namespace bsatn { /* Functions in global bsatn namespace */ \
        /* Forward declaration of the concrete deserialization function */ \
        CppTypeName deserialize_##SanitizedCppTypeName(::bsatn::Reader& reader); \
        \
        inline void serialize(::bsatn::Writer& writer, const CppTypeName& value) { \
            FIELDS_MACRO(SPACETIMEDB_XX_SERIALIZE_FIELD, writer, value); \
        } \
        \
        /* Specialization calling the forward-declared function */ \
        template<> \
        inline CppTypeName deserialize<CppTypeName>(::bsatn::Reader& reader) { \
            return deserialize_##SanitizedCppTypeName(reader); \
        } \
        /* Definition of the concrete deserialization function */ \
        inline CppTypeName deserialize_##SanitizedCppTypeName(::bsatn::Reader& reader) { \
            CppTypeName obj; \
            FIELDS_MACRO(SPACETIMEDB_XX_DESERIALIZE_FIELD, reader, obj); \
            return obj; \
        } \
    }

#endif // SPACETIME_MACROS_H
