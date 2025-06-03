#include "spacetimedb/internal/raw_module_def_v9.h"
#include "spacetimedb/internal/module_schema.h"
#include "spacetimedb/bsatn_all.h"
#include <iostream>

namespace SpacetimeDb {
    namespace Internal {

        // Serialize RawModuleDef enum
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawModuleDef& def) {
            // Serialize as enum: tag byte + variant data
            writer.write_u8(static_cast<uint8_t>(def.variant));
            
            switch (def.variant) {
                case RawModuleDefVariant::V8BackCompat:
                    // Not implemented - we only support V9
                    throw std::runtime_error("V8BackCompat not supported in C++ SDK");
                    break;
                    
                case RawModuleDefVariant::V9:
                    serialize(writer, def.v9_def);
                    break;
            }
        }

        // Serialize Typespace (simplified for now)
        void serialize(SpacetimeDb::bsatn::Writer& writer, const Typespace& ts) {
            // Typespace is a vector of AlgebraicType - serialize as empty vector for now
            writer.write_u32_le(0); // Empty vector length
        }

        // Serialize RawModuleDefV9
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawModuleDefV9& def) {
            // Serialize all fields in order
            serialize(writer, def.typespace);
            serialize(writer, def.tables);
            serialize(writer, def.reducers);
            serialize(writer, def.types);
            serialize(writer, def.misc_exports);
            serialize(writer, def.row_level_security);
        }

        // Serialize RawTableDefV9
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawTableDefV9& def) {
            serialize(writer, def.table_name);
            writer.write_u32_le(def.product_type_ref);
            serialize(writer, def.is_public);
        }

        // Serialize RawReducerDefV9
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawReducerDefV9& def) {
            serialize(writer, def.reducer_name);
            writer.write_u32_le(def.func_type_ref);
        }

        // Serialize RawTypeDefV9
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawTypeDefV9& def) {
            serialize(writer, def.type_name);
            writer.write_u32_le(def.algebraic_type_ref);
        }

        // Serialize RawMiscModuleExportV9 (empty)
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawMiscModuleExportV9& def) {
            // Empty struct - nothing to serialize
        }

        // Serialize RawRowLevelSecurityDefV9 (empty)
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawRowLevelSecurityDefV9& def) {
            // Empty struct - nothing to serialize
        }

        // Build RawModuleDef from ModuleSchema
        RawModuleDef build_raw_module_def_v9(const SpacetimeDb::ModuleSchema& user_schema) {
            RawModuleDef raw_def;
            raw_def.variant = RawModuleDefVariant::V9;
            
            RawModuleDefV9& v9_def = raw_def.v9_def;
            
            // Initialize typespace (empty for now)
            v9_def.typespace = Typespace{};
            
            // Convert tables
            for (const auto& [table_name, table_def] : user_schema.tables) {
                RawTableDefV9 raw_table;
                raw_table.table_name = table_def.spacetime_name;
                raw_table.product_type_ref = 0; // TODO: Proper type reference
                raw_table.is_public = table_def.is_public;
                v9_def.tables.push_back(raw_table);
            }
            
            // Convert reducers
            for (const auto& [reducer_name, reducer_def] : user_schema.reducers) {
                RawReducerDefV9 raw_reducer;
                raw_reducer.reducer_name = reducer_def.spacetime_name;
                raw_reducer.func_type_ref = 0; // TODO: Proper type reference
                v9_def.reducers.push_back(raw_reducer);
            }
            
            // Convert types
            for (const auto& [type_name, type_def] : user_schema.types) {
                RawTypeDefV9 raw_type;
                raw_type.type_name = type_def.spacetime_db_name;
                raw_type.algebraic_type_ref = 0; // TODO: Proper type reference
                v9_def.types.push_back(raw_type);
            }
            
            // Empty misc_exports and row_level_security for now
            v9_def.misc_exports.clear();
            v9_def.row_level_security.clear();
            
            return raw_def;
        }

        // Get serialized bytes
        std::vector<uint8_t> get_raw_module_def_v9_bytes() {
            // Get the user schema
            const SpacetimeDb::ModuleSchema& schema = SpacetimeDb::ModuleSchema::instance();
            
            // Debug: Print what we found in the schema
            std::cerr << "DEBUG: ModuleSchema contains:" << std::endl;
            std::cerr << "  Tables: " << schema.tables.size() << std::endl;
            for (const auto& [name, table] : schema.tables) {
                std::cerr << "    Table: " << name << " (spacetime_name: " << table.spacetime_name << ")" << std::endl;
            }
            std::cerr << "  Reducers: " << schema.reducers.size() << std::endl;
            for (const auto& [name, reducer] : schema.reducers) {
                std::cerr << "    Reducer: " << name << " (spacetime_name: " << reducer.spacetime_name << ")" << std::endl;
            }
            std::cerr << "  Types: " << schema.types.size() << std::endl;
            for (const auto& [name, type] : schema.types) {
                std::cerr << "    Type: " << name << " (spacetime_name: " << type.spacetime_db_name << ")" << std::endl;
            }
            
            // Build the raw module definition
            RawModuleDef raw_def = build_raw_module_def_v9(schema);
            
            // Serialize to BSATN
            SpacetimeDb::bsatn::Writer writer;
            serialize(writer, raw_def);
            
            std::cerr << "DEBUG: Serialized module def to " << writer.get_buffer().size() << " bytes" << std::endl;
            
            return writer.take_buffer();
        }

    } // namespace Internal
} // namespace SpacetimeDb