#include <spacetimedb/macros.h>
#include <spacetimedb/sdk/spacetimedb_sdk_reducer.h>
#include <spacetimedb/sdk/database.h>
#include <spacetimedb/sdk/table.h>
#include <spacetimedb/abi/spacetimedb_abi.h>
#include <cstdint>
#include <string>

// Add using declarations for SpacetimeDB SDK types
using spacetimedb::sdk::ReducerContext;
using spacetimedb::sdk::Database;
using spacetimedb::sdk::Table;

namespace simple_module {

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
    simple_module::User, simple_module_User,
    "User",
    USER_FIELDS,
    ({
        SPACETIMEDB_FIELD("id", ::SpacetimeDb::CoreType::U32, false, true),
        SPACETIMEDB_FIELD("name", ::SpacetimeDb::CoreType::String, false, false),
        SPACETIMEDB_FIELD("age", ::SpacetimeDb::CoreType::U32, false, false)
    })
);

// Register table
SPACETIMEDB_TABLE(simple_module::User, "UsersTable", true, nullptr);
SPACETIMEDB_PRIMARY_KEY("UsersTable", "id");

// Reducer functions
void add_user(spacetimedb::sdk::ReducerContext& ctx, const std::string& name, uint32_t age) {
    auto users_table = ctx.db().get_table<User>("UsersTable");
    
    User new_user;
    new_user.id = 0; // Will be auto-generated
    new_user.name = name;
    new_user.age = age;
    
    users_table.insert(new_user);
}

void get_user_count(spacetimedb::sdk::ReducerContext& ctx) {
    auto users_table = ctx.db().get_table<User>("UsersTable");
    
    uint32_t count = 0;
    for (const auto& user : users_table.scan()) {
        count++;
    }
    
    // Log count using raw ABI
    std::string message = "Total users: " + std::to_string(count);
    _console_log(0, nullptr, 0, nullptr, 0, 0, 
                 reinterpret_cast<const uint8_t*>(message.c_str()), 
                 message.length());
}

// Register reducers with SDK
SPACETIMEDB_REDUCER(simple_module::add_user, const std::string&, uint32_t);
SPACETIMEDB_REDUCER(simple_module::get_user_count);

// Register reducer schemas
SPACETIMEDB_REDUCER_NAMED("add_user", simple_module::add_user,
    { 
        SPACETIMEDB_REDUCER_PARAM("name", SpacetimeDb::CoreType::String),
        SPACETIMEDB_REDUCER_PARAM("age", SpacetimeDb::CoreType::U32) 
    },
    std::string, uint32_t
);

SPACETIMEDB_REDUCER_NAMED("get_user_count", simple_module::get_user_count,
    { },
    // No parameters
);

} // namespace simple_module