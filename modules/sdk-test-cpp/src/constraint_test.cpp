/**
 * Test module for primary key and unique constraint support
 */

#define SPACETIMEDB_TABLES_LIST \
    X(User, users, true) \
    X(Product, products, true)

#include "spacetimedb/spacetimedb.h"

using namespace SpacetimeDb;

// User table with primary key constraint
struct User {
    uint32_t id;        // Will be primary key
    std::string email;  // Will be unique
    std::string name;
    uint32_t age;
};

// Product table with constraints
struct Product {
    uint32_t id;          // Primary key
    std::string sku;      // Unique constraint
    std::string name;
    double price;
};

// Register primary keys and unique constraints
SPACETIMEDB_PRIMARY_KEY(User, id);
SPACETIMEDB_UNIQUE(User, email);

SPACETIMEDB_PRIMARY_KEY(Product, id);
SPACETIMEDB_UNIQUE(Product, sku);

// Test insert with constraint checking
SPACETIMEDB_REDUCER(create_user, ReducerContext ctx, uint32_t id, std::string email, std::string name, uint32_t age) {
    User user{
        .id = id,
        .email = email,
        .name = name,
        .age = age
    };
    
    try {
        ctx.db->users().insert(user);
        LOG_INFO("Successfully created user with id: " + std::to_string(id) + ", email: " + email);
    } catch (const SpacetimeDB::UniqueConstraintViolationException& e) {
        LOG_INFO("Constraint violation: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(create_product, ReducerContext ctx, uint32_t id, std::string sku, std::string name, double price) {
    Product product{
        .id = id,
        .sku = sku,
        .name = name,
        .price = price
    };
    
    try {
        ctx.db->products().insert(product);
        LOG_INFO("Successfully created product with id: " + std::to_string(id) + ", sku: " + sku);
    } catch (const SpacetimeDB::UniqueConstraintViolationException& e) {
        LOG_INFO("Constraint violation: " + std::string(e.what()));
    }
}

// Test duplicate detection
SPACETIMEDB_REDUCER(test_duplicate_user, ReducerContext ctx, uint32_t id, std::string email) {
    User user1{.id = id, .email = email, .name = "User1", .age = 25};
    User user2{.id = id, .email = email, .name = "User2", .age = 30};
    
    try {
        ctx.db->users().insert(user1);
        LOG_INFO("First user inserted successfully");
        
        ctx.db->users().insert(user2);
        LOG_INFO("ERROR: Second user with same ID should have failed!");
    } catch (const SpacetimeDB::UniqueConstraintViolationException& e) {
        LOG_INFO("Expected constraint violation caught: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(test_unique_email, ReducerContext ctx, std::string email) {
    User user1{.id = 100, .email = email, .name = "User1", .age = 25};
    User user2{.id = 101, .email = email, .name = "User2", .age = 30};
    
    try {
        ctx.db->users().insert(user1);
        LOG_INFO("First user with email inserted successfully");
        
        ctx.db->users().insert(user2);
        LOG_INFO("ERROR: Second user with same email should have failed!");
    } catch (const SpacetimeDB::UniqueConstraintViolationException& e) {
        LOG_INFO("Expected unique email violation caught: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(init_constraint_test, ReducerContext ctx) {
    LOG_INFO("Constraint test module initialized");
    
    // Insert some test data
    User test_user{.id = 1, .email = "test@example.com", .name = "Test User", .age = 25};
    Product test_product{.id = 1, .sku = "TEST001", .name = "Test Product", .price = 99.99};
    
    try {
        ctx.db->users().insert(test_user);
        ctx.db->products().insert(test_product);
        LOG_INFO("Test data inserted successfully");
    } catch (const SpacetimeDB::UniqueConstraintViolationException& e) {
        LOG_INFO("Constraint error during init: " + std::string(e.what()));
    }
}