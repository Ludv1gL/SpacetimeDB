// SpacetimeDB C++ Module using Module Library Macros
#include <spacetimedb/spacetimedb.h>

// Define a simple User table using the macro
SPACETIMEDB_TABLE(User, public)
struct User {
    SPACETIMEDB_DATA_MEMBER(id)
    uint32_t id;
    
    SPACETIMEDB_DATA_MEMBER(name)
    std::string name;
    
    SPACETIMEDB_PRIMARY_KEY(id)
};

// Define a reducer to add users
SPACETIMEDB_REDUCER(add_user, UserDefined, ctx, uint32_t id, const std::string& name) {
    // Insert a new user
    User user{id, name};
    ctx.insert_row("User", user);
    
    // Log the action
    LOG_INFO("Added user: " << id << " - " << name);
}

// Define a reducer to list all users
SPACETIMEDB_REDUCER(list_users, UserDefined, ctx) {
    LOG_INFO("Listing all users...");
    
    // Query all users
    auto users = ctx.query<User>("SELECT * FROM User");
    for (const auto& user : users) {
        LOG_INFO("User: " << user.id << " - " << user.name);
    }
}