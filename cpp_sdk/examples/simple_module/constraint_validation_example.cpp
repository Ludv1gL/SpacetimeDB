#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/constraint_validation.h>
#include <spacetimedb/validated_table.h>
#include <iostream>
#include <iomanip>

using namespace spacetimedb;

// Example 1: User table with various constraints
struct User {
    uint32_t id;           // Primary key, auto-increment
    std::string username;  // Unique, not null, length <= 50
    std::string email;     // Unique, not null, email pattern
    uint8_t age;          // Check: age >= 18 && age <= 150
    std::optional<std::string> bio;  // Optional, max length 500
    
    // BSATN serialization (required for SpacetimeDB)
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_u32(id);
        writer.write_string(username);
        writer.write_string(email);
        writer.write_u8(age);
        writer.write_option(bio);
    }
    
    void bsatn_deserialize(bsatn::Reader& reader) {
        id = reader.read_u32();
        username = reader.read_string();
        email = reader.read_string();
        age = reader.read_u8();
        bio = reader.read_option<std::string>();
    }
};

// Register fields for User
SPACETIMEDB_REGISTER_FIELDS(User,
    SPACETIMEDB_FIELD(User, id, uint32_t);
    SPACETIMEDB_FIELD(User, username, std::string);
    SPACETIMEDB_FIELD(User, email, std::string);
    SPACETIMEDB_FIELD(User, age, uint8_t);
    SPACETIMEDB_FIELD(User, bio, std::optional<std::string>);
)

// Define the table with constraints
SPACETIMEDB_TABLE(User, users, true)

// Example 2: Product table with composite constraints
struct Product {
    std::string sku;       // Primary key part 1
    std::string variant;   // Primary key part 2
    std::string name;      // Not null
    double price;          // Check: price > 0
    uint32_t stock;        // Check: stock >= 0 (implicit for unsigned)
    bool active;           // Default: true
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_string(sku);
        writer.write_string(variant);
        writer.write_string(name);
        writer.write_f64(price);
        writer.write_u32(stock);
        writer.write_bool(active);
    }
    
    void bsatn_deserialize(bsatn::Reader& reader) {
        sku = reader.read_string();
        variant = reader.read_string();
        name = reader.read_string();
        price = reader.read_f64();
        stock = reader.read_u32();
        active = reader.read_bool();
    }
};

SPACETIMEDB_REGISTER_FIELDS(Product,
    SPACETIMEDB_FIELD(Product, sku, std::string);
    SPACETIMEDB_FIELD(Product, variant, std::string);
    SPACETIMEDB_FIELD(Product, name, std::string);
    SPACETIMEDB_FIELD(Product, price, double);
    SPACETIMEDB_FIELD(Product, stock, uint32_t);
    SPACETIMEDB_FIELD(Product, active, bool);
)

SPACETIMEDB_TABLE(Product, products, true)

// Example 3: Order table with foreign key simulation
struct Order {
    uint64_t order_id;     // Primary key, auto-increment
    uint32_t user_id;      // Foreign key to User.id
    std::string product_sku;     // Foreign key to Product.sku
    std::string product_variant; // Foreign key to Product.variant
    uint32_t quantity;     // Check: quantity > 0
    double total_price;    // Check: total_price > 0
    std::string status;    // Check: status in ('pending', 'shipped', 'delivered', 'cancelled')
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_u64(order_id);
        writer.write_u32(user_id);
        writer.write_string(product_sku);
        writer.write_string(product_variant);
        writer.write_u32(quantity);
        writer.write_f64(total_price);
        writer.write_string(status);
    }
    
    void bsatn_deserialize(bsatn::Reader& reader) {
        order_id = reader.read_u64();
        user_id = reader.read_u32();
        product_sku = reader.read_string();
        product_variant = reader.read_string();
        quantity = reader.read_u32();
        total_price = reader.read_f64();
        status = reader.read_string();
    }
};

SPACETIMEDB_REGISTER_FIELDS(Order,
    SPACETIMEDB_FIELD(Order, order_id, uint64_t);
    SPACETIMEDB_FIELD(Order, user_id, uint32_t);
    SPACETIMEDB_FIELD(Order, product_sku, std::string);
    SPACETIMEDB_FIELD(Order, product_variant, std::string);
    SPACETIMEDB_FIELD(Order, quantity, uint32_t);
    SPACETIMEDB_FIELD(Order, total_price, double);
    SPACETIMEDB_FIELD(Order, status, std::string);
)

SPACETIMEDB_TABLE(Order, orders, true)

// Helper function to validate email format (simple version)
bool is_valid_email(const std::string& email) {
    return email.find('@') != std::string::npos && 
           email.find('.') != std::string::npos &&
           email.length() >= 5;
}

// Helper function to validate order status
bool is_valid_order_status(const std::string& status) {
    return status == "pending" || status == "shipped" || 
           status == "delivered" || status == "cancelled";
}

