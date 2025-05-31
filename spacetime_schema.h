#ifndef SPACETIME_SCHEMA_H
#define SPACETIME_SCHEMA_H

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <map>
#include <vector> // For std::vector<std::byte>
#include <cstddef> // For std::byte

// Forward declarations
namespace SpacetimeDb {
    struct TypeDefinition;
    struct TableDefinition;
    struct ReducerDefinition;

    // Enum to represent basic SpacetimeDB types and user-defined types
    enum class CoreType {
        Bool, U8, U16, U32, U64, U128, // U256 removed for simplicity, can be added later
        I8, I16, I32, I64, I128, // I256 removed for simplicity
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
        // Potentially a TypeIdentifier if enums can have associated data
    };

    struct StructDefinition {
        std::string cpp_name; // C++ name of the struct
        std::string spacetime_db_name; // SpacetimeDB name (can be different)
        std::vector<FieldDefinition> fields;
    };

    struct EnumDefinition {
        std::string cpp_name; // C++ name of the enum
        std::string spacetime_db_name; // SpacetimeDB name
        std::vector<EnumVariantDefinition> variants;
    };

    struct TypeDefinition {
        std::string name; // The C++ name of the type (used as key in ModuleSchema::types)
        std::string spacetime_db_name; // The name as it should appear in SpacetimeDB schema
        std::variant<StructDefinition, EnumDefinition> definition;
    };

    struct TableDefinition {
        std::string spacetime_name;
        std::string cpp_row_type_name; // References a TypeDefinition (must be a struct)
        std::string primary_key_field_name; // Empty if no PK
    };

    using ReducerFunctionWrapper = std::function<void(const std::vector<std::byte>& /* packed_args */)>;

    struct ReducerParameterDefinition {
        std::string name;
        TypeIdentifier type;
    };

    struct ReducerDefinition {
        std::string spacetime_name;
        std::string cpp_function_name; // For informational purposes
        std::vector<ReducerParameterDefinition> parameters;
        std::function<void(bsatn::Reader&)> invoker; // Type-erased invoker
    };

    class ModuleSchema {
    public:
        std::map<std::string, TypeDefinition> types; // Keyed by C++ type name
        std::map<std::string, TableDefinition> tables; // Keyed by C++ row type name for now for easy PK association
        std::map<std::string, ReducerDefinition> reducers; // Keyed by SpacetimeDB reducer name

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
            type_def.spacetime_db_name = spacetimedb_name; // Assume same for now, or add another param
            type_def.definition = def_enum;
            types[cpp_name] = type_def;
        }

        void register_table(const std::string& cpp_row_type, const std::string& spacetime_db_table_name) {
            TableDefinition def;
            def.cpp_row_type_name = cpp_row_type;
            def.spacetime_name = spacetime_db_table_name;
            // Use cpp_row_type_name as key for now, assuming one table def per C++ type.
            // If multiple tables per C++ type are needed with different SpacetimeDB names,
            // the key for `tables` map might need to be `spacetime_db_table_name` or a composite.
            // For now, let's stick to the design doc's implication that CppRowType is the key and PK is added later.
            // The design doc states: `std::vector<TableDefinition> tables;` which implies multiple tables are possible.
            // Let's change `tables` to be a vector as per the design doc.
            // We'll need a way to find a table to set its PK.
            // Let's make `tables` a map keyed by `spacetime_name` as that's more unique for lookup.
            tables[spacetime_db_table_name] = def;
        }

        void set_primary_key(const std::string& spacetime_db_table_name, const std::string& pk_field_name) {
            auto it = tables.find(spacetime_db_table_name);
            if (it != tables.end()) {
                it->second.primary_key_field_name = pk_field_name;
            } else {
                // Handle error: table not found
                // For now, just ignore or throw an exception in a real scenario
            }
        }

        // Updated register_reducer to accept an invoker
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
            static ModuleSchema schema;
            return schema;
        }
    private:
        ModuleSchema() = default;
        ModuleSchema(const ModuleSchema&) = delete;
        ModuleSchema& operator=(const ModuleSchema&) = delete;
    };
} // namespace SpacetimeDb

#endif // SPACETIME_SCHEMA_H
