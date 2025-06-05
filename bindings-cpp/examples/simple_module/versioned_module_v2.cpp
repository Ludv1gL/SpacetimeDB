/**
 * Example: Versioned Module v2.0.0
 * 
 * This example demonstrates upgrading from v1.0.0 to v2.0.0
 * Major version change with schema migrations
 */

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/versioning.h>
#include <spacetimedb/migration.h>

// Define module version
SPACETIMEDB_MODULE_VERSION(2, 0, 0)

// Define module metadata
SPACETIMEDB_MODULE_METADATA(
    "UserManagement",
    "Example Author",
    "User management module v2 with profile support",
    "MIT"
)

// Version 2.0.0 Schema - Enhanced with profile information
struct User {
    uint64_t id;
    std::string username;
    std::string email;
    std::string display_name;  // NEW in v2
    uint64_t created_at;
    uint64_t updated_at;       // NEW in v2
};

struct UserProfile {  // NEW table in v2
    uint64_t user_id;
    std::optional<std::string> bio;
    std::optional<std::string> avatar_url;
    std::optional<std::string> location;
    std::map<std::string, std::string> social_links;
};

// Register fields
SPACETIMEDB_REGISTER_FIELDS(User,
    SPACETIMEDB_FIELD(User, id, uint64_t);
    SPACETIMEDB_FIELD(User, username, std::string);
    SPACETIMEDB_FIELD(User, email, std::string);
    SPACETIMEDB_FIELD(User, display_name, std::string);
    SPACETIMEDB_FIELD(User, created_at, uint64_t);
    SPACETIMEDB_FIELD(User, updated_at, uint64_t);
)

SPACETIMEDB_REGISTER_FIELDS(UserProfile,
    SPACETIMEDB_FIELD(UserProfile, user_id, uint64_t);
    SPACETIMEDB_FIELD(UserProfile, bio, std::optional<std::string>);
    SPACETIMEDB_FIELD(UserProfile, avatar_url, std::optional<std::string>);
    SPACETIMEDB_FIELD(UserProfile, location, std::optional<std::string>);
    SPACETIMEDB_FIELD(UserProfile, social_links, std::map<std::string, std::string>);
)

// Tables
SPACETIMEDB_TABLE(User, users, true)
SPACETIMEDB_TABLE(UserProfile, user_profiles, true)

// Define migration from v1 to v2
DEFINE_MIGRATION(MigrateV1ToV2, 1, 0, 0, 2, 0, 0)

void MigrateV1ToV2::up(SpacetimeDb::MigrationContext& ctx) {
    ctx.log_info("Starting migration from v1.0.0 to v2.0.0");
    
    // Step 1: Add new columns to users table
    // In practice, this would be done through schema evolution
    // Here we show the conceptual approach
    
    // Step 2: Migrate existing user data
    // Set display_name to username for existing users
    // Set updated_at to created_at for existing users
    
    // Step 3: Create user_profiles table (handled by schema)
    
    // Step 4: Create default profiles for existing users
    auto users = ctx.db().table<User>("users");
    // Iterate through users and create profiles
    // (Implementation would depend on actual table iteration API)
    
    ctx.log_info("Migration completed successfully");
}

void MigrateV1ToV2::down(SpacetimeDb::MigrationContext& ctx) {
    ctx.log_info("Rolling back migration from v2.0.0 to v1.0.0");
    
    // Remove user_profiles table
    // Remove new columns from users table
    
    ctx.log_warning("Rollback will lose profile data!");
}

std::string MigrateV1ToV2::description() const {
    return "Add user profiles and enhanced user fields";
}

// Register the migration
REGISTER_MIGRATION(MigrateV1ToV2)

// Module state
struct ModuleState {
    static SpacetimeDb::ModuleVersionManager version_manager;
};

SpacetimeDb::ModuleVersionManager ModuleState::version_manager(MODULE_METADATA);

// Enhanced reducers for v2
SPACETIMEDB_REDUCER(create_user, SpacetimeDb::ReducerContext ctx, 
                   std::string username, std::string email, std::string display_name) {
    static uint64_t next_id = 1;
    uint64_t now = static_cast<uint64_t>(std::time(nullptr));
    
    User user{
        .id = next_id++,
        .username = username,
        .email = email,
        .display_name = display_name,
        .created_at = now,
        .updated_at = now
    };
    
    ctx.db.table<User>("users").insert(user);
    
    // Create default profile
    UserProfile profile{
        .user_id = user.id,
        .bio = std::nullopt,
        .avatar_url = std::nullopt,
        .location = std::nullopt,
        .social_links = {}
    };
    
    ctx.db.table<UserProfile>("user_profiles").insert(profile);
    
    SpacetimeDb::log("Created user with profile: " + username);
}

SPACETIMEDB_REDUCER(update_profile, SpacetimeDb::ReducerContext ctx,
                   uint64_t user_id, 
                   std::optional<std::string> bio,
                   std::optional<std::string> avatar_url,
                   std::optional<std::string> location) {
    auto profiles = ctx.db.table<UserProfile>("user_profiles");
    
    // Update profile (would need actual update API)
    SpacetimeDb::log("Updating profile for user: " + std::to_string(user_id));
    
    // Also update user's updated_at timestamp
    auto users = ctx.db.table<User>("users");
    // Update user timestamp
}

SPACETIMEDB_REDUCER(add_social_link, SpacetimeDb::ReducerContext ctx,
                   uint64_t user_id, std::string platform, std::string url) {
    auto profiles = ctx.db.table<UserProfile>("user_profiles");
    
    // Add social link to profile
    SpacetimeDb::log("Adding social link for user: " + std::to_string(user_id));
}

// Version management reducers
SPACETIMEDB_REDUCER(get_module_info, SpacetimeDb::ReducerContext ctx) {
    auto& metadata = ModuleState::version_manager.metadata();
    
    SpacetimeDb::log("Module: " + metadata.name);
    SpacetimeDb::log("Version: " + metadata.version.to_string());
    SpacetimeDb::log("Author: " + metadata.author);
    SpacetimeDb::log("Description: " + metadata.description);
}

// Migration execution reducer
SPACETIMEDB_REDUCER(__migrate__, SpacetimeDb::ReducerContext ctx,
                   std::string from_version_str, std::string to_version_str) {
    auto from_version = SpacetimeDb::ModuleVersion::parse(from_version_str);
    auto to_version = SpacetimeDb::ModuleVersion::parse(to_version_str);
    
    SpacetimeDb::log("Migrating from " + from_version_str + " to " + to_version_str);
    
    // Get migration path
    auto& registry = SpacetimeDb::MigrationRegistry::instance();
    auto migrations = registry.find_migration_path(from_version, to_version);
    
    if (!migrations.has_value()) {
        SpacetimeDb::log("No migration path found!");
        return;
    }
    
    // Execute migrations
    SpacetimeDb::MigrationContext migration_ctx(&ctx, from_version, to_version);
    
    for (auto* migration : migrations.value()) {
        SpacetimeDb::log("Executing: " + migration->description());
        migration->up(migration_ctx);
    }
    
    SpacetimeDb::log("Migration completed successfully");
}

// Module initialization
SPACETIMEDB_REDUCER(__init__, SpacetimeDb::ReducerContext ctx) {
    SpacetimeDb::log("Initializing UserManagement module v2.0.0");
    
    // Check if this is an upgrade
    // In practice, would check stored version in database
    
    ModuleState::version_manager.register_version(
        MODULE_VERSION, MODULE_METADATA
    );
}