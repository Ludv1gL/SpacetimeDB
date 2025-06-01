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

// Helper macros for token pasting
#define SPACETIMEDB_PASTE_IMPL(a, b) a##b
#define SPACETIMEDB_PASTE(a, b) SPACETIMEDB_PASTE_IMPL(a, b)

#define SPACETIMEDB_PASTE_PREFIX_IMPL(prefix, type) prefix##type
#define SPACETIMEDB_PASTE_PREFIX(prefix, type) SPACETIMEDB_PASTE_PREFIX_IMPL(prefix, type)

// --- Type Definition Macros ---

/** @internal Basic helper to construct a FieldDefinition for schema registration. */
inline ::SpacetimeDB::FieldDefinition SPACETIMEDB_FIELD_INTERNAL(const char* name, ::SpacetimeDB::CoreType core_type, const char* user_defined_name, bool is_optional, bool is_unique_field, bool is_auto_inc_field) {
    ::SpacetimeDB::TypeIdentifier type_id;
    type_id.core_type = core_type;
    if (user_defined_name) {
        type_id.user_defined_name = user_defined_name;
    }
    ::SpacetimeDB::FieldDefinition field_def;
    field_def.name = name;
    field_def.type = type_id;
    field_def.is_optional = is_optional;
    field_def.is_unique = is_unique_field;
    field_def.is_auto_increment = is_auto_inc_field;
    return field_def;
}

#define SPACETIMEDB_FIELD(FieldNameStr, FieldCoreType, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, false, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_OPTIONAL(FieldNameStr, FieldCoreType, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, true, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_CUSTOM(FieldNameStr, UserDefinedTypeNameStr, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, ::SpacetimeDB::CoreType::UserDefined, UserDefinedTypeNameStr, false, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_CUSTOM_OPTIONAL(FieldNameStr, UserDefinedTypeNameStr, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, ::SpacetimeDB::CoreType::UserDefined, UserDefinedTypeNameStr, true, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_XX_SERIALIZE_FIELD(WRITER, VALUE_OBJ, CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        (WRITER).write_optional((VALUE_OBJ).FIELD_NAME); \
    } else if constexpr (IS_VECTOR) { \
        (WRITER).write_vector((VALUE_OBJ).FIELD_NAME); \
    } else { \
        SpacetimeDB::bsatn::serialize((WRITER), (VALUE_OBJ).FIELD_NAME); \
    }

#define SPACETIMEDB_XX_DESERIALIZE_FIELD(READER, VALUE_OBJ, CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        (VALUE_OBJ).FIELD_NAME = (READER).read_optional<CPP_TYPE>(); \
    } else if constexpr (IS_VECTOR) { \
        (VALUE_OBJ).FIELD_NAME = (READER).read_vector<CPP_TYPE>(); \
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
        else if constexpr (std::is_same_v<CPP_TYPE, ::SpacetimeDB::sdk::Identity>) { obj.FIELD_NAME.bsatn_deserialize(reader); } \
        else if constexpr (std::is_same_v<CPP_TYPE, ::SpacetimeDB::sdk::Timestamp>) { obj.FIELD_NAME.bsatn_deserialize(reader); } \
        else if constexpr (std::is_same_v<CPP_TYPE, ::SpacetimeDB::Types::uint128_t_placeholder>) { obj.FIELD_NAME = reader.read_u128_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, ::SpacetimeDB::Types::int128_t_placeholder>) { obj.FIELD_NAME = reader.read_i128_le(); } \
        else if constexpr (std::is_same_v<CPP_TYPE, ::SpacetimeDB::sdk::u256_placeholder>) { obj.FIELD_NAME.bsatn_deserialize(reader); } \
        else if constexpr (std::is_same_v<CPP_TYPE, ::SpacetimeDB::sdk::i256_placeholder>) { obj.FIELD_NAME.bsatn_deserialize(reader); } \
        /* Add other SpacetimeDB::sdk types here if they have direct bsatn_deserialize methods */ \
        /* Fallback for enums or other custom structs that have global deserialize specializations */ \
        else { \
            obj.FIELD_NAME = SpacetimeDB::bsatn::deserialize<CPP_TYPE>(reader); \
        } \
    }

// Schema-only struct registration (no BSATN generation)
#define SPACETIMEDB_TYPE_STRUCT(CppTypeName, SanitizedCppTypeName, SpacetimeDbTypeNameStr, FieldsInitializerList) \
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct Register##SanitizedCppTypeName { \
            Register##SanitizedCppTypeName() { \
                ::SpacetimeDB::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbTypeNameStr, \
                    std::vector< ::SpacetimeDB::FieldDefinition> FieldsInitializerList \
                ); \
            } \
        }; \
        static Register##SanitizedCppTypeName register_##SanitizedCppTypeName##_instance; \
    }}

#define SPACETIMEDB_ENUM_VARIANT(VariantNameStr) \
    ::SpacetimeDB::EnumVariantDefinition{VariantNameStr}


#define SPACETIMEDB_TYPE_ENUM(CppTypeName, SanitizedCppTypeName, SpacetimeDbEnumNameStr, VariantsInitializerList) \
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct SPACETIMEDB_PASTE(Register, SanitizedCppTypeName) { \
            SPACETIMEDB_PASTE(Register, SanitizedCppTypeName)() { \
                ::SpacetimeDB::ModuleSchema::instance().register_enum_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbEnumNameStr, \
                    std::vector< ::SpacetimeDB::EnumVariantDefinition> VariantsInitializerList \
                ); \
            } \
        }; \
        static SPACETIMEDB_PASTE(Register, SanitizedCppTypeName) SPACETIMEDB_PASTE(register_, SPACETIMEDB_PASTE(SanitizedCppTypeName, _instance)); \
    }} /* SpacetimeDB::ModuleRegistration */ \
namespace SpacetimeDB::bsatn { /* Functions in SpacetimeDB::bsatn namespace */ \
    /* Forward declaration of the concrete deserialization function */ \
    /* Ensure CppTypeName is fully qualified or this is within a context where CppTypeName is known */ \
    inline void serialize(::SpacetimeDB::bsatn::Writer& writer, const CppTypeName& value) { \
        writer.write_u8(static_cast<uint8_t>(value)); \
    } \
    template<> \
    inline CppTypeName deserialize<CppTypeName>(::SpacetimeDB::bsatn::Reader& reader) { \
        uint8_t val = reader.read_u8(); \
        return static_cast<CppTypeName>(val); \
    } \
}


#define SPACETIMEDB_TABLE(CppRowTypeName, SpacetimeDbTableNameStr, IsPublicBool, ScheduledReducerNameStr) \
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct RegisterTable_##CppRowTypeName##_##SpacetimedbNameStr { \
            RegisterTable_##CppRowTypeName##_##SpacetimedbNameStr() { \
                ::SpacetimeDB::ModuleSchema::instance().register_table( \
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
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct SetPrimaryKey_Default_##FieldNameStr { \
             SetPrimaryKey_Default_##FieldNameStr() { \
                ::SpacetimeDB::ModuleSchema::instance().set_primary_key( \
                    SpacetimeDbTableNameStr, \
                    FieldNameStr \
                ); \
            } \
        }; \
        static SetPrimaryKey_Default_##FieldNameStr set_pk_##FieldNameStr##_instance; \
    }}

#define SPACETIMEDB_INDEX(SpacetimeDbTableNameStr, IndexNameStr, ColumnFieldNamesInitializerList) \
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct RegisterIndex_##SpacetimeDbTableNameStr##_##IndexNameStr { \
            RegisterIndex_##SpacetimeDbTableNameStr##_##IndexNameStr() { \
                ::SpacetimeDB::IndexDefinition index_def; \
                index_def.index_name = IndexNameStr; \
                std::vector<std::string> cols = ColumnFieldNamesInitializerList; \
                index_def.column_field_names = std::move(cols); \
                ::SpacetimeDB::ModuleSchema::instance().add_index( \
                    SpacetimeDbTableNameStr, \
                    index_def \
                ); \
            } \
        }; \
        static RegisterIndex_##SpacetimeDbTableNameStr##_##IndexNameStr register_index_##SpacetimeDbTableNameStr##_##IndexNameStr##_instance; \
    }}


#define SPACETIMEDB_REDUCER_PARAM(ParamNameStr, ParamCoreType) \
    ::SpacetimeDB::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, ParamCoreType, nullptr)

#define SPACETIMEDB_REDUCER_PARAM_CUSTOM(ParamNameStr, UserDefinedTypeNameStr) \
    ::SpacetimeDB::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, ::SpacetimeDB::CoreType::UserDefined, UserDefinedTypeNameStr)

inline ::SpacetimeDB::ReducerParameterDefinition SPACETIMEDB_REDUCER_PARAM_INTERNAL(const char* name, ::SpacetimeDB::CoreType core_type, const char* user_defined_name) {
    ::SpacetimeDB::TypeIdentifier type_id;
    type_id.core_type = core_type;
    if (user_defined_name) {
        type_id.user_defined_name = user_defined_name;
    }
    ::SpacetimeDB::ReducerParameterDefinition param_def;
    param_def.name = name;
    param_def.type = type_id;
    return param_def;
}

#define SPACETIMEDB_XX_DESERIALIZE_REDUCER_ARG_AND_PASS(ParamCppType, ParamName, ReaderName, ArgsVectorName) \
    ArgsVectorName.push_back(std::make_any<ParamCppType>(::bsatn::deserialize<ParamCppType>(ReaderName)));

#define SPACETIMEDB_REDUCER_ARG_DECLARE_HELPER(ParamCppType, ParamName, reader_instance) \
    ParamCppType ParamName = ::bsatn::deserialize<ParamCppType>(reader_instance);

#define SPACETIMEDB_REGISTER_REDUCER_SCHEMA(SpacetimedbNameStr, CppFunctionName, Kind, RegParamsInitializerList, ...) \
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct RegisterReducer_##CppFunctionName { \
            RegisterReducer_##CppFunctionName() { \
                auto invoker_lambda = [](::SpacetimeDB::bsatn::Reader& reader_param) { \
                    std::tuple<__VA_ARGS__> args_tuple = \
                        std::make_tuple(SpacetimeDB::bsatn::deserialize<__VA_ARGS__>(reader_param)...); \
                    std::apply(CppFunctionName, args_tuple); \
                }; \
                ::SpacetimeDB::ModuleSchema::instance().register_reducer( \
                    SpacetimedbNameStr, \
                    SPACETIMEDB_STRINGIFY(CppFunctionName), \
                    std::vector< ::SpacetimeDB::ReducerParameterDefinition> RegParamsInitializerList, \
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
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA("init", CppFunctionName, ::SpacetimeDB::ReducerKind::Init, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER("init", CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_CLIENT_CONNECTED(CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA("client_connected", CppFunctionName, ::SpacetimeDB::ReducerKind::ClientConnected, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER("client_connected", CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_CLIENT_DISCONNECTED(CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA("client_disconnected", CppFunctionName, ::SpacetimeDB::ReducerKind::ClientDisconnected, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER("client_disconnected", CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_SCHEDULED(SpacetimedbNameStr, CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA(SpacetimedbNameStr, CppFunctionName, ::SpacetimeDB::ReducerKind::Scheduled, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER(SpacetimedbNameStr, CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_REDUCER_NAMED(SpacetimedbNameStr, CppFunctionName, ParamsSchemaList, ...) \
    SPACETIMEDB_REGISTER_REDUCER_SCHEMA(SpacetimedbNameStr, CppFunctionName, ::SpacetimeDB::ReducerKind::None, ParamsSchemaList, ##__VA_ARGS__); \
    SPACETIMEDB_EXPORT_REDUCER(SpacetimedbNameStr, CppFunctionName, ##__VA_ARGS__)

#define SPACETIMEDB_CLIENT_VISIBILITY_FILTER(FilterNameConstStr, SqlString) \
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct RegisterFilter_##FilterNameConstStr { \
            RegisterFilter_##FilterNameConstStr() { \
                ::SpacetimeDB::ModuleSchema::instance().register_filter( \
                    #FilterNameConstStr, \
                    SqlString \
                ); \
            } \
        }; \
        static RegisterFilter_##FilterNameConstStr register_filter_##FilterNameConstStr##_instance; \
    }}

#define SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(CppTypeName, SanitizedCppTypeName, SpacetimeDbNameStr, FIELDS_MACRO, RegFieldsInitializerList) \
    namespace SpacetimeDB { namespace ModuleRegistration { \
        struct SPACETIMEDB_PASTE(Register, SanitizedCppTypeName) { \
            SPACETIMEDB_PASTE(Register, SanitizedCppTypeName)() { \
                ::SpacetimeDB::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbNameStr, \
                    std::vector< ::SpacetimeDB::FieldDefinition> RegFieldsInitializerList \
                ); \
            } \
        }; \
        static SPACETIMEDB_PASTE(Register, SanitizedCppTypeName) SPACETIMEDB_PASTE(register_, SPACETIMEDB_PASTE(SanitizedCppTypeName, _instance)); \
    }} /* SpacetimeDB::ModuleRegistration */ \
namespace SpacetimeDB::bsatn { /* Functions in SpacetimeDB::bsatn namespace */ \
    /* Forward declaration for the struct's deserialize function */ \
    /* CppTypeName needs to be fully qualified if this is not in its namespace, or use ADL */ \
    inline void serialize(::SpacetimeDB::bsatn::Writer& writer, const CppTypeName& value) { \
        FIELDS_MACRO(SPACETIMEDB_XX_SERIALIZE_FIELD, writer, value); \
    } \
    template<> \
    inline CppTypeName deserialize<CppTypeName>(::SpacetimeDB::bsatn::Reader& reader) { \
        CppTypeName obj{}; \
        FIELDS_MACRO(SPACETIMEDB_XX_DESERIALIZE_FIELD, reader, obj); \
        return obj; \
    } \
}

#endif // SPACETIME_MACROS_H

