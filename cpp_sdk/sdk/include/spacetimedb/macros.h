#ifndef SPACETIMEDB_MACROS_H
#define SPACETIMEDB_MACROS_H

#include "spacetimedb/internal/module_schema.h" // Updated path
#include "spacetimedb/bsatn/reader.h"           // Added for bsatn::Reader
#include "spacetimedb/bsatn/writer.h"           // Added for bsatn::Writer

#include <string>
#include <vector>
#include <utility> // For std::make_pair for potential future use, not directly now
#include <tuple>   // For std::tuple, std::apply (used in SPACETIMEDB_REDUCER)
#include <functional> // For std::function (used in ReducerDefinition)


// Helper macro to stringify its argument
#define SPACETIMEDB_STRINGIFY_IMPL(x) #x
#define SPACETIMEDB_STRINGIFY(x) SPACETIMEDB_STRINGIFY_IMPL(x)

// --- Type Definition Macros ---

/**
 * @file spacetime_macros.h
 * @brief Defines macros for SpacetimeDB C++ module schema definition, type registration,
 *        and BSATN (de)serialization function generation.
 * @details These macros help users define their data model (structs, enums, tables, reducers)
 *          and automatically register this schema with the SpacetimeDB C++ SDK.
 *          For types defined with `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS` and `SPACETIMEDB_TYPE_ENUM`,
 *          BSATN serialization and deserialization functions are also generated.
 */

// Helper for defining fields within SPACETIMEDB_TYPE_STRUCT
// Usage: SPACETIMEDB_FIELD("fieldName", SpacetimeDb::CoreType::U64)
// Or for user-defined types: SPACETIMEDB_FIELD_CUSTOM("fieldName", "MyCustomStructName")
// Or for optional fields: SPACETIMEDB_FIELD_OPTIONAL("fieldName", SpacetimeDb::CoreType::String)
// Or for optional custom: SPACETIMEDB_FIELD_CUSTOM_OPTIONAL("fieldName", "MyCustomStructName")

/** @internal Basic helper to construct a FieldDefinition for schema registration. */
inline SpacetimeDb::FieldDefinition SPACETIMEDB_FIELD_INTERNAL(const char* name, SpacetimeDb::CoreType core_type, const char* user_defined_name, bool is_optional) {
    SpacetimeDb::TypeIdentifier type_id;
    type_id.core_type = core_type;
    if (user_defined_name) {
        type_id.user_defined_name = user_defined_name;
    }
    SpacetimeDb::FieldDefinition field_def;
    field_def.name = name;
    field_def.type = type_id;
    field_def.is_optional = is_optional;
    return field_def;
}

/**
 * @brief Defines a basic field for schema registration.
 * @param FieldNameStr The string name of the field.
 * @param FieldCoreType The `SpacetimeDb::CoreType` of the field (e.g., `SpacetimeDb::CoreType::U64`).
 */
#define SPACETIMEDB_FIELD(FieldNameStr, FieldCoreType) \
    SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, false)

/**
 * @brief Defines an optional basic field for schema registration.
 * @param FieldNameStr The string name of the field.
 * @param FieldCoreType The `SpacetimeDb::CoreType` of the field's value (e.g., `SpacetimeDb::CoreType::String`).
 */
#define SPACETIMEDB_FIELD_OPTIONAL(FieldNameStr, FieldCoreType) \
    SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, true)

/**
 * @brief Defines a field of a user-defined type for schema registration.
 * @param FieldNameStr The string name of the field.
 * @param UserDefinedTypeNameStr The string name of the user-defined C++ type (must be registered separately).
 */
#define SPACETIMEDB_FIELD_CUSTOM(FieldNameStr, UserDefinedTypeNameStr) \
    SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr, false)

/**
 * @brief Defines an optional field of a user-defined type for schema registration.
 * @param FieldNameStr The string name of the field.
 * @param UserDefinedTypeNameStr The string name of the user-defined C++ type (must be registered separately).
 */
#define SPACETIMEDB_FIELD_CUSTOM_OPTIONAL(FieldNameStr, UserDefinedTypeNameStr) \
    SpacetimeDb::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr, true)


