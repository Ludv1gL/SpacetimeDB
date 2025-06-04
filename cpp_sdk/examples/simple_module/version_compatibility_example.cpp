/**
 * Example: Version Compatibility and Backward Compatible Changes
 * 
 * This example shows how to make backward-compatible changes
 * within the same major version (1.x.x)
 */

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/versioning.h>
#include <spacetimedb/migration.h>

// Module versions progression: 1.0.0 -> 1.1.0 -> 1.2.0
SPACETIMEDB_MODULE_VERSION(1, 2, 0)

SPACETIMEDB_MODULE_METADATA(
    "CompatibleEvolution",
    "Example Author",
    "Demonstrates backward-compatible schema evolution",
    "MIT"
)

// Core schema (unchanged since 1.0.0)
struct Product {
    uint64_t id;
    std::string name;
    std::string description;
    uint64_t price_cents;
    uint64_t created_at;
};

// Added in 1.1.0 - new table (backward compatible)
struct ProductCategory {
    uint64_t id;
    std::string name;
    std::optional<uint64_t> parent_id;
};

// Added in 1.2.0 - another new table (backward compatible)
struct ProductTag {
    uint64_t product_id;
    std::string tag;
};

// Register fields
SPACETIMEDB_REGISTER_FIELDS(Product,
    SPACETIMEDB_FIELD(Product, id, uint64_t);
    SPACETIMEDB_FIELD(Product, name, std::string);
    SPACETIMEDB_FIELD(Product, description, std::string);
    SPACETIMEDB_FIELD(Product, price_cents, uint64_t);
    SPACETIMEDB_FIELD(Product, created_at, uint64_t);
)

SPACETIMEDB_REGISTER_FIELDS(ProductCategory,
    SPACETIMEDB_FIELD(ProductCategory, id, uint64_t);
    SPACETIMEDB_FIELD(ProductCategory, name, std::string);
    SPACETIMEDB_FIELD(ProductCategory, parent_id, std::optional<uint64_t>);
)

SPACETIMEDB_REGISTER_FIELDS(ProductTag,
    SPACETIMEDB_FIELD(ProductTag, product_id, uint64_t);
    SPACETIMEDB_FIELD(ProductTag, tag, std::string);
)

// Tables
SPACETIMEDB_TABLE(Product, products, true)
SPACETIMEDB_TABLE(ProductCategory, categories, true)  // Added in 1.1.0
SPACETIMEDB_TABLE(ProductTag, product_tags, true)     // Added in 1.2.0

// Version compatibility helper
class VersionCompatibility {
public:
    static bool check_compatibility(const spacetimedb::ModuleVersion& client_version,
                                  const spacetimedb::ModuleVersion& module_version) {
        // Same major version = compatible
        if (client_version.major == module_version.major) {
            // Client can use older module version
            if (client_version <= module_version) {
                return true;
            }
            // Module can serve older clients (backward compatibility)
            return true;
        }
        return false;
    }
    
    static std::string get_compatibility_message(const spacetimedb::ModuleVersion& client_version,
                                               const spacetimedb::ModuleVersion& module_version) {
        if (check_compatibility(client_version, module_version)) {
            return "Versions are compatible";
        }
        
        if (client_version.major != module_version.major) {
            return "Major version mismatch - not compatible";
        }
        
        return "Unknown compatibility issue";
    }
};

// Feature flags based on version
struct FeatureFlags {
    bool has_categories = true;      // Added in 1.1.0
    bool has_tags = true;           // Added in 1.2.0
    bool has_bulk_import = false;   // Planned for 1.3.0
    
    static FeatureFlags for_version(const spacetimedb::ModuleVersion& version) {
        FeatureFlags flags;
        
        // Categories added in 1.1.0
        if (version < spacetimedb::ModuleVersion{1, 1, 0}) {
            flags.has_categories = false;
        }
        
        // Tags added in 1.2.0
        if (version < spacetimedb::ModuleVersion{1, 2, 0}) {
            flags.has_tags = false;
        }
        
        return flags;
    }
};

// Reducers with version awareness
SPACETIMEDB_REDUCER(create_product, spacetimedb::ReducerContext ctx,
                   std::string name, std::string description, uint64_t price_cents) {
    static uint64_t next_id = 1;
    
    Product product{
        .id = next_id++,
        .name = name,
        .description = description,
        .price_cents = price_cents,
        .created_at = static_cast<uint64_t>(std::time(nullptr))
    };
    
    ctx.db.table<Product>("products").insert(product);
    spacetimedb::log("Created product: " + name);
}

// Added in 1.1.0
SPACETIMEDB_REDUCER(create_category, spacetimedb::ReducerContext ctx,
                   std::string name, std::optional<uint64_t> parent_id) {
    static uint64_t next_id = 1;
    
    ProductCategory category{
        .id = next_id++,
        .name = name,
        .parent_id = parent_id
    };
    
    ctx.db.table<ProductCategory>("categories").insert(category);
    spacetimedb::log("Created category: " + name);
}

// Added in 1.2.0
SPACETIMEDB_REDUCER(tag_product, spacetimedb::ReducerContext ctx,
                   uint64_t product_id, std::string tag) {
    ProductTag product_tag{
        .product_id = product_id,
        .tag = tag
    };
    
    ctx.db.table<ProductTag>("product_tags").insert(product_tag);
    spacetimedb::log("Tagged product " + std::to_string(product_id) + " with: " + tag);
}

// Version negotiation reducer
SPACETIMEDB_REDUCER(negotiate_version, spacetimedb::ReducerContext ctx,
                   std::string client_version_str) {
    auto client_version = spacetimedb::ModuleVersion::parse(client_version_str);
    auto module_version = MODULE_VERSION;
    
    bool compatible = VersionCompatibility::check_compatibility(client_version, module_version);
    std::string message = VersionCompatibility::get_compatibility_message(client_version, module_version);
    
    spacetimedb::log("Client version: " + client_version_str);
    spacetimedb::log("Module version: " + module_version.to_string());
    spacetimedb::log("Compatibility: " + message);
    
    // Return available features for client version
    auto features = FeatureFlags::for_version(client_version);
    spacetimedb::log("Available features:");
    spacetimedb::log("  - Categories: " + std::string(features.has_categories ? "yes" : "no"));
    spacetimedb::log("  - Tags: " + std::string(features.has_tags ? "yes" : "no"));
}

// Schema evolution helper
SPACETIMEDB_REDUCER(get_schema_version_info, spacetimedb::ReducerContext ctx) {
    spacetimedb::log("Schema Version History:");
    spacetimedb::log("1.0.0 - Initial release with products table");
    spacetimedb::log("1.1.0 - Added categories table (backward compatible)");
    spacetimedb::log("1.2.0 - Added product tags table (backward compatible)");
    spacetimedb::log("");
    spacetimedb::log("Current version: " + MODULE_VERSION.to_string());
    
    // Show what changes are backward compatible
    spacetimedb::log("");
    spacetimedb::log("Backward compatible changes:");
    spacetimedb::log("- Adding new tables");
    spacetimedb::log("- Adding new reducers");
    spacetimedb::log("- Adding optional fields to existing tables");
    spacetimedb::log("- Adding new indexes");
    
    spacetimedb::log("");
    spacetimedb::log("Breaking changes (require major version bump):");
    spacetimedb::log("- Removing tables or columns");
    spacetimedb::log("- Changing column types");
    spacetimedb::log("- Removing or changing reducer signatures");
    spacetimedb::log("- Making optional fields required");
}

// Module initialization with version check
SPACETIMEDB_REDUCER(__init__, spacetimedb::ReducerContext ctx) {
    spacetimedb::log("Initializing CompatibleEvolution module " + MODULE_VERSION.to_string());
    
    // In a real implementation, check if this is an upgrade
    // and perform any necessary data migrations
    
    // Log feature availability
    auto features = FeatureFlags::for_version(MODULE_VERSION);
    spacetimedb::log("Module features enabled:");
    spacetimedb::log("  - Categories: " + std::string(features.has_categories ? "yes" : "no"));
    spacetimedb::log("  - Tags: " + std::string(features.has_tags ? "yes" : "no"));
    spacetimedb::log("  - Bulk import: " + std::string(features.has_bulk_import ? "yes" : "no"));
}