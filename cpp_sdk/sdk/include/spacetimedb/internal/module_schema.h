#ifndef SPACETIMEDB_INTERNAL_MODULE_SCHEMA_H // Renamed header guard
#define SPACETIMEDB_INTERNAL_MODULE_SCHEMA_H

#include "spacetimedb/bsatn/reader.h" // For bsatn::Reader

#include <string>
#include <vector>
#include <variant>
#include <functional> // For std::function
#include <map>
#include <cstddef> // For std::byte

// Forward declarations within the namespace
namespace bsatn { class Reader; } // Already included, but good practice if it were only forward needed by this header

namespace SpacetimeDb {
    // Forward declarations for types defined within this file, used in ModuleSchema
    struct TypeDefinition;
    struct TableDefinition;
    struct ReducerDefinition;
    struct FieldDefinition;
    struct EnumVariantDefinition;
    struct ReducerParameterDefinition;

    // Enum to represent basic SpacetimeDB types and user-defined types
    // This is used by the macros to describe types to the schema system.
    enum class CoreType {
        Bool, U8, U16, U32, U64, U128,
        I8, I16, I32, I64, I128,
        F32, F64,
        String, Bytes,
        UserDefined // For structs and enums by name
    };

    struct TypeIdentifier {
        CoreType core_type;
        std::string user_defined_name; // Empty if not UserDefined

        bool operator<(const TypeIdentifier& other) const { // For map keys
            if (core_type != other.core_type) return core_type < other.core_type;
            return user_defined_name < other.user_defined_name;
        }
         bool operator==(const TypeIdentifier& other) const {
            return core_type == other.core_type && user_defined_name == other.user_defined_name;
        }
    };

    struct FieldDefinition {
        std::string name;
        TypeIdentifier type;
        bool is_optional = false;
    };

    struct EnumVariantDefinition {
        std::string name;
    };

    struct StructDefinition {
        std::string cpp_name;
        std::string spacetime_db_name;
        std::vector<FieldDefinition> fields;
    };

    struct EnumDefinition {
        std::string cpp_name;
        std::string spacetime_db_name;
        std::vector<EnumVariantDefinition> variants;
    };

    struct TypeDefinition {
        std::string name; // The C++ name of the type (used as key in ModuleSchema::types)
        std::string spacetime_db_name;
        std::variant<StructDefinition, EnumDefinition> definition;
    };

    struct TableDefinition {
        std::string spacetime_name;
        std::string cpp_row_type_name;
        std::string primary_key_field_name;
    };

    struct ReducerParameterDefinition {
        std::string name;
        TypeIdentifier type;
    };

    struct ReducerDefinition {
        std::string spacetime_name;
        std::string cpp_function_name;
        std::vector<ReducerParameterDefinition> parameters;
        std::function<void(bsatn::Reader&)> invoker;
    };

    class ModuleSchema {
    public:
        std::map<std::string, TypeDefinition> types;
        std::map<std::string, TableDefinition> tables;
        std::map<std::string, ReducerDefinition> reducers;

        void register_struct_type(const std::string& cpp_name, const std::string& spacetimedb_name, const std::vector<FieldDefinition>& fields) {
            StructDefinition def_struct;
            def_struct.cpp_name = cpp_name;
            def_struct.spacetime_db_name = spacetimedb_name;
            def_struct.fields = fields;

            TypeDefinition type_def;
            type_def.name = cpp_name;
            type_def.spacetime_db_name = spacetimedb_name;
            type_def.definition = def_struct;
            types[cpp_name] = type_def;
        }

        void register_enum_type(const std::string& cpp_name, const std::string& spacetimedb_name, const std::vector<EnumVariantDefinition>& variants) {
            EnumDefinition def_enum;
            def_enum.cpp_name = cpp_name;
            def_enum.spacetime_db_name = spacetimedb_name;
            def_enum.variants = variants;

            TypeDefinition type_def;
            type_def.name = cpp_name;
            type_def.spacetime_db_name = spacetimedb_name;
            type_def.definition = def_enum;
            types[cpp_name] = type_def;
        }

        void register_table(const std::string& cpp_row_type, const std::string& spacetime_db_table_name) {
            TableDefinition def;
            def.cpp_row_type_name = cpp_row_type;
            def.spacetime_name = spacetime_db_table_name;
            tables[spacetime_db_table_name] = def;
        }

        void set_primary_key(const std::string& spacetime_db_table_name, const std::string& pk_field_name) {
            auto it = tables.find(spacetime_db_table_name);
            if (it != tables.end()) {
                it->second.primary_key_field_name = pk_field_name;
            } else {
                // Consider logging an error or throwing if table not found
            }
        }

        void register_reducer(const std::string& spacetimedb_name,
                              const std::string& cpp_func_name,
                              const std::vector<ReducerParameterDefinition>& params,
                              std::function<void(bsatn::Reader&)> invoker_func) {
            ReducerDefinition def;
            def.spacetime_name = spacetimedb_name;
            def.cpp_function_name = cpp_func_name;
            def.parameters = params;
            def.invoker = std::move(invoker_func);
            reducers[spacetimedb_name] = def;
        }

        static ModuleSchema& instance() {
            static ModuleSchema schema; // Singleton instance
            return schema;
        }
    private:
        ModuleSchema() = default;
        ModuleSchema(const ModuleSchema&) = delete;
        ModuleSchema& operator=(const ModuleSchema&) = delete;
    };
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_MODULE_SCHEMA_H