/**
 * @brief Registers a C++ struct with SpacetimeDB for schema definition.
 * @details The C++ struct (e.g., `struct Player { ... };`) must be defined manually by the user.
 *          This macro only handles registering the type and its fields with the SpacetimeDB schema.
 *          It does **not** generate BSATN (de)serialization functions. For that, use
 *          `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS`.
 * @param CppTypeName The C++ name of the struct (e.g., `Player`).
 * @param SpacetimeDbTypeNameStr The string name of this type as it should appear in the SpacetimeDB schema (e.g., "PlayerDbName").
 * @param FieldsInitializerList An initializer list of `SpacetimeDb::FieldDefinition` objects, typically
 *                              created using `SPACETIMEDB_FIELD`, `SPACETIMEDB_FIELD_OPTIONAL`, etc.
 *                              Example: `{ SPACETIMEDB_FIELD("id", SpacetimeDb::CoreType::U64), SPACETIMEDB_FIELD("name", SpacetimeDb::CoreType::String) }`
 * @ingroup schema_definition
 * @see SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS
 * @see SPACETIMEDB_FIELD
 */
#define SPACETIMEDB_TYPE_STRUCT(CppTypeName, SpacetimeDbTypeNameStr, FieldsInitializerList) \
    /* static_assert(true, "Ensuring semicolon at end of macro call"); // Dummy static assert for semicolon */ \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##CppTypeName { \
            Register##CppTypeName() { \
                SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbTypeNameStr, \
                    std::vector<SpacetimeDb::FieldDefinition> FieldsInitializerList \
                ); \
            } \
        }; \
        static Register##CppTypeName register_##CppTypeName##_instance; \
    }}


/**
 * @brief Helper to define an enum variant for schema registration.
 * @param VariantNameStr The string name of the enum variant.
 * @return A `SpacetimeDb::EnumVariantDefinition` object.
 */
#define SPACETIMEDB_ENUM_VARIANT(VariantNameStr) \
    SpacetimeDb::EnumVariantDefinition{VariantNameStr}

/**
 * @brief Registers a C++ `enum class` with SpacetimeDB and generates its BSATN (de)serialization functions.
 * @details The C++ `enum class` (e.g., `enum class MyStatus : uint8_t { ... };`) must be defined manually by the user.
 *          It is **required** that the enum has an underlying type of `uint8_t`.
 *          This macro handles both schema registration and generation of `SpacetimeDB::bsatn::serialize` and
 *          `SpacetimeDB::bsatn::deserialize_CppEnumName` functions.
 * @param CppEnumName The C++ name of the enum class (e.g., `PlayerStatus`).
 * @param SpacetimeDbEnumNameStr The string name of this enum as it should appear in the SpacetimeDB schema (e.g., "PlayerStatusDbName").
 * @param VariantsInitializerList An initializer list of `SpacetimeDb::EnumVariantDefinition` objects, typically
 *                                created using `SPACETIMEDB_ENUM_VARIANT`.
 *                                Example: `{ SPACETIMEDB_ENUM_VARIANT("Online"), SPACETIMEDB_ENUM_VARIANT("Offline") }`
 * @ingroup schema_definition
 * @see SPACETIMEDB_ENUM_VARIANT
 */
#define SPACETIMEDB_TYPE_ENUM(CppEnumName, SpacetimeDbEnumNameStr, VariantsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##CppEnumName { \
            Register##CppEnumName() { \
                SpacetimeDb::ModuleSchema::instance().register_enum_type( \
                    SPACETIMEDB_STRINGIFY(CppEnumName), \
                    SpacetimeDbEnumNameStr, \
                    std::vector<SpacetimeDb::EnumVariantDefinition> VariantsInitializerList \
                ); \
            } \
        }; \
        static Register##CppEnumName register_##CppEnumName##_instance; \
    }} \
    namespace SpacetimeDB { namespace bsatn { \
        inline void serialize(::bsatn::Writer& writer, const CppEnumName& value) { \
            writer.write_u8(static_cast<uint8_t>(value)); \
        } \
        inline CppEnumName deserialize_##CppEnumName(::bsatn::Reader& reader) { \
            uint8_t val = reader.read_u8(); \
            /* TODO: Add validation if val is a valid enum member based on VariantsInitializerList if possible/needed */ \
            return static_cast<CppEnumName>(val); \
        } \
    }} /* ; intentionally omitted to require semicolon from user */


// --- Table Definition Macros ---

/**
 * @brief Declares a SpacetimeDB table for schema registration.
 * @details Associates a C++ row type (previously registered, typically with `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS`
 *          or `SPACETIMEDB_TYPE_STRUCT`) with a table name in the SpacetimeDB schema.
 * @param CppRowTypeName The C++ name of the struct representing a row in this table (e.g., `Player`).
 * @param SpacetimeDbTableNameStr The string name of this table as it should appear in the SpacetimeDB schema (e.g., "PlayersTable").
 * @ingroup schema_definition
 */
#define SPACETIMEDB_TABLE(CppRowTypeName, SpacetimeDbTableNameStr) \
    /* static_assert(true, "Ensuring semicolon at end of macro call"); // Dummy static assert for semicolon */ \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        /* Using a simple concatenation for struct name; complex table names might need mangling for uniqueness. */ \
        struct RegisterTable_##CppRowTypeName##_Default { \
            /* Default suffix used if table name is not simple enough for identifier */ \
            /* A better approach might involve hashing or a counter for unique static var names */ \
            RegisterTable_##CppRowTypeName##_Default() { \
            /*RegisterTable_##CppRowTypeName##_##SpacetimeDbTableNameStr() {*/ \
                /* Replace invalid chars in SpacetimeDbTableNameStr for unique struct name */ \
                SpacetimeDb::ModuleSchema::instance().register_table( \
                    SPACETIMEDB_STRINGIFY(CppRowTypeName), \
                    SpacetimeDbTableNameStr \
                ); \
            } \
        }; \
        /* Using CppRowTypeName for static var name for simplicity, assuming one table per CppRowType for this registration style. */ \
        /* If multiple tables use the same CppRowType, this static var name would collide. */ \
        /* The registration key in ModuleSchema is SpacetimeDbTableNameStr, which is fine. */ \
        static RegisterTable_##CppRowTypeName##_Default register_table_##CppRowTypeName##_instance; \
    }}

/**
 * @brief Specifies the primary key for a previously declared SpacetimeDB table.
 * @param SpacetimeDbTableNameStr The string name of the table (must match the name used in `SPACETIMEDB_TABLE`).
 * @param FieldNameStr The string name of the field within the table's row type that serves as the primary key.
 * @ingroup schema_definition
 */
#define SPACETIMEDB_PRIMARY_KEY(SpacetimeDbTableNameStr, FieldNameStr) \
    /* static_assert(true, "Ensuring semicolon at end of macro call"); // Dummy static assert for semicolon */ \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        /* Using a simple concatenation for struct name; complex table/field names might need mangling. */ \
        struct SetPrimaryKey_Default_##FieldNameStr { \
            /* A better approach might involve hashing or a counter for unique static var names */ \
             SetPrimaryKey_Default_##FieldNameStr() { \
                SpacetimeDb::ModuleSchema::instance().set_primary_key( \
                    SpacetimeDbTableNameStr, \
                    FieldNameStr \
                ); \
            } \
        }; \
        /* Using FieldNameStr for static var name for simplicity, assuming one PK per field name (globally) for this style. */ \
        static SetPrimaryKey_Default_##FieldNameStr set_pk_##FieldNameStr##_instance; \
    }}


// --- Reducer Definition Macros ---

/**
 * @brief Helper to define a reducer parameter for schema registration.
 * @param ParamNameStr The string name of the parameter.
 * @param ParamCoreType The `SpacetimeDb::CoreType` of the parameter.
 * @return A `SpacetimeDb::ReducerParameterDefinition` object.
 */
#define SPACETIMEDB_REDUCER_PARAM(ParamNameStr, ParamCoreType) \
    SpacetimeDb::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, ParamCoreType, nullptr)

/**
 * @brief Helper to define a reducer parameter of a user-defined type for schema registration.
 * @param ParamNameStr The string name of the parameter.
 * @param UserDefinedTypeNameStr The string name of the user-defined C++ type of the parameter.
 * @return A `SpacetimeDb::ReducerParameterDefinition` object.
 */
#define SPACETIMEDB_REDUCER_PARAM_CUSTOM(ParamNameStr, UserDefinedTypeNameStr) \
    SpacetimeDb::SPACETIMEDB_REDUCER_PARAM_INTERNAL(ParamNameStr, SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr)

/** @internal Basic helper to construct a ReducerParameterDefinition for schema registration. */
inline SpacetimeDb::ReducerParameterDefinition SPACETIMEDB_REDUCER_PARAM_INTERNAL(const char* name, SpacetimeDb::CoreType core_type, const char* user_defined_name) {
    SpacetimeDb::TypeIdentifier type_id;
    type_id.core_type = core_type;
    if (user_defined_name) {
        type_id.user_defined_name = user_defined_name;
    }
    SpacetimeDb::ReducerParameterDefinition param_def;
    param_def.name = name;
    param_def.type = type_id;
    return param_def;
}

/** @internal Not directly used by new SPACETIMEDB_REDUCER, kept for reference or potential future use. */
#define SPACETIMEDB_XX_DESERIALIZE_REDUCER_ARG_AND_PASS(ParamCppType, ParamName, ReaderName, ArgsVectorName) \
    ArgsVectorName.push_back(std::make_any<ParamCppType>(bsatn::deserialize<ParamCppType>(ReaderName)));

/** @internal Not directly used by new SPACETIMEDB_REDUCER, kept for reference. */
#define SPACETIMEDB_REDUCER_ARG_DECLARE_HELPER(ParamCppType, ParamName, reader_instance) \
    ParamCppType ParamName = bsatn::deserialize<ParamCppType>(reader_instance);


/**
 * @brief Registers a C++ function as a SpacetimeDB reducer and generates its invoker.
 * @details This macro registers the reducer with the SpacetimeDB schema and creates a type-erased
 *          invoker function. The invoker handles deserializing arguments from BSATN format
 *          and calling the actual C++ reducer function.
 *          The C++ reducer function (e.g., `void my_reducer(uint64_t p1, std::string p2);`)
 *          must be defined by the user.
 * @param SpacetimeDbReducerNameStr The string name of this reducer as it should be known to SpacetimeDB.
 * @param CppFunctionName The C++ name of the reducer function (e.g., `my_reducer`).
 * @param RegParamsInitializerList An initializer list of `SpacetimeDb::ReducerParameterDefinition` for schema
 *                                 registration, created using `SPACETIMEDB_REDUCER_PARAM` or `SPACETIMEDB_REDUCER_PARAM_CUSTOM`.
 *                                 Example: `{ SPACETIMEDB_REDUCER_PARAM("p1", SpacetimeDb::CoreType::U64), ... }`
 * @param ... A variable argument list of the C++ types of the reducer function's parameters, in order.
 *            Example: `uint64_t, std::string` for `void my_reducer(uint64_t, std::string);`
 * @ingroup schema_definition
 * @note Requires C++17 for `std::apply` and fold expressions in the invoker.
 * @see SPACETIMEDB_REDUCER_PARAM
 * @see SPACETIMEDB_REDUCER_PARAM_CUSTOM
 *
 * Example Usage:
 * ```cpp
 * void my_actual_reducer(uint64_t id, std::string name);
 * SPACETIMEDB_REDUCER("CreatePlayer", my_actual_reducer,
 *    { SPACETIMEDB_REDUCER_PARAM("id", SpacetimeDb::CoreType::U64),
 *      SPACETIMEDB_REDUCER_PARAM("name", SpacetimeDb::CoreType::String) },
 *    uint64_t, std::string // These are the __VA_ARGS__ for types
 * );
 * ```
 */
#define SPACETIMEDB_REDUCER(SpacetimeDbReducerNameStr, CppFunctionName, RegParamsInitializerList, ...) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct RegisterReducer_##CppFunctionName { \
            RegisterReducer_##CppFunctionName() { \
                /* Define a helper struct to unpack __VA_ARGS__ into arguments for CppFunctionName */ \
                /* This assumes CppFunctionName is a free function or static member. */ \
                /* __VA_ARGS__ here are the C++ types of the reducer parameters */ \
                auto invoker_lambda = [](::bsatn::Reader& reader_param) { \
                    /* Deserialize arguments based on __VA_ARGS__ types */ \
                    std::tuple<__VA_ARGS__> args_tuple = \
                        std::make_tuple(::bsatn::deserialize<__VA_ARGS__>(reader_param)...); \
                    /* Call the actual function using std::apply */ \
                    std::apply(CppFunctionName, args_tuple); \
                }; \
                SpacetimeDb::ModuleSchema::instance().register_reducer( \
                    SpacetimeDbReducerNameStr, \
                    SPACETIMEDB_STRINGIFY(CppFunctionName), \
                    std::vector<SpacetimeDb::ReducerParameterDefinition> RegParamsInitializerList, \
                    invoker_lambda \
                ); \
            } \
        }; \
        static RegisterReducer_##CppFunctionName register_reducer_##CppFunctionName##_instance; \
    }} /* ; intentionally omitted */

// Example Usage:
// void my_actual_reducer(uint64_t id, std::string name);
// SPACETIMEDB_REDUCER("CreatePlayer", my_actual_reducer,
//    { SPACETIMEDB_REDUCER_PARAM("id", SpacetimeDb::CoreType::U64),
//      SPACETIMEDB_REDUCER_PARAM("name", SpacetimeDb::CoreType::String) },
//    uint64_t, std::string // These are the __VA_ARGS__ for types
// );


// --- X-Macro Helpers for Struct Serialization ---
// These macros are used by the FIELDS_MACRO passed to SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS

// XX_SERIALIZE_FIELD(WRITER_NAME, STRUCT_INSTANCE_NAME, CPP_TYPE_OR_ELEMENT_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR)
#define SPACETIMEDB_XX_SERIALIZE_FIELD(WRITER, VALUE_OBJ, CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        (WRITER).write_optional((VALUE_OBJ).FIELD_NAME); /* Relies on Writer's template write_optional calling bsatn::serialize */ \
    } else if constexpr (IS_VECTOR) { \
        (WRITER).write_vector((VALUE_OBJ).FIELD_NAME); /* Relies on Writer's template write_vector calling bsatn::serialize */ \
    } else { \
        SpacetimeDB::bsatn::serialize((WRITER), (VALUE_OBJ).FIELD_NAME); \
    }

// XX_DESERIALIZE_FIELD(READER_NAME, STRUCT_INSTANCE_NAME, CPP_TYPE_OR_ELEMENT_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR)
// Note: For IS_VECTOR, CPP_TYPE is the ELEMENT type of the vector.
// Uses bsatn::deserialize<T>(reader) which relies on specializations for T.
#define SPACETIMEDB_XX_DESERIALIZE_FIELD(READER, VALUE_OBJ, CPP_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR) \
    if constexpr (IS_OPTIONAL) { \
        (VALUE_OBJ).FIELD_NAME = (READER).read_optional<CPP_TYPE>(); /* Relies on Reader's template read_optional calling bsatn::deserialize */ \
    } else if constexpr (IS_VECTOR) { \
        (VALUE_OBJ).FIELD_NAME = (READER).read_vector<CPP_TYPE>(); /* Relies on Reader's template read_vector calling bsatn::deserialize */ \
    } else { \
        (VALUE_OBJ).FIELD_NAME = ::bsatn::deserialize<CPP_TYPE>((READER)); \
    }

/**
 * @brief Registers a C++ struct, generates BSATN (de)serialization, using an X-Macro for fields.
 * @details The C++ struct must be defined manually. This macro handles schema registration and
 *          generates `SpacetimeDB::bsatn::serialize` and `SpacetimeDB::bsatn::deserialize_CppTypeName`
 *          functions, along with the `bsatn::deserialize<CppTypeName>` specialization.
 *          The user defines a `FIELDS_MACRO(XX)` which calls `XX(CPP_TYPE_OR_ELEMENT_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR)`
 *          for each field.
 *          - For regular fields: `CPP_TYPE_OR_ELEMENT_TYPE` is the type of the field.
 *          - For `std::vector<T>` fields: `CPP_TYPE_OR_ELEMENT_TYPE` is the element type `T`.
 *          - For `std::optional<T>` fields: `CPP_TYPE_OR_ELEMENT_TYPE` is the underlying type `T`.
 * @param CppTypeName The C++ name of the struct (e.g., `MyStruct`).
 * @param SpacetimeDbNameStr The string name of this type in the SpacetimeDB schema.
 * @param FIELDS_MACRO An X-Macro defined by the user that lists the struct's fields.
 * @param RegFieldsInitializerList An initializer list of `SpacetimeDb::FieldDefinition` for schema registration.
 * @ingroup schema_definition
 * @see SPACETIMEDB_XX_SERIALIZE_FIELD
 * @see SPACETIMEDB_XX_DESERIALIZE_FIELD
 *
 * Example:
 * ```cpp
 * struct MyStruct { uint64_t id; std::optional<std::string> name; std::vector<MyOtherStruct> items; };
 * #define MY_STRUCT_FIELDS(XX) \
 *    XX(uint64_t, id, false, false) \
 *    XX(std::string, name, true, false) \
 *    XX(MyOtherStruct, items, false, true)
 * SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(MyStruct, "MyStructDB", MY_STRUCT_FIELDS,
 *     { SPACETIMEDB_FIELD("id", SpacetimeDb::CoreType::U64), ... });
 * ```
 */
#define SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(CppTypeName, SpacetimeDbNameStr, FIELDS_MACRO, RegFieldsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##CppTypeName { \
            Register##CppTypeName() { \
                SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbNameStr, \
                    std::vector<SpacetimeDb::FieldDefinition> RegFieldsInitializerList \
                ); \
            } \
        }; \
        static Register##CppTypeName register_##CppTypeName##_instance; \
    }} \
    namespace SpacetimeDB { namespace bsatn { \
        /* Forward declare deserialize for potential recursive types */ \
        CppTypeName deserialize_##CppTypeName(::bsatn::Reader& reader); \
        \
        inline void serialize(::bsatn::Writer& writer, const CppTypeName& value) { \
            FIELDS_MACRO(SPACETIMEDB_XX_SERIALIZE_FIELD, writer, value) \
        } \
        \
        /* Define the deserialize function, matching the SpacetimeDB::bsatn::deserialize_TypeName convention */ \
        /* This also serves as the function that bsatn::deserialize<CppTypeName> will specialize to call. */ \
        inline CppTypeName deserialize_##CppTypeName(::bsatn::Reader& reader) { \
            CppTypeName obj; \
            FIELDS_MACRO(SPACETIMEDB_XX_DESERIALIZE_FIELD, reader, obj) \
            return obj; \
        } \
        \
    }} /* ; intentionally omitted */ \
    /* Specialization for bsatn::deserialize<CppTypeName> must be in bsatn namespace */ \
    namespace bsatn { \
        template<> \
        inline CppTypeName deserialize<CppTypeName>(Reader& reader) { \
            return SpacetimeDB::bsatn::deserialize_##CppTypeName(reader); \
        } \
    }


// Original SPACETIMEDB_TYPE_STRUCT kept for now, users should migrate to _WITH_FIELDS for auto bsatn
// This macro will NOT generate bsatn functions.
#define SPACETIMEDB_TYPE_STRUCT(CppTypeName, SpacetimeDbTypeNameStr, FieldsInitializerList) \
    namespace SpacetimeDb { namespace ModuleRegistration { \
        struct Register##CppTypeName { \
            Register##CppTypeName() { \
                SpacetimeDb::ModuleSchema::instance().register_struct_type( \
                    SPACETIMEDB_STRINGIFY(CppTypeName), \
                    SpacetimeDbTypeNameStr, \
                    std::vector<SpacetimeDb::FieldDefinition> FieldsInitializerList \
                ); \
            } \
        }; \
        static Register##CppTypeName register_##CppTypeName##_instance; \
    }} /* ; intentionally omitted */


#endif // SPACETIME_MACROS_H