// Set up constraints for User table
std::unique_ptr<ValidatedTable<User>> setup_user_table(uint32_t table_id) {
    auto table = std::make_unique<ValidatedTable<User>>(table_id, "users");
    
    // Build constraints
    ConstraintBuilder<User> builder;
    
    // Primary key on id (unique)
    builder.unique("pk_user_id", {"id"}, [](const User& u) { return u.id; });
    
    // Unique username
    builder.unique("unique_username", {"username"}, [](const User& u) { return u.username; });
    
    // Unique email
    builder.unique("unique_email", {"email"}, [](const User& u) { return u.email; });
    
    // Not null constraints
    builder.not_null("username", [](const User& u) { return &u.username; });
    builder.not_null("email", [](const User& u) { return &u.email; });
    
    // Check constraints
    builder.check("check_age_range", 
        [](const User& u) { return u.age >= 18 && u.age <= 150; },
        "age >= 18 AND age <= 150");
    
    builder.check("check_username_length",
        [](const User& u) { return u.username.length() <= 50 && u.username.length() > 0; },
        "length(username) > 0 AND length(username) <= 50");
    
    builder.check("check_email_format",
        [](const User& u) { return is_valid_email(u.email); },
        "email must be valid format");
    
    builder.check("check_bio_length",
        [](const User& u) { return !u.bio || u.bio->length() <= 500; },
        "length(bio) <= 500");
    
    // Add all constraints to table
    auto composite = builder.build();
    table->constraints().add_validator(
        std::make_unique<CompositeValidator<User>>(std::move(composite))
    );
    
    return table;
}

// Set up constraints for Product table
std::unique_ptr<ValidatedTable<Product>> setup_product_table(uint32_t table_id) {
    auto table = std::make_unique<ValidatedTable<Product>>(table_id, "products");
    
    ConstraintBuilder<Product> builder;
    
    // Composite primary key
    builder.unique("pk_product", {"sku", "variant"}, 
        [](const Product& p) { return std::make_pair(p.sku, p.variant); });
    
    // Not null constraints
    builder.not_null("sku", [](const Product& p) { return &p.sku; });
    builder.not_null("variant", [](const Product& p) { return &p.variant; });
    builder.not_null("name", [](const Product& p) { return &p.name; });
    
    // Check constraints
    builder.check("check_price_positive",
        [](const Product& p) { return p.price > 0; },
        "price > 0");
    
    builder.check("check_sku_format",
        [](const Product& p) { return p.sku.length() >= 3 && p.sku.length() <= 20; },
        "length(sku) BETWEEN 3 AND 20");
    
    auto composite = builder.build();
    table->constraints().add_validator(
        std::make_unique<CompositeValidator<Product>>(std::move(composite))
    );
    
    return table;
}

// Set up constraints for Order table
std::unique_ptr<ValidatedTable<Order>> setup_order_table(uint32_t table_id) {
    auto table = std::make_unique<ValidatedTable<Order>>(table_id, "orders");
    
    ConstraintBuilder<Order> builder;
    
    // Primary key
    builder.unique("pk_order_id", {"order_id"}, [](const Order& o) { return o.order_id; });
    
    // Check constraints
    builder.check("check_quantity_positive",
        [](const Order& o) { return o.quantity > 0; },
        "quantity > 0");
    
    builder.check("check_total_price_positive",
        [](const Order& o) { return o.total_price > 0; },
        "total_price > 0");
    
    builder.check("check_valid_status",
        [](const Order& o) { return is_valid_order_status(o.status); },
        "status IN ('pending', 'shipped', 'delivered', 'cancelled')");
    
    auto composite = builder.build();
    table->constraints().add_validator(
        std::make_unique<CompositeValidator<Order>>(std::move(composite))
    );
    
    return table;
}

// Demo reducer: Test constraint validation
SPACETIMEDB_REDUCER(test_constraints, spacetimedb::ReducerContext ctx) {
    std::cout << "\n=== SpacetimeDB C++ Constraint Validation Demo ===\n" << std::endl;
    
    // Create validated tables (in real code, these would be managed by the module)
    auto user_table = setup_user_table(1);  // Mock table ID
    auto product_table = setup_product_table(2);
    auto order_table = setup_order_table(3);
    
    // Test 1: Valid user insertion
    std::cout << "Test 1: Insert valid user" << std::endl;
    try {
        User valid_user{1, "john_doe", "john@example.com", 25, std::nullopt};
        auto result = user_table->constraints().validate(valid_user);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 2: Invalid age
    std::cout << "\nTest 2: Insert user with invalid age" << std::endl;
    try {
        User invalid_age{2, "jane_doe", "jane@example.com", 16, std::nullopt};
        auto result = user_table->constraints().validate(invalid_age);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 3: Invalid email format
    std::cout << "\nTest 3: Insert user with invalid email" << std::endl;
    try {
        User invalid_email{3, "bob_smith", "not-an-email", 30, std::nullopt};
        auto result = user_table->constraints().validate(invalid_email);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 4: Username too long
    std::cout << "\nTest 4: Insert user with username too long" << std::endl;
    try {
        std::string long_username(55, 'a');  // 55 characters
        User long_name{4, long_username, "long@example.com", 25, std::nullopt};
        auto result = user_table->constraints().validate(long_name);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 5: Valid product
    std::cout << "\nTest 5: Insert valid product" << std::endl;
    try {
        Product valid_product{"SKU001", "BLUE", "Blue Widget", 19.99, 100, true};
        auto result = product_table->constraints().validate(valid_product);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 6: Invalid price
    std::cout << "\nTest 6: Insert product with invalid price" << std::endl;
    try {
        Product invalid_price{"SKU002", "RED", "Red Widget", -5.00, 50, true};
        auto result = product_table->constraints().validate(invalid_price);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 7: Valid order
    std::cout << "\nTest 7: Insert valid order" << std::endl;
    try {
        Order valid_order{1001, 1, "SKU001", "BLUE", 2, 39.98, "pending"};
        auto result = order_table->constraints().validate(valid_order);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 8: Invalid order status
    std::cout << "\nTest 8: Insert order with invalid status" << std::endl;
    try {
        Order invalid_status{1002, 1, "SKU001", "BLUE", 1, 19.99, "processing"};
        auto result = order_table->constraints().validate(invalid_status);
        std::cout << "  Validation result: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << std::endl;
    }
    
    // Test 9: Batch validation
    std::cout << "\nTest 9: Batch validation with mixed valid/invalid data" << std::endl;
    std::vector<User> user_batch = {
        {5, "alice", "alice@example.com", 22, std::nullopt},      // Valid
        {6, "bob", "bob@example", 19, std::nullopt},              // Invalid email
        {7, "charlie", "charlie@example.com", 200, std::nullopt}, // Invalid age
        {8, "diana", "diana@example.com", 35, std::nullopt}       // Valid
    };
    
    std::cout << "  Validating batch of " << user_batch.size() << " users:" << std::endl;
    for (size_t i = 0; i < user_batch.size(); ++i) {
        auto result = user_table->constraints().validate(user_batch[i]);
        std::cout << "    User " << i + 1 << ": " 
                  << (result.is_valid() ? "VALID" : "INVALID");
        if (!result.is_valid()) {
            std::cout << " - " << result.violations()[0].message;
        }
        std::cout << std::endl;
    }
    
    // Test 10: Transaction with validation disabled
    std::cout << "\nTest 10: Bulk insert with validation disabled" << std::endl;
    {
        ValidationTransaction<User> txn(*user_table, false);  // Disable validation
        std::cout << "  Validation disabled for bulk operation" << std::endl;
        
        // These would normally fail validation
        User bulk_user1{100, "x", "bad", 10, std::nullopt};
        auto result = user_table->constraints().validate(bulk_user1);
        std::cout << "  Inserting invalid data: " 
                  << (result.is_valid() ? "Would succeed" : "Would fail") 
                  << " (validation disabled)" << std::endl;
    }
    std::cout << "  Validation re-enabled after transaction" << std::endl;
    
    std::cout << "\n=== Constraint Validation Demo Complete ===\n" << std::endl;
}

// Main entry point
SPACETIMEDB_REDUCER(__init__, spacetimedb::ReducerContext ctx) {
    std::cout << "Constraint Validation Module Initialized" << std::endl;
}

// Helper reducer to show table schemas with constraints
SPACETIMEDB_REDUCER(show_constraints, spacetimedb::ReducerContext ctx) {
    std::cout << "\n=== Table Constraint Definitions ===\n" << std::endl;
    
    std::cout << "Users Table:" << std::endl;
    std::cout << "  - Primary Key: id (auto-increment)" << std::endl;
    std::cout << "  - Unique: username, email" << std::endl;
    std::cout << "  - Not Null: username, email" << std::endl;
    std::cout << "  - Check: age BETWEEN 18 AND 150" << std::endl;
    std::cout << "  - Check: length(username) <= 50" << std::endl;
    std::cout << "  - Check: email format validation" << std::endl;
    std::cout << "  - Check: length(bio) <= 500 (if provided)" << std::endl;
    
    std::cout << "\nProducts Table:" << std::endl;
    std::cout << "  - Primary Key: (sku, variant) composite" << std::endl;
    std::cout << "  - Not Null: sku, variant, name" << std::endl;
    std::cout << "  - Check: price > 0" << std::endl;
    std::cout << "  - Check: length(sku) BETWEEN 3 AND 20" << std::endl;
    
    std::cout << "\nOrders Table:" << std::endl;
    std::cout << "  - Primary Key: order_id (auto-increment)" << std::endl;
    std::cout << "  - Foreign Key: user_id REFERENCES users(id)" << std::endl;
    std::cout << "  - Foreign Key: (product_sku, product_variant) REFERENCES products(sku, variant)" << std::endl;
    std::cout << "  - Check: quantity > 0" << std::endl;
    std::cout << "  - Check: total_price > 0" << std::endl;
    std::cout << "  - Check: status IN ('pending', 'shipped', 'delivered', 'cancelled')" << std::endl;
    
    std::cout << "\n=================================\n" << std::endl;
}