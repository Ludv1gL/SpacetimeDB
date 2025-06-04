#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/spacetimedb_autogen.h>

using namespace spacetimedb;

// Example struct with renamed fields
struct UserProfile {
    uint32_t id;
    std::string username;      // Will be stored as "user_name" in database
    std::string emailAddress;  // Will be stored as "email" in database
    uint64_t createdAt;        // Will be stored as "created_timestamp" in database
};

// Register the type with field renaming
SPACETIMEDB_REGISTER_TYPE(UserProfile, 
    SPACETIMEDB_FIELD(UserProfile, id),
    SPACETIMEDB_FIELD_RENAMED(UserProfile, username, "user_name"),
    SPACETIMEDB_FIELD_RENAMED(UserProfile, emailAddress, "email"),
    SPACETIMEDB_FIELD_RENAMED(UserProfile, createdAt, "created_timestamp")
)

// Register as a table
SPACETIMEDB_TABLE(UserProfile, "user_profiles", true)

// Reducer that creates a new user profile
SPACETIMEDB_REDUCER(create_user_profile, UserDefined, ctx, 
    std::string username, std::string email)
{
    UserProfile profile{
        .id = 0,  // Will be auto-generated
        .username = username,
        .emailAddress = email,
        .createdAt = static_cast<uint64_t>(std::time(nullptr))
    };
    
    auto table = get_UserProfile_table();
    table.insert(profile);
    
    LOG_INFO("Created user profile for: " + username);
}

// Alternative approach using attributes (for future implementation)
// This would require C++ attribute support in the compiler
/*
struct ModernUserProfile {
    [[spacetimedb::primary_key, spacetimedb::auto_inc]]
    uint32_t id;
    
    [[spacetimedb::rename("user_name"), spacetimedb::unique]]
    std::string username;
    
    [[spacetimedb::rename("email"), spacetimedb::index]]
    std::string emailAddress;
    
    [[spacetimedb::rename("created_timestamp")]]
    uint64_t createdAt;
};
*/

// Example of field renaming with complex types
struct OrderItem {
    uint64_t orderId;         // Renamed to "order_id"
    uint32_t productId;       // Renamed to "product_id"
    uint16_t quantity;
    double unitPrice;         // Renamed to "unit_price"
    std::optional<std::string> discountCode;  // Renamed to "discount_code"
};

SPACETIMEDB_REGISTER_TYPE(OrderItem,
    SPACETIMEDB_FIELD_RENAMED(OrderItem, orderId, "order_id"),
    SPACETIMEDB_FIELD_RENAMED(OrderItem, productId, "product_id"),
    SPACETIMEDB_FIELD(OrderItem, quantity),
    SPACETIMEDB_FIELD_RENAMED(OrderItem, unitPrice, "unit_price"),
    SPACETIMEDB_FIELD_RENAMED(OrderItem, discountCode, "discount_code")
)

SPACETIMEDB_TABLE(OrderItem, "order_items", true)

// Init reducer to set up test data
SPACETIMEDB_INIT(init_module, ctx)
{
    LOG_INFO("Field rename example module initialized");
    
    // Create a test user
    UserProfile test_user{
        .id = 0,
        .username = "test_user",
        .emailAddress = "test@example.com",
        .createdAt = static_cast<uint64_t>(std::time(nullptr))
    };
    
    auto user_table = get_UserProfile_table();
    user_table.insert(test_user);
}

// Query reducer demonstrating field access
SPACETIMEDB_REDUCER(get_user_by_email, UserDefined, ctx, std::string email)
{
    auto user_table = get_UserProfile_table();
    
    // Note: Even though the database column is "email", 
    // we still access it using the C++ field name "emailAddress"
    for (const auto& user : user_table.iter()) {
        if (user.emailAddress == email) {
            LOG_INFO("Found user: " + user.username);
            return;
        }
    }
    
    LOG_WARN("User not found with email: " + email);
}