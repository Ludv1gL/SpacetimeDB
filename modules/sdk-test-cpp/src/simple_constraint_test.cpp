/**
 * Simple constraint test without complex validation framework
 */

#define SPACETIMEDB_TABLES_LIST \
    X(User, users, true)

#include "spacetimedb/spacetimedb.h"

using namespace SpacetimeDb;

// Simple User table
struct User {
    uint32_t id;
    std::string email;
    std::string name;
    uint32_t age;
};

// Test that the constraint macros compile correctly
SPACETIMEDB_PRIMARY_KEY(User, id);
SPACETIMEDB_UNIQUE(User, email);

// Test insert reducer
SPACETIMEDB_REDUCER(create_user, ReducerContext ctx, uint32_t id, std::string email, std::string name, uint32_t age) {
    User user{
        .id = id,
        .email = email,
        .name = name,
        .age = age
    };
    
    // Note: Actual constraint enforcement happens in SpacetimeDB core via unique indexes
    // This is just testing that the constraint registration macros compile
    ctx.db->users().insert(user);
    LOG_INFO("Created user with id: " + std::to_string(id) + ", email: " + email);
}

SPACETIMEDB_REDUCER(init_simple_constraint_test, ReducerContext ctx) {
    LOG_INFO("Simple constraint test module initialized");
    
    // Insert test data
    User test_user{.id = 1, .email = "test@example.com", .name = "Test User", .age = 25};
    ctx.db->users().insert(test_user);
    LOG_INFO("Test data inserted successfully");
}