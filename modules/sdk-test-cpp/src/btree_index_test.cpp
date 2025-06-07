/**
 * BTree Index Operations Test Module
 * 
 * Demonstrates:
 * - Index creation via macros
 * - Range queries using BTree indexes
 * - Index-based deletion
 * - Iterator-based filtering
 */

#define SPACETIMEDB_TABLES_LIST \
    X(Product, products, true) \
    X(Order, orders, true)

#include "spacetimedb/spacetimedb.h"

using namespace SpacetimeDb;

// Product table with indexed fields
struct Product {
    uint32_t id;
    std::string name;
    double price;
    uint32_t category_id;
    uint32_t stock_quantity;
};

// Order table with indexed fields  
struct Order {
    uint32_t id;
    uint32_t product_id;
    uint32_t customer_id;
    double amount;
    uint64_t order_date;  // Unix timestamp
};

// BSATN traits for Product
namespace SpacetimeDb::bsatn {
    template<>
    struct bsatn_traits<Product> {
        static void serialize(Writer& writer, const Product& value) {
            writer.write_u32_le(value.id);
            serialize(writer, value.name);
            writer.write_f64_le(value.price);
            writer.write_u32_le(value.category_id);
            writer.write_u32_le(value.stock_quantity);
        }
        
        static Product deserialize(Reader& reader) {
            Product result;
            result.id = reader.read_u32_le();
            result.name = reader.read_string();
            result.price = reader.read_f64_le();
            result.category_id = reader.read_u32_le();
            result.stock_quantity = reader.read_u32_le();
            return result;
        }
    };

    template<>
    struct bsatn_traits<Order> {
        static void serialize(Writer& writer, const Order& value) {
            writer.write_u32_le(value.id);
            writer.write_u32_le(value.product_id);
            writer.write_u32_le(value.customer_id);
            writer.write_f64_le(value.amount);
            writer.write_u64_le(value.order_date);
        }
        
        static Order deserialize(Reader& reader) {
            Order result;
            result.id = reader.read_u32_le();
            result.product_id = reader.read_u32_le();
            result.customer_id = reader.read_u32_le();
            result.amount = reader.read_f64_le();
            result.order_date = reader.read_u64_le();
            return result;
        }
    };
}

// Create indexes for efficient queries
SPACETIMEDB_PRIMARY_KEY(Product, id);
SPACETIMEDB_INDEX(Product, price, price_idx);
SPACETIMEDB_INDEX(Product, category_id, category_idx);

SPACETIMEDB_PRIMARY_KEY(Order, id);
SPACETIMEDB_INDEX(Order, product_id, product_idx);
SPACETIMEDB_INDEX(Order, customer_id, customer_idx);
SPACETIMEDB_INDEX(Order, order_date, date_idx);

// Helper function to create a BTree index for products by price
std::unique_ptr<BTreeIndex<Product, double>> GetProductPriceIndex() {
    return std::make_unique<BTreeIndex<Product, double>>("price_idx");
}

// Helper function to create a BTree index for orders by date
std::unique_ptr<BTreeIndex<Order, uint64_t>> GetOrderDateIndex() {
    return std::make_unique<BTreeIndex<Order, uint64_t>>("date_idx");
}

SPACETIMEDB_REDUCER(seed_test_data, ReducerContext ctx) {
    LOG_INFO("Seeding test data for BTree index operations");
    
    // Insert test products with various prices
    std::vector<Product> products = {
        {1, "Laptop", 999.99, 1, 10},
        {2, "Mouse", 29.99, 1, 50},
        {3, "Keyboard", 79.99, 1, 30},
        {4, "Monitor", 299.99, 2, 15},
        {5, "Tablet", 399.99, 3, 25},
        {6, "Phone", 699.99, 3, 20},
        {7, "Headphones", 149.99, 4, 40},
        {8, "Speakers", 199.99, 4, 35}
    };
    
    for (const auto& product : products) {
        ctx.db->products().insert(product);
    }
    
    // Insert test orders
    std::vector<Order> orders = {
        {1, 1, 101, 999.99, 1640995200},  // 2022-01-01
        {2, 2, 102, 29.99, 1641081600},   // 2022-01-02
        {3, 1, 103, 999.99, 1641168000},  // 2022-01-03
        {4, 3, 101, 79.99, 1641254400},   // 2022-01-04
        {5, 5, 104, 399.99, 1641340800},  // 2022-01-05
    };
    
    for (const auto& order : orders) {
        ctx.db->orders().insert(order);
    }
    
    LOG_INFO("Test data seeded successfully");
}

SPACETIMEDB_REDUCER(test_price_range_query, ReducerContext ctx, double min_price, double max_price) {
    LOG_INFO("Testing price range query: " + std::to_string(min_price) + " to " + std::to_string(max_price));
    
    try {
        auto price_index = GetProductPriceIndex();
        
        // Create range for price query
        Range<double> price_range(
            Bound<double>::Inclusive(min_price),
            Bound<double>::Inclusive(max_price)
        );
        
        // Perform range query
        auto products = price_index->FilterRange(price_range);
        
        LOG_INFO("Found " + std::to_string(products.size()) + " products in price range");
        
        // Log each found product (if type registration works)
        for (size_t i = 0; i < products.size(); ++i) {
            // Note: Until BSATN type registration is complete, 
            // we can't access individual fields, but we can count results
            LOG_INFO("Product " + std::to_string(i + 1) + " found in range");
        }
        
    } catch (const std::exception& e) {
        LOG_INFO("Price range query failed: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(test_date_range_query, ReducerContext ctx, uint64_t start_date, uint64_t end_date) {
    LOG_INFO("Testing date range query: " + std::to_string(start_date) + " to " + std::to_string(end_date));
    
    try {
        auto date_index = GetOrderDateIndex();
        
        // Create range for date query
        Range<uint64_t> date_range(
            Bound<uint64_t>::Inclusive(start_date),
            Bound<uint64_t>::Inclusive(end_date)
        );
        
        // Perform range query
        auto orders = date_index->FilterRange(date_range);
        
        LOG_INFO("Found " + std::to_string(orders.size()) + " orders in date range");
        
    } catch (const std::exception& e) {
        LOG_INFO("Date range query failed: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(test_exact_price_query, ReducerContext ctx, double exact_price) {
    LOG_INFO("Testing exact price query: " + std::to_string(exact_price));
    
    try {
        auto price_index = GetProductPriceIndex();
        
        // Perform exact match query
        auto products = price_index->Filter(exact_price);
        
        LOG_INFO("Found " + std::to_string(products.size()) + " products with exact price");
        
    } catch (const std::exception& e) {
        LOG_INFO("Exact price query failed: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(test_iterator_query, ReducerContext ctx, double min_price, double max_price) {
    LOG_INFO("Testing iterator-based price query: " + std::to_string(min_price) + " to " + std::to_string(max_price));
    
    try {
        auto price_index = GetProductPriceIndex();
        
        // Create range for iterator query
        Range<double> price_range(
            Bound<double>::Inclusive(min_price),
            Bound<double>::Inclusive(max_price)
        );
        
        // Use iterator to scan results
        uint32_t count = 0;
        for (auto it = price_index->begin(price_range); it != price_index->end(); ++it) {
            count++;
            // Note: We can't access product fields until BSATN type registration is complete
            // but we can count the iterations
        }
        
        LOG_INFO("Iterator found " + std::to_string(count) + " products in price range");
        
    } catch (const std::exception& e) {
        LOG_INFO("Iterator query failed: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(test_index_deletion, ReducerContext ctx, double min_price, double max_price) {
    LOG_INFO("Testing index-based deletion: " + std::to_string(min_price) + " to " + std::to_string(max_price));
    
    try {
        auto price_index = GetProductPriceIndex();
        
        // Create range for deletion
        Range<double> price_range(
            Bound<double>::Inclusive(min_price),
            Bound<double>::Inclusive(max_price)
        );
        
        // Delete products in price range
        uint32_t deleted = price_index->DeleteRange(price_range);
        
        LOG_INFO("Deleted " + std::to_string(deleted) + " products in price range");
        
    } catch (const std::exception& e) {
        LOG_INFO("Index deletion failed: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(init_btree_index_test, ReducerContext ctx) {
    LOG_INFO("BTree Index Test Module initialized");
    LOG_INFO("Available reducers:");
    LOG_INFO("  seed_test_data - Populate test data");
    LOG_INFO("  test_price_range_query min_price max_price - Query products by price range");
    LOG_INFO("  test_date_range_query start_date end_date - Query orders by date range");
    LOG_INFO("  test_exact_price_query price - Find products with exact price");
    LOG_INFO("  test_iterator_query min_price max_price - Iterator-based price query");
    LOG_INFO("  test_index_deletion min_price max_price - Delete products by price range");
}