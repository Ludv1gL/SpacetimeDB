#ifndef SPACETIMEDB_INTERNAL_MODULE_SCHEMA_H // Renamed header guard
#define SPACETIMEDB_INTERNAL_MODULE_SCHEMA_H

#include "spacetimedb/bsatn_all.h" // For BSATN types

#include <string>
#include <vector>
#include <variant>
#include <functional> // For std::function
#include <map>
#include <cstddef> // For std::byte

// Forward declarations within the namespace
namespace bsatn { class Reader; } // Already included, but good practice if it were only forward needed by this header

namespace SpacetimeDb {
    enum class ReducerKind {
        None, // Regular named reducer
        Init,
        ClientConnected,
        ClientDisconnected,
        Scheduled // For reducers linked to scheduled tables
    };

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
        Bool, U8, U16, U32, U64, U128, U256,
        I8, I16, I32, I64, I128, I256,
        F32, F64,
        String, Bytes,
        UserDefined // For structs and enums by name
        // ScheduleAt will be a UserDefined type for now
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
        bool is_unique = false; // Default to not unique
        bool is_auto_increment = false; // Default to false
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

    struct IndexDefinition {
        std::string index_name;
        std::vector<std::string> column_field_names;
        // bool is_unique_index = false; // Optional: Consider for future enhancement
    };

    struct TableDefinition {
        std::string spacetime_name;
        std::string cpp_row_type_name;
        std::string primary_key_field_name;
        bool is_public = false; // Default to private
        std::vector<IndexDefinition> indexes;
        std::string scheduled_reducer_name; // Name of the reducer for scheduled tables
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
        ReducerKind kind = ReducerKind::None;
    };

    class ModuleSchema {
    public:
        std::map<std::string, TypeDefinition> types;
        std::map<std::string, TableDefinition> tables;
        std::map<std::string, ReducerDefinition> reducers;
        std::map<std::string, std::string> client_visibility_filters;

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

        void register_table(const std::string& cpp_row_type,
                            const std::string& spacetime_db_table_name,
                            bool is_public_table,
                            const std::string& scheduled_reducer_name_or_empty) {
            TableDefinition def;
            def.cpp_row_type_name = cpp_row_type;
            def.spacetime_name = spacetime_db_table_name;
            def.is_public = is_public_table;
            def.scheduled_reducer_name = scheduled_reducer_name_or_empty;
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

        void add_index(const std::string& spacetime_db_table_name, const IndexDefinition& index_def) {
            auto it = tables.find(spacetime_db_table_name);
            if (it != tables.end()) {
                it->second.indexes.push_back(index_def);
            } else {
                // Consider logging an error or throwing if table not found
                // For now, do nothing if table not found to avoid exceptions during static init order issues.
                // A separate validation step could check for this.
            }
        }

        void register_reducer(const std::string& spacetimedb_name,
                              const std::string& cpp_func_name,
                              const std::vector<ReducerParameterDefinition>& params,
                              std::function<void(bsatn::Reader&)> invoker_func,
                              ReducerKind reducer_kind) {
            ReducerDefinition def;
            def.spacetime_name = spacetimedb_name;
            def.cpp_function_name = cpp_func_name;
            def.parameters = params;
            def.invoker = std::move(invoker_func);
            def.kind = reducer_kind;
            reducers[spacetimedb_name] = def;
        }

        void register_filter(const std::string& filter_name, const std::string& sql_string) {
            client_visibility_filters[filter_name] = sql_string;
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
