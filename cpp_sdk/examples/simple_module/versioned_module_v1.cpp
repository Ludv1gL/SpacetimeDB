/**
 * Example: Versioned Module v1.0.0
 * 
 * This example demonstrates a module with version 1.0.0
 * Initial version with basic user management
 */

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/versioning.h>

// Define module version
SPACETIMEDB_MODULE_VERSION(1, 0, 0)

// Define module metadata
SPACETIMEDB_MODULE_METADATA(
    "UserManagement",
    "Example Author",
    "User management module with versioning support",
    "MIT"
)

// Version 1.0.0 Schema
struct User {
    uint64_t id;
    std::string username;
    std::string email;
    uint64_t created_at;
};

SPACETIMEDB_REGISTER_FIELDS(User,
    SPACETIMEDB_FIELD(User, id, uint64_t);
    SPACETIMEDB_FIELD(User, username, std::string);
    SPACETIMEDB_FIELD(User, email, std::string);
    SPACETIMEDB_FIELD(User, created_at, uint64_t);
)

// Tables
SPACETIMEDB_TABLE(User, users, true)

// Module state for version tracking
struct ModuleState {
    static spacetimedb::ModuleVersionManager version_manager;
};

spacetimedb::ModuleVersionManager ModuleState::version_manager(MODULE_METADATA);

// Reducers
SPACETIMEDB_REDUCER(create_user, spacetimedb::ReducerContext ctx, 
                   std::string username, std::string email) {
    // Auto-increment ID (in real implementation, use sequences)
    static uint64_t next_id = 1;
    
    User user{
        .id = next_id++,
        .username = username,
        .email = email,
        .created_at = static_cast<uint64_t>(std::time(nullptr))
    };
    
    ctx.db.table<User>("users").insert(user);
    
    spacetimedb::log("Created user: " + username);
}

SPACETIMEDB_REDUCER(get_user_by_username, spacetimedb::ReducerContext ctx,
                   std::string username) {
    auto users_table = ctx.db.table<User>("users");
    
    // In a real implementation, you'd use proper query methods
    // For now, this is a placeholder showing the concept
    spacetimedb::log("Looking up user: " + username);
}

SPACETIMEDB_REDUCER(list_users, spacetimedb::ReducerContext ctx) {
    auto users_table = ctx.db.table<User>("users");
    
    spacetimedb::log("Listing all users");
    // In real implementation, iterate through users
}

// Version management reducer
SPACETIMEDB_REDUCER(get_module_info, spacetimedb::ReducerContext ctx) {
    auto& metadata = ModuleState::version_manager.metadata();
    
    spacetimedb::log("Module: " + metadata.name);
    spacetimedb::log("Version: " + metadata.version.to_string());
    spacetimedb::log("Author: " + metadata.author);
    spacetimedb::log("Description: " + metadata.description);
}

// Module initialization
SPACETIMEDB_REDUCER(__init__, spacetimedb::ReducerContext ctx) {
    spacetimedb::log("Initializing UserManagement module v1.0.0");
    
    // Register this version
    ModuleState::version_manager.register_version(
        MODULE_VERSION, MODULE_METADATA
    );
}

// Enhanced module description with version info
extern "C" {
    __attribute__((export_name("__describe_module_with_version__")))
    void __describe_module_with_version__(uint32_t sink) {
        // This would integrate version metadata into the module description
        // For now, we use the standard __describe_module__ approach
        // In a full implementation, this would add version info to the module def
    }
}