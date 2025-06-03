/**
 * SpacetimeDB C++ SDK Example: Integrated Table Declaration
 * 
 * This example demonstrates the cleanest way to declare tables
 * in SpacetimeDB modules - all in one place!
 */

// Step 1: Define ALL your tables using X-Macro pattern
// Format: X(TypeName, table_name, is_public)
// This single declaration handles everything:
//   - Forward declaration of types
//   - Table registration with SpacetimeDB
//   - Accessor method generation (ctx.db.table_name())
#define SPACETIMEDB_TABLES_LIST \
    X(User, users, true) \
    X(Message, messages, true) \
    X(Config, config, false)

// Step 2: Include the header
#include <spacetimedb/spacetimedb_easy.h>
#include <string>

// Step 3: Define your table types
struct User {
    uint32_t id;
    std::string name;
    std::string email;
};

struct Message {
    uint32_t id;
    uint32_t user_id;
    std::string content;
    uint64_t timestamp;
};

struct Config {
    std::string key;
    std::string value;
};

// That's it! No separate SPACETIMEDB_TABLE declarations needed!

// Step 5: Use the automatically generated accessors in reducers
SPACETIMEDB_REDUCER(create_user, spacetimedb::ReducerContext ctx, 
                    uint32_t id, const std::string& name, const std::string& email) {
    User user{id, name, email};
    // ctx.db.users() is automatically generated!
    ctx.db.users().insert(user);
}

SPACETIMEDB_REDUCER(post_message, spacetimedb::ReducerContext ctx,
                    uint32_t id, uint32_t user_id, const std::string& content) {
    Message msg{id, user_id, content, 0}; // timestamp would be set by system
    // ctx.db.messages() is automatically generated!
    ctx.db.messages().insert(msg);
}

SPACETIMEDB_REDUCER(set_config, spacetimedb::ReducerContext ctx,
                    const std::string& key, const std::string& value) {
    Config cfg{key, value};
    // ctx.db.config() is automatically generated!
    ctx.db.config().insert(cfg);
}

// Alternative access patterns are still available:
SPACETIMEDB_REDUCER(example_generic_access, spacetimedb::ReducerContext ctx) {
    // Generic access using table<T>(name)
    User user{999, "generic", "user@example.com"};
    ctx.db.table<User>("users").insert(user);
    
    // Runtime table check
    if (ctx.db.has_table("users")) {
        spacetimedb::log("Users table exists!");
    }
    
    // Get table count
    spacetimedb::log("Total tables: " + std::to_string(ctx.db.get_table_count()));
}