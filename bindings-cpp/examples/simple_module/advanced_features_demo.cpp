#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/spacetimedb.h>

// Example: E-commerce product catalog with advanced features

struct Product {
    uint32_t id;
    std::string name;
    std::string category;
    double price;
    uint32_t stock_quantity;
    bool is_active;
};

SPACETIMEDB_REGISTER_FIELDS(Product,
    SPACETIMEDB_FIELD(Product, id, uint32_t);
    SPACETIMEDB_FIELD(Product, name, std::string);
    SPACETIMEDB_FIELD(Product, category, std::string);
    SPACETIMEDB_FIELD(Product, price, double);
    SPACETIMEDB_FIELD(Product, stock_quantity, uint32_t);
    SPACETIMEDB_FIELD(Product, is_active, bool);
)

// Define table with schema constraints
SPACETIMEDB_TABLE(Product, products, true)

// Define indexes for efficient querying
SPACETIMEDB_INDEX(products, idx_category, category)
SPACETIMEDB_INDEX(products, idx_price, price)
SPACETIMEDB_UNIQUE_INDEX(products, idx_name, name)

// -----------------------------------------------------------------------------
// Advanced Query Examples
// -----------------------------------------------------------------------------

SPACETIMEDB_REDUCER(find_products_in_category, SpacetimeDb::ReducerContext ctx, std::string category) {
    auto products = ctx.db.enhanced_table<Product>("products");
    
    // Use filter to find products in category
    auto results = products.filter([&category](const Product& p) {
        return p.category == category && p.is_active;
    });
    
    LOG_INFO("Found " + std::to_string(results.count()) + " products in category: " + category);
}

SPACETIMEDB_REDUCER(update_product_prices, SpacetimeDb::ReducerContext ctx, std::string category, double percent_change) {
    auto products = ctx.db.enhanced_table<Product>("products");
    
    // Update all products in category
    size_t updated = products.update_where(
        [&category](const Product& p) { return p.category == category; },
        [percent_change](Product& p) { p.price *= (1 + percent_change / 100); }
    );
    
    LOG_INFO("Updated prices for " + std::to_string(updated) + " products");
}

SPACETIMEDB_REDUCER(remove_out_of_stock, SpacetimeDb::ReducerContext ctx) {
    auto products = ctx.db.enhanced_table<Product>("products");
    
    // Delete products with no stock
    size_t deleted = products.delete_where([](const Product& p) {
        return p.stock_quantity == 0 && !p.is_active;
    });
    
    LOG_INFO("Removed " + std::to_string(deleted) + " out of stock products");
}

// -----------------------------------------------------------------------------
// Index-based Queries
// -----------------------------------------------------------------------------

SPACETIMEDB_REDUCER(find_products_by_price_range, SpacetimeDb::ReducerContext ctx, double min_price, double max_price) {
    auto products = ctx.db.enhanced_table<Product>("products");
    auto price_index = products.btree_index<double>("idx_price");
    
    // Use index for efficient range query
    auto results = price_index.range(
        Bound<double>::included(min_price),
        Bound<double>::excluded(max_price)
    );
    
    uint32_t count = 0;
    for (auto it = results.begin(); it != results.end(); ++it) {
        count++;
    }
    
    LOG_INFO("Found " + std::to_string(count) + " products in price range");
}

SPACETIMEDB_REDUCER(check_product_exists, SpacetimeDb::ReducerContext ctx, std::string name) {
    auto products = ctx.db.enhanced_table<Product>("products");
    auto name_index = products.unique_index<std::string>("idx_name");
    
    // Use unique index for O(1) lookup
    auto result = name_index.get(name);
    
    if (result.has_value()) {
        LOG_INFO("Product '" + name + "' exists with ID: " + std::to_string(result->id));
    } else {
        LOG_INFO("Product '" + name + "' not found");
    }
}

// -----------------------------------------------------------------------------
// Query Builder Examples
// -----------------------------------------------------------------------------

SPACETIMEDB_REDUCER(complex_product_search, SpacetimeDb::ReducerContext ctx, std::string category, double max_price) {
    auto products = ctx.db.enhanced_table<Product>("products");
    
    // Build complex query
    auto results = products.query()
        .where([&category](const Product& p) { return p.category == category; })
        .and_where([&max_price](const Product& p) { return p.price <= max_price; })
        .and_where([](const Product& p) { return p.is_active && p.stock_quantity > 0; })
        .order_by([](const Product& a, const Product& b) { return a.price < b.price; })
        .limit(10)
        .execute();
    
    LOG_INFO("Found " + std::to_string(results.size()) + " matching products");
}

// -----------------------------------------------------------------------------
// Schema Definition Example
// -----------------------------------------------------------------------------

void define_schema() {
    // This demonstrates the schema definition API
    // In a real implementation, this would integrate with module registration
    
    SpacetimeDb::define_table<Product>("products")
        .primary_key("id")
        .auto_increment("id")
        .unique("name")
        .indexed("category")
        .indexed("price")
        .register_table();
}

// Init reducer to demonstrate schema setup
SPACETIMEDB_INIT(on_init) {
    LOG_INFO("E-commerce catalog module initialized");
    
    // Create some sample products
    Product samples[] = {
        {0, "Laptop Pro", "Electronics", 1299.99, 50, true},
        {0, "Wireless Mouse", "Electronics", 29.99, 200, true},
        {0, "Office Chair", "Furniture", 399.99, 25, true},
        {0, "Standing Desk", "Furniture", 599.99, 15, true},
        {0, "USB-C Cable", "Electronics", 19.99, 500, true}
    };
    
    auto products = ctx.db.table<Product>("products");
    for (const auto& product : samples) {
        products.insert(product);
    }
    
    LOG_INFO("Sample products created");
}