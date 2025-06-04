// Test module to validate the new consolidated macro system
// This tests all the new features including scheduled reducers, constraints, and indexes

#include "spacetimedb/spacetimedb.h"

using namespace SpacetimeDb;

// ========== DATA STRUCTURES ==========

// User table with primary key and index
struct User {
    SPACETIMEDB_PRIMARY_KEY_AUTO
    uint32_t id;
    
    SPACETIMEDB_UNIQUE
    std::string username;
    
    SPACETIMEDB_INDEX
    std::string email;
    
    uint64_t created_at;
};
SPACETIMEDB_TYPE(User)

// Post table with foreign key to User
struct Post {
    SPACETIMEDB_PRIMARY_KEY_AUTO
    uint32_t id;
    
    uint32_t user_id;  // Foreign key to User.id
    std::string title;
    std::string content;
    
    SPACETIMEDB_INDEX
    uint64_t created_at;
};
SPACETIMEDB_TYPE(Post)

// Session table for cleanup example
struct Session {
    SPACETIMEDB_PRIMARY_KEY
    std::string session_id;
    
    uint32_t user_id;
    uint64_t last_active;
    uint64_t expires_at;
};
SPACETIMEDB_TYPE(Session)

// Product table with check constraint
struct Product {
    SPACETIMEDB_PRIMARY_KEY_AUTO
    uint32_t id;
    
    std::string name;
    double price;
    uint32_t stock_quantity;
};
SPACETIMEDB_TYPE(Product)

// Scheduled task tracking table
struct ScheduledTask {
    SPACETIMEDB_PRIMARY_KEY_AUTO
    uint32_t id;
    
    std::string task_name;
    uint64_t last_run;
    uint64_t next_run;
    std::string status;
};
SPACETIMEDB_TYPE(ScheduledTask)

// ========== TABLE REGISTRATIONS ==========

// Basic public tables
SPACETIMEDB_TABLE(User, "users", true)
SPACETIMEDB_TABLE(Post, "posts", true)
SPACETIMEDB_TABLE(Session, "sessions", false)  // Private table
SPACETIMEDB_TABLE(Product, "products", true)

// Scheduled table - cleaned up by scheduled reducer
SPACETIMEDB_TABLE(ScheduledTask, "scheduled_tasks", true, "task_scheduler", "next_run")

// ========== INDEXES ==========

// Multi-column index on posts
SPACETIMEDB_INDEX_BTREE_MULTI(Post, post_user_created_idx, user_id, created_at)

// Unique index on sessions
SPACETIMEDB_INDEX_UNIQUE(Session, user_id)

// Hash index on user email (for fast lookups)
SPACETIMEDB_INDEX_HASH(User, email)

// ========== CONSTRAINTS ==========

// Foreign key constraints
SPACETIMEDB_FOREIGN_KEY(Post, user_id, User, id)
SPACETIMEDB_FOREIGN_KEY(Session, user_id, User, id)

// Check constraints
SPACETIMEDB_CHECK_CONSTRAINT(Product, "price > 0")
SPACETIMEDB_CHECK_CONSTRAINT(Product, "stock_quantity >= 0")

// ========== LIFECYCLE REDUCERS ==========

// Module initialization
SPACETIMEDB_INIT(on_module_init, ctx) {
    SpacetimeDb::Log::info("Module initialized");
    
    // Create initial admin user
    auto users = get_User_table();
    User admin{0, "admin", "admin@example.com", ctx.timestamp.ToSeconds()};
    users.insert(admin);
}

// Client connection handler (required version)
SPACETIMEDB_CLIENT_CONNECTED_REQUIRED(on_connect, ctx) {
    auto identity = ctx.identity;
    SpacetimeDb::Log::info("Client connected:", identity.ToString());
    
    // Create session for connected user
    auto sessions = get_Session_table();
    Session session{
        identity.ToString(),  // Use identity as session ID
        0,  // Will be set when user logs in
        ctx.timestamp.ToSeconds(),
        ctx.timestamp.ToSeconds() + 3600  // 1 hour expiry
    };
    sessions.insert(session);
}

// Client disconnection handler
SPACETIMEDB_CLIENT_DISCONNECTED(on_disconnect, ctx) {
    auto identity = ctx.identity;
    SpacetimeDb::Log::info("Client disconnected:", identity.ToString());
    
    // Remove session
    auto sessions = get_Session_table();
    // Note: In real implementation, would use delete_where with predicate
}

// ========== SCHEDULED REDUCERS ==========

// Clean up expired sessions every minute
SPACETIMEDB_SCHEDULED_EVERY_MINUTE(cleanup_expired_sessions, ctx) {
    auto sessions = get_Session_table();
    auto current_time = ctx.timestamp.ToSeconds();
    
    uint32_t cleaned = 0;
    for (auto& session : sessions.iter()) {
        if (session.expires_at < current_time) {
            sessions.delete_row(session);
            cleaned++;
        }
    }
    
    if (cleaned > 0) {
        SpacetimeDb::Log::info("Cleaned up", cleaned, "expired sessions");
    }
}

// Run maintenance tasks every hour
SPACETIMEDB_SCHEDULED_EVERY_HOUR(hourly_maintenance, ctx) {
    SpacetimeDb::Log::info("Running hourly maintenance");
    
    // Update scheduled task status
    auto tasks = get_ScheduledTask_table();
    for (auto& task : tasks.iter()) {
        if (task.task_name == "hourly_maintenance") {
            task.last_run = ctx.timestamp.ToSeconds();
            task.next_run = ctx.timestamp.ToSeconds() + 3600;
            task.status = "completed";
            // Note: In real implementation, would use update operation
        }
    }
}

// Custom scheduled reducer (runs every 30 seconds)
SPACETIMEDB_SCHEDULED(quick_check, Duration::from_seconds(30), ctx) {
    static uint32_t run_count = 0;
    run_count++;
    SpacetimeDb::Log::debug("Quick check run #", run_count);
}

// ========== BUSINESS LOGIC REDUCERS ==========

// Create a new user
SPACETIMEDB_REDUCER(create_user, UserDefined, ctx, std::string username, std::string email) {
    auto users = get_User_table();
    
    // Check if username already exists (unique constraint will also enforce this)
    for (auto& user : users.iter()) {
        if (user.username == username) {
            SpacetimeDb::Log::error("Username already exists:", username);
            return;
        }
    }
    
    User new_user{0, username, email, ctx.timestamp.ToSeconds()};
    auto inserted = users.insert(new_user);
    SpacetimeDb::Log::info("Created user:", inserted.username, "with ID:", inserted.id);
}

// Create a new post
SPACETIMEDB_REDUCER(create_post, UserDefined, ctx, uint32_t user_id, std::string title, std::string content) {
    // Foreign key constraint will validate user_id exists
    auto posts = get_Post_table();
    
    Post new_post{0, user_id, title, content, ctx.timestamp.ToSeconds()};
    auto inserted = posts.insert(new_post);
    SpacetimeDb::Log::info("Created post:", inserted.title, "by user:", user_id);
}

// Add a product with validation
SPACETIMEDB_REDUCER(add_product, UserDefined, ctx, std::string name, double price, uint32_t quantity) {
    if (price <= 0) {
        SpacetimeDb::Log::error("Invalid price:", price);
        return;
    }
    
    auto products = get_Product_table();
    Product new_product{0, name, price, quantity};
    
    // Check constraint will validate price > 0 and quantity >= 0
    auto inserted = products.insert(new_product);
    SpacetimeDb::Log::info("Added product:", inserted.name, "at $", inserted.price);
}

// ========== CLIENT VISIBILITY FILTER ==========

// Only show user's own sessions
SPACETIMEDB_CLIENT_VISIBILITY_FILTER(
    user_sessions_only,
    "SELECT * FROM sessions WHERE user_id = current_user_id()"
)

// ========== MODULE METADATA ==========

SPACETIMEDB_MODULE_VERSION(1, 0, 0)
SPACETIMEDB_MODULE_METADATA(
    "test_new_macros",
    "SpacetimeDB Team",
    "Test module for new macro system with all features",
    "MIT"
)

// Note: RLS policies would be added here when implemented
// SPACETIMEDB_RLS_SELECT(Session, "user_sessions", "user_id = current_user_id()")
// SPACETIMEDB_RLS_INSERT(Post, "user_posts", "user_id = current_user_id()")