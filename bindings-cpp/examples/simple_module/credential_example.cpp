#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/credentials.h>
#include <iostream>
#include <sstream>

using namespace SpacetimeDb;

// Table to store user credentials and permissions
struct UserCredential {
    Identity identity;
    std::string username;
    std::string role;  // "admin", "user", "guest"
    uint64_t created_at;
    std::optional<uint64_t> revoked_at;
};

// Register fields for UserCredential
SPACETIMEDB_REGISTER_FIELDS(UserCredential,
    SPACETIMEDB_FIELD(UserCredential, identity, Identity);
    SPACETIMEDB_FIELD(UserCredential, username, std::string);
    SPACETIMEDB_FIELD(UserCredential, role, std::string);
    SPACETIMEDB_FIELD(UserCredential, created_at, uint64_t);
    SPACETIMEDB_FIELD(UserCredential, revoked_at, std::optional<uint64_t>);
)

// Table to track actions/permissions
struct PermissionLog {
    uint64_t id;
    Identity actor;
    std::string action;
    bool allowed;
    uint64_t timestamp;
    std::string reason;
};

SPACETIMEDB_REGISTER_FIELDS(PermissionLog,
    SPACETIMEDB_FIELD(PermissionLog, id, uint64_t);
    SPACETIMEDB_FIELD(PermissionLog, actor, Identity);
    SPACETIMEDB_FIELD(PermissionLog, action, std::string);
    SPACETIMEDB_FIELD(PermissionLog, allowed, bool);
    SPACETIMEDB_FIELD(PermissionLog, timestamp, uint64_t);
    SPACETIMEDB_FIELD(PermissionLog, reason, std::string);
)

// Register tables
SPACETIMEDB_TABLE(UserCredential, user_credentials, true)
SPACETIMEDB_TABLE(PermissionLog, permission_logs, true)

// Helper function to get current timestamp (mock)
uint64_t get_current_timestamp() {
    // In a real module, this would come from the reducer context
    return 1234567890;
}

// Helper function to check if a user has a specific role
bool has_role(SpacetimeDb::ReducerContext& ctx, const Identity& identity, const std::string& required_role) {
    auto credentials = ctx.db.table<UserCredential>("user_credentials");
    
    // Look for the user's credential
    for (const auto& cred : credentials.iter()) {
        if (cred.identity == identity && !cred.revoked_at.has_value()) {
            return cred.role == required_role || cred.role == "admin";
        }
    }
    
    return false;
}

// Log a permission check
void log_permission_check(SpacetimeDb::ReducerContext& ctx, const Identity& actor, 
                         const std::string& action, bool allowed, const std::string& reason) {
    static uint64_t next_id = 1;
    
    PermissionLog log{
        next_id++,
        actor,
        action,
        allowed,
        get_current_timestamp(),
        reason
    };
    
    ctx.db.table<PermissionLog>("permission_logs").insert(log);
}

// Initialize the module with a default admin
SPACETIMEDB_REDUCER(init, SpacetimeDb::ReducerContext ctx) {
    // Get module identity to make it the initial admin
    Identity module_id = get_module_identity();
    
    UserCredential admin{
        module_id,
        "module_admin",
        "admin",
        get_current_timestamp(),
        std::nullopt
    };
    
    ctx.db.table<UserCredential>("user_credentials").insert(admin);
    
    // Log the initialization
    std::cout << "Module initialized with admin identity: " << identity_extensions::to_abbreviated_hex(module_id) << std::endl;
}

// Create a new user credential (admin only)
SPACETIMEDB_REDUCER(create_user_credential, SpacetimeDb::ReducerContext ctx, 
                   std::string username, std::string role) {
    // Check if caller is admin
    if (!has_role(ctx, ctx.sender, "admin")) {
        log_permission_check(ctx, ctx.sender, "create_user_credential", false, "Not an admin");
        throw std::runtime_error("Only admins can create user credentials");
    }
    
    // Create identity from username and a fixed issuer
    Identity new_identity = Credentials::create_identity("spacetimedb", username);
    
    UserCredential cred{
        new_identity,
        username,
        role,
        get_current_timestamp(),
        std::nullopt
    };
    
    ctx.db.table<UserCredential>("user_credentials").insert(cred);
    
    log_permission_check(ctx, ctx.sender, "create_user_credential", true, "Admin privilege");
    
    std::cout << "Created credential for " << username << " with identity: " 
              << identity_extensions::to_abbreviated_hex(new_identity) << std::endl;
}

// Revoke a user credential (admin only)
SPACETIMEDB_REDUCER(revoke_credential, SpacetimeDb::ReducerContext ctx, std::string username) {
    // Check if caller is admin
    if (!has_role(ctx, ctx.sender, "admin")) {
        log_permission_check(ctx, ctx.sender, "revoke_credential", false, "Not an admin");
        throw std::runtime_error("Only admins can revoke credentials");
    }
    
    auto credentials = ctx.db.table<UserCredential>("user_credentials");
    bool found = false;
    
    for (auto& cred : credentials.iter()) {
        if (cred.username == username && !cred.revoked_at.has_value()) {
            cred.revoked_at = get_current_timestamp();
            credentials.update(cred);
            found = true;
            
            std::cout << "Revoked credential for " << username << std::endl;
            break;
        }
    }
    
    if (!found) {
        throw std::runtime_error("User credential not found or already revoked");
    }
    
    log_permission_check(ctx, ctx.sender, "revoke_credential", true, "Admin privilege");
}

// Perform an action that requires specific role
SPACETIMEDB_REDUCER(perform_privileged_action, SpacetimeDb::ReducerContext ctx, 
                   std::string action, std::string required_role) {
    if (has_role(ctx, ctx.sender, required_role)) {
        log_permission_check(ctx, ctx.sender, action, true, "Has required role: " + required_role);
        std::cout << "Action '" << action << "' performed by " << identity_extensions::to_abbreviated_hex(ctx.sender) << std::endl;
    } else {
        log_permission_check(ctx, ctx.sender, action, false, "Missing required role: " + required_role);
        throw std::runtime_error("Insufficient privileges for action: " + action);
    }
}

// Check current user's permissions
SPACETIMEDB_REDUCER(check_my_permissions, SpacetimeDb::ReducerContext ctx) {
    auto credentials = ctx.db.table<UserCredential>("user_credentials");
    
    bool found = false;
    for (const auto& cred : credentials.iter()) {
        if (cred.identity == ctx.sender && !cred.revoked_at.has_value()) {
            std::cout << "Your credentials:" << std::endl;
            std::cout << "  Username: " << cred.username << std::endl;
            std::cout << "  Role: " << cred.role << std::endl;
            std::cout << "  Identity: " << identity_extensions::to_hex(cred.identity) << std::endl;
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "No active credentials found for your identity: " 
                  << identity_extensions::to_hex(ctx.sender) << std::endl;
    }
}

// View permission logs (admin only)
SPACETIMEDB_REDUCER(view_permission_logs, SpacetimeDb::ReducerContext ctx, uint32_t limit) {
    if (!has_role(ctx, ctx.sender, "admin")) {
        log_permission_check(ctx, ctx.sender, "view_permission_logs", false, "Not an admin");
        throw std::runtime_error("Only admins can view permission logs");
    }
    
    auto logs = ctx.db.table<PermissionLog>("permission_logs");
    uint32_t count = 0;
    
    std::cout << "Recent permission checks:" << std::endl;
    for (const auto& log : logs.iter()) {
        if (count >= limit) break;
        
        std::cout << "  [" << log.timestamp << "] " 
                  << identity_extensions::to_abbreviated_hex(log.actor) << " - "
                  << log.action << ": " 
                  << (log.allowed ? "ALLOWED" : "DENIED")
                  << " (" << log.reason << ")" << std::endl;
        count++;
    }
    
    log_permission_check(ctx, ctx.sender, "view_permission_logs", true, "Admin privilege");
}

// Module exports
extern "C" {

void __describe_module__(uint8_t* data, size_t* len) {
    // Module description implementation
    // This would generate the proper module definition
}

int16_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t address_0, uint64_t address_1,
    uint64_t timestamp,
    const uint8_t* args, size_t args_len
) {
    // Reducer dispatch implementation
    return 0;
}

} // extern "C"