#ifndef SPACETIMEDB_MACROS_H
#define SPACETIMEDB_MACROS_H

#include "spacetimedb/internal/Module.h"
#include "spacetimedb/bsatn_all.h"
#include "spacetimedb/sdk/reducer_context.h"
#include "spacetimedb/table_ops.h"

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits> // For std::is_same_v
#include <unordered_map>

// Helper macro to stringify its argument
#define SPACETIMEDB_STRINGIFY_IMPL(x) #x
#define SPACETIMEDB_STRINGIFY(x) SPACETIMEDB_STRINGIFY_IMPL(x)

// Helper macros for token pasting
#define SPACETIMEDB_PASTE_IMPL(a, b) a##b
#define SPACETIMEDB_PASTE(a, b) SPACETIMEDB_PASTE_IMPL(a, b)

// --- New Table and Reducer Definition Macros ---

// Helper to get table ID from name - cached like Rust
static uint32_t get_table_id(const std::string& name) {
    static std::unordered_map<std::string, uint32_t> cache;
    auto it = cache.find(name);
    if (it != cache.end()) {
        return it->second;
    }
    
    uint32_t id;
    auto err = SpacetimeDb::Internal::FFI::table_id_from_name(
        reinterpret_cast<const uint8_t*>(name.data()),
        static_cast<uint32_t>(name.size()),
        &id
    );
    
    if (err != SpacetimeDb::Internal::FFI::Errno::OK) {
        throw std::runtime_error("Table not found: " + name);
    }
    
    cache[name] = id;
    return id;
}

// Simplified table macro - combines Rust's directness with C++ type safety
#define SPACETIMEDB_TABLE(RowType, table_name, is_public) \
    /* Table handle type */ \
    struct table_name##__TableHandle { \
        SpacetimeDb::TableOps<RowType> ops; \
        \
        table_name##__TableHandle() : ops(get_table_id(SPACETIMEDB_STRINGIFY(table_name)), SPACETIMEDB_STRINGIFY(table_name)) {} \
        \
        /* Delegate to ops */ \
        uint64_t count() const { return ops.count(); } \
        SpacetimeDb::TableIterator<RowType> iter() const { return ops.iter(); } \
        RowType insert(const RowType& row) { return ops.insert(row); } \
        bool delete_row(const RowType& row) { return ops.delete_row(row); } \
    }; \
    \
    /* Trait for ReducerContext.db */ \
    struct table_name { \
        table_name##__TableHandle table_name() { return {}; } \
    }; \
    \
    /* Registration */ \
    namespace { \
        struct Register_##table_name##_Table { \
            Register_##table_name##_Table() { \
                SpacetimeDb::Internal::Module::RegisterTableDirect( \
                    SPACETIMEDB_STRINGIFY(table_name), \
                    is_public ? SpacetimeDb::Internal::TableAccess::Public : SpacetimeDb::Internal::TableAccess::Private, \
                    []() -> std::vector<uint8_t> { \
                        /* Generate AlgebraicType for RowType */ \
                        return spacetimedb_generate_type<RowType>(); \
                    } \
                ); \
            } \
        }; \
        static Register_##table_name##_Table register_##table_name##_table_instance; \
    }

// Helper to count macro arguments
#define SPACETIMEDB_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define SPACETIMEDB_NARGS(...) SPACETIMEDB_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Simplified reducer macro - Rust-like with C++ conveniences
#define SPACETIMEDB_REDUCER(name, ctx_param, ...) \
    void name(SpacetimeDb::ReducerContext& ctx_param, ##__VA_ARGS__); \
    namespace { \
        SpacetimeDb::Internal::FFI::Errno name##_wrapper( \
            SpacetimeDb::sdk::ReducerContext ctx, \
            const uint8_t* args, size_t args_len \
        ) { \
            try { \
                /* TODO: Deserialize arguments */ \
                name(ctx); \
                return SpacetimeDb::Internal::FFI::Errno::OK; \
            } catch (const std::exception& e) { \
                /* Log error */ \
                return SpacetimeDb::Internal::FFI::Errno::HOST_CALL_FAILURE; \
            } \
        } \
        \
        struct Register_##name##_Reducer { \
            Register_##name##_Reducer() { \
                SpacetimeDb::Internal::Module::RegisterReducerDirect( \
                    SPACETIMEDB_STRINGIFY(name), \
                    name##_wrapper \
                ); \
            } \
        }; \
        static Register_##name##_Reducer register_##name##_reducer_instance; \
    } \
    void name(SpacetimeDb::ReducerContext& ctx_param, ##__VA_ARGS__)

// Helper function to generate AlgebraicType for a type
// This should be specialized for user types
template<typename T>
std::vector<uint8_t> spacetimedb_generate_type() {
    // Default implementation for primitive types
    SpacetimeDb::bsatn::Writer writer;
    
    // Determine type tag based on T
    if constexpr (std::is_same_v<T, bool>) {
        writer.write_u8(5);  // Bool
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        writer.write_u8(12); // U8
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        writer.write_u8(13); // U16
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        writer.write_u8(14); // U32
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        writer.write_u8(15); // U64
    } else if constexpr (std::is_same_v<T, int8_t>) {
        writer.write_u8(6);  // I8
    } else if constexpr (std::is_same_v<T, int16_t>) {
        writer.write_u8(7);  // I16
    } else if constexpr (std::is_same_v<T, int32_t>) {
        writer.write_u8(8);  // I32
    } else if constexpr (std::is_same_v<T, int64_t>) {
        writer.write_u8(9);  // I64
    } else if constexpr (std::is_same_v<T, float>) {
        writer.write_u8(18); // F32
    } else if constexpr (std::is_same_v<T, double>) {
        writer.write_u8(19); // F64
    } else if constexpr (std::is_same_v<T, std::string>) {
        writer.write_u8(4);  // String
    } else {
        // For user types, generate a product type
        writer.write_u8(2);  // Product
        writer.write_u32_le(0); // TODO: Add field count
    }
    
    return writer.take_buffer();
}

// Macro to specialize type generation for user structs
#define SPACETIMEDB_TYPE(Type, ...) \
    template<> \
    std::vector<uint8_t> spacetimedb_generate_type<Type>() { \
        SpacetimeDb::bsatn::Writer writer; \
        writer.write_u8(2); /* Product type */ \
        /* TODO: Generate field information */ \
        writer.write_u32_le(0); /* Field count */ \
        return writer.take_buffer(); \
    }

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
