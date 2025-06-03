#ifndef SPACETIMEDB_MACROS_H
#define SPACETIMEDB_MACROS_H

#include "spacetimedb/internal/Module.h"
#include "spacetimedb/bsatn_all.h"
#include "spacetimedb/sdk/reducer_context.h"

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

// --- New Table and Reducer Definition Macros ---

// Table definition macro using the new Internal API
#define SPACETIMEDB_TABLE(RowType, table_name, is_public) \
    class table_name##_TableView : public SpacetimeDb::Internal::ITableView<table_name##_TableView, RowType> { \
    public: \
        static SpacetimeDb::Internal::RawTableDefV9 MakeTableDesc(SpacetimeDb::Internal::ITypeRegistrar& registrar) { \
            SpacetimeDb::Internal::RawTableDefV9 table; \
            table.name = SPACETIMEDB_STRINGIFY(table_name); \
            table.table_access = is_public ? SpacetimeDb::Internal::TableAccess::Public : SpacetimeDb::Internal::TableAccess::Private; \
            table.table_type = SpacetimeDb::Internal::TableType::User; \
            /* TODO: Generate product type ref from RowType */ \
            table.product_type_ref = 0; \
            return table; \
        } \
        static uint32_t GetTableId() { \
            static uint32_t id = 0xFFFFFFFF; \
            if (id == 0xFFFFFFFF) { \
                std::string name = SPACETIMEDB_STRINGIFY(table_name); \
                SpacetimeDb::Internal::FFI::table_id_from_name( \
                    reinterpret_cast<const uint8_t*>(name.data()), \
                    name.size(), \
                    &id \
                ); \
            } \
            return id; \
        } \
        static RowType ReadGenFields(bsatn::Reader& reader, const RowType& row) { \
            /* TODO: Read generated fields */ \
            return row; \
        } \
    }; \
    namespace { \
        struct Register_##table_name##_Table { \
            Register_##table_name##_Table() { \
                SpacetimeDb::Internal::Module::RegisterTable<RowType, table_name##_TableView>(); \
            } \
        }; \
        static Register_##table_name##_Table register_##table_name##_table_instance; \
    }

// Reducer definition macro using the new Internal API
#define SPACETIMEDB_REDUCER(name, ctx_param, ...) \
    void name##_impl(SpacetimeDb::sdk::ReducerContext ctx_param, ##__VA_ARGS__); \
    class name##_Reducer : public SpacetimeDb::Internal::IReducer { \
    public: \
        SpacetimeDb::Internal::RawReducerDefV9 MakeReducerDef(SpacetimeDb::Internal::ITypeRegistrar& registrar) override { \
            SpacetimeDb::Internal::RawReducerDefV9 reducer; \
            reducer.name = SPACETIMEDB_STRINGIFY(name); \
            /* TODO: Generate function type ref */ \
            reducer.func_type_ref = 0; \
            return reducer; \
        } \
        void Invoke(bsatn::Reader& reader, SpacetimeDb::Internal::IReducerContext& ctx) override { \
            /* TODO: Deserialize arguments and call implementation */ \
            SpacetimeDb::sdk::ReducerContext sdk_ctx{/* TODO: Initialize from IReducerContext */}; \
            name##_impl(sdk_ctx, ##__VA_ARGS__); \
        } \
    }; \
    namespace { \
        struct Register_##name##_Reducer { \
            Register_##name##_Reducer() { \
                SpacetimeDb::Internal::Module::RegisterReducer<name##_Reducer>(); \
            } \
        }; \
        static Register_##name##_Reducer register_##name##_reducer_instance; \
    } \
    void name##_impl(SpacetimeDb::sdk::ReducerContext ctx_param, ##__VA_ARGS__)

// Legacy compatibility - these will be removed in future
#define SPACETIMEDB_FIELD_INTERNAL(name, core_type, user_defined_name, is_optional, is_unique_field, is_auto_inc_field) \
    /* Deprecated - use SPACETIMEDB_TABLE macro instead */
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
    ArgsVectorName.push_back(std::make_any<ParamCppType>(::SpacetimeDb::bsatn::deserialize<ParamCppType>(ReaderName)));

#define SPACETIMEDB_REDUCER_ARG_DECLARE_HELPER(ParamCppType, ParamName, reader_instance) \
    ParamCppType ParamName = ::SpacetimeDb::bsatn::deserialize<ParamCppType>(reader_instance);

#define SPACETIMEDB_REGISTER_REDUCER_SCHEMA(SpacetimedbNameStr, CppFunctionName, Kind, RegParamsInitializerList, ...) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct RegisterReducer_##CppFunctionName { \
            RegisterReducer_##CppFunctionName() { \
                auto invoker_lambda = [](::SpacetimeDb::bsatn::Reader& reader_param) { \
                    std::tuple<__VA_ARGS__> args_tuple = \
                        std::make_tuple(SpacetimeDb::bsatn::deserialize<__VA_ARGS__>(reader_param)...); \
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

// Re-typed SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS
#define SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(_actual_cpp_type_name_, SanitizedCppTypeName, SpacetimeDbNameStr, FIELDS_MACRO, RegFieldsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct SPACETIMEDB_PASTE(Register, SanitizedCppTypeName) { \
            SPACETIMEDB_PASTE(Register, SanitizedCppTypeName)() { \
                ::SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(_actual_cpp_type_name_), \
                    SpacetimeDbNameStr, \
                    std::vector< ::SpacetimeDb::FieldDefinition> RegFieldsInitializerList \
                ); \
            } \
        }; \
        static SPACETIMEDB_PASTE(Register, SanitizedCppTypeName) SPACETIMEDB_PASTE(register_, SPACETIMEDB_PASTE(SanitizedCppTypeName, _instance)); \
    }} /* SpacetimeDb::ModuleRegistration */ \
    namespace SpacetimeDb::bsatn { /* Functions in SpacetimeDb::bsatn namespace */ \
            inline void serialize(::SpacetimeDb::bsatn::Writer& writer, const _actual_cpp_type_name_& value) { \
                FIELDS_MACRO(SPACETIMEDB_XX_SERIALIZE_FIELD, writer, value); \
        } \
    } \
    /* Custom deserialize function that will be found by ADL */ \
    inline _actual_cpp_type_name_ spacetimedb_deserialize(::SpacetimeDb::bsatn::Reader& reader, _actual_cpp_type_name_*) { \
        _actual_cpp_type_name_ obj{}; \
        using namespace SpacetimeDb::bsatn; \
        FIELDS_MACRO(SPACETIMEDB_XX_DESERIALIZE_FIELD, reader, obj); \
        return obj; \
    }

#endif // SPACETIMEDB_MACROS_H
