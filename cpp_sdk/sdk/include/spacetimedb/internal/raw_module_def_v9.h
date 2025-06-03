#ifndef SPACETIMEDB_INTERNAL_RAW_MODULE_DEF_V9_H
#define SPACETIMEDB_INTERNAL_RAW_MODULE_DEF_V9_H

#include "spacetimedb/bsatn_all.h"
#include "spacetimedb/internal/module_schema.h"
#include <string>
#include <vector>
#include <cstdint>

namespace SpacetimeDb {
    namespace Internal {

        // Forward declarations
        struct RawTableDefV9;
        struct RawReducerDefV9;
        struct RawTypeDefV9;
        struct RawMiscModuleExportV9;
        struct RawRowLevelSecurityDefV9;

        // Typespace structure 
        struct Typespace {
            std::vector<std::vector<uint8_t>> types;  // Vector of AlgebraicType serializations
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const Typespace& ts);

        // RawModuleDefV9 structure that matches Rust definition
        struct RawModuleDefV9 {
            Typespace typespace;
            std::vector<RawTableDefV9> tables;
            std::vector<RawReducerDefV9> reducers;
            std::vector<RawTypeDefV9> types;
            std::vector<RawMiscModuleExportV9> misc_exports;
            std::vector<RawRowLevelSecurityDefV9> row_level_security;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawModuleDefV9& def);

        // RawModuleDef enum that matches Rust definition
        enum class RawModuleDefVariant : uint8_t {
            V8BackCompat = 0,
            V9 = 1
        };

        struct RawModuleDef {
            RawModuleDefVariant variant;
            RawModuleDefV9 v9_def;  // Only used when variant == V9
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawModuleDef& def);

        // Table definition (simplified for now)
        struct RawTableDefV9 {
            std::string table_name;
            uint32_t product_type_ref;  // AlgebraicTypeRef to the row type
            bool is_public;
            // TODO: Add other fields like primary_key, indexes, constraints, etc.
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawTableDefV9& def);

        // Reducer definition (simplified for now)
        struct RawReducerDefV9 {
            std::string reducer_name;
            uint32_t func_type_ref;  // AlgebraicTypeRef to function signature
            // TODO: Add lifecycle, etc.
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawReducerDefV9& def);

        // Type definition (simplified for now)
        struct RawTypeDefV9 {
            std::string type_name;
            uint32_t algebraic_type_ref;
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawTypeDefV9& def);

        // Miscellaneous exports (empty for now)
        struct RawMiscModuleExportV9 {
            // Empty for now
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawMiscModuleExportV9& def);

        // Row level security (empty for now)
        struct RawRowLevelSecurityDefV9 {
            // Empty for now
        };
        void serialize(SpacetimeDb::bsatn::Writer& writer, const RawRowLevelSecurityDefV9& def);

        // Builder function
        RawModuleDef build_raw_module_def_v9(const SpacetimeDb::ModuleSchema& user_schema);

        // Get serialized bytes for the module definition
        std::vector<uint8_t> get_raw_module_def_v9_bytes();

    } // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_RAW_MODULE_DEF_V9_H