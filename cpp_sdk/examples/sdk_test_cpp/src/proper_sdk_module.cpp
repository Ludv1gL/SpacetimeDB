#include <spacetimedb/macros.h>
#include <cstdint>
#include <string>

// Proper C++ SDK module using the official macros and headers
namespace my_module {

// Define a simple user struct
struct User {
    uint32_t id;
    std::string name;
    uint32_t age;
};

// Register the User struct with SpacetimeDB
#define USER_FIELDS(ACTION, WRITER_OR_READER, VALUE_OR_OBJ) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, id, false, false) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, std::string, name, false, false) \
    ACTION(WRITER_OR_READER, VALUE_OR_OBJ, uint32_t, age, false, false)

SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(
    my_module::User, my_module_User,
    "User",
    USER_FIELDS,
    ({
        SPACETIMEDB_FIELD("id", ::SpacetimeDb::CoreType::U32, false, false),
        SPACETIMEDB_FIELD("name", ::SpacetimeDb::CoreType::String, false, false),
        SPACETIMEDB_FIELD("age", ::SpacetimeDb::CoreType::U32, false, false)
    })
);

// Register a table for users
SPACETIMEDB_TABLE(my_module::User, users_table, "UsersTable");

// Simple add user reducer
SPACETIMEDB_REDUCER(add_user, (std::string name, uint32_t age)) {
    // Log the call
    SPACETIMEDB_LOG_INFO("Adding user: {} age {}", name, age);
    
    // Create a new user
    User new_user;
    new_user.id = 1; // Simple ID for demo
    new_user.name = name;
    new_user.age = age;
    
    // Insert into table
    users_table.insert(new_user);
    
    SPACETIMEDB_LOG_INFO("User added successfully");
}

// Get user count reducer
SPACETIMEDB_REDUCER(get_user_count, ()) {
    uint32_t count = 0;
    
    // Count users in table
    for (const auto& user : users_table.scan()) {
        count++;
    }
    
    SPACETIMEDB_LOG_INFO("Total users: {}", count);
}

} // namespace my_module