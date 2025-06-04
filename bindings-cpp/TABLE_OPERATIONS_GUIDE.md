# SpacetimeDB C++ Module Library - Table Operations Guide

This guide provides comprehensive documentation for all table operations available in the SpacetimeDB C++ Module Library.

## Overview

The C++ Module Library provides a clean, type-safe API for database operations through the reducer context (`ctx`). All operations are strongly typed and follow modern C++ patterns.

## Basic Table Operations

### Insert

Insert new rows into a table:

```cpp
SPACETIMEDB_REDUCER(add_user, ctx, std::string name, uint32_t age) {
    User user{.name = name, .age = age};
    User inserted = ctx.insert(user);
    // inserted.id will contain the auto-generated ID
}
```

### Update

Update existing rows:

```cpp
SPACETIMEDB_REDUCER(update_user_age, ctx, uint32_t user_id, uint32_t new_age) {
    auto user = ctx.find<User>(user_id);
    if (user) {
        user->age = new_age;
        ctx.update(std::move(*user));
    }
}
```

### Delete

Delete rows by various criteria:

```cpp
// Delete by primary key
ctx.delete_by_id<User>(user_id);

// Delete by condition
ctx.delete_where<User>([](const User& u) {
    return u.age > 100;
});

// Delete single row
ctx.delete_row(user);
```

### Query

Multiple ways to query data:

```cpp
// Find by primary key
auto user = ctx.find<User>(user_id);

// Select with predicate
auto adults = ctx.select<User>([](const User& u) {
    return u.age >= 18;
});

// Get all rows
auto all_users = ctx.all<User>();

// Check existence
bool exists = ctx.exists<User>([](const User& u) {
    return u.name == "Alice";
});
```

## Advanced Queries

### Using Indexes

```cpp
// Query using unique index
auto user = ctx.find_by_unique<User>("email", "alice@example.com");

// Query using btree index
auto users = ctx.find_by_index<User>("age_idx", 25);
```

### Filtering and Sorting

```cpp
// Complex filtering
auto results = ctx.select<User>([](const User& u) {
    return u.age >= 18 && u.age <= 65 && u.active;
});

// With ordering (in memory)
std::sort(results.begin(), results.end(), 
    [](const User& a, const User& b) {
        return a.name < b.name;
    });
```

### Joins (Manual)

```cpp
SPACETIMEDB_REDUCER(get_user_orders, ctx, uint32_t user_id) {
    auto user = ctx.find<User>(user_id);
    if (!user) return;
    
    auto orders = ctx.select<Order>([&](const Order& o) {
        return o.user_id == user_id;
    });
    
    // Process user with their orders
}
```

## Batch Operations

### Bulk Insert

```cpp
SPACETIMEDB_REDUCER(import_users, ctx, std::vector<UserData> user_data) {
    for (const auto& data : user_data) {
        User user{.name = data.name, .age = data.age};
        ctx.insert(user);
    }
}
```

### Bulk Update

```cpp
SPACETIMEDB_REDUCER(deactivate_old_users, ctx) {
    auto old_users = ctx.select<User>([](const User& u) {
        return u.last_login < spacetimedb::now() - spacetimedb::days(365);
    });
    
    for (auto& user : old_users) {
        user.active = false;
        ctx.update(std::move(user));
    }
}
```

## Table Iteration

### Basic Iteration

```cpp
// Iterate all rows
for (const auto& user : ctx.iter<User>()) {
    spacetimedb::log("User: " + user.name);
}
```

### Early Termination

```cpp
// Find first matching
std::optional<User> found;
for (const auto& user : ctx.iter<User>()) {
    if (user.email == target_email) {
        found = user;
        break;
    }
}
```

## Counting and Aggregation

```cpp
// Count all rows
size_t total_users = ctx.count<User>();

// Count with condition
size_t active_users = ctx.count<User>([](const User& u) {
    return u.active;
});

// Manual aggregation
double total_balance = 0;
for (const auto& account : ctx.iter<Account>()) {
    total_balance += account.balance;
}
```

## Working with Auto-increment Fields

```cpp
SPACETIMEDB_TABLE(Post, posts, public,
    autoinc<uint32_t> id;
    std::string title;
    std::string content
);

SPACETIMEDB_REDUCER(create_post, ctx, std::string title, std::string content) {
    Post post{.title = title, .content = content};
    Post inserted = ctx.insert(post);
    spacetimedb::log("Created post with ID: " + std::to_string(inserted.id));
}
```

## Error Handling

```cpp
SPACETIMEDB_REDUCER(safe_update, ctx, uint32_t id, std::string new_value) {
    auto item = ctx.find<Item>(id);
    if (!item) {
        ctx.fail("Item not found");
        return;
    }
    
    if (new_value.empty()) {
        ctx.fail("Value cannot be empty");
        return;
    }
    
    item->value = new_value;
    ctx.update(std::move(*item));
}
```

## Performance Considerations

### Efficient Queries

```cpp
// Good: Use indexes when available
auto user = ctx.find<User>(user_id);  // Uses primary key index

// Avoid: Full table scans for single items
auto user = ctx.select<User>([&](const User& u) {
    return u.id == user_id;  // Scans entire table
}).front();
```

### Batch vs Individual Operations

```cpp
// Good: Batch operations when possible
std::vector<User> users_to_insert;
// ... populate vector
for (const auto& user : users_to_insert) {
    ctx.insert(user);
}

// Consider: Transaction support (when available)
// ctx.transaction([&] {
//     for (const auto& user : users_to_insert) {
//         ctx.insert(user);
//     }
// });
```

## Best Practices

1. **Use the Right Operation**: Use `find` for primary key lookups, `select` for filtering
2. **Handle Missing Data**: Always check optional returns from `find` operations
3. **Validate Input**: Validate data before database operations
4. **Use Indexes**: Design tables with appropriate indexes for common queries
5. **Batch When Possible**: Group related operations together
6. **Error Messages**: Provide clear error messages with `ctx.fail()`

## Example: Complete CRUD Module

```cpp
#include <spacetimedb/spacetimedb.h>

SPACETIMEDB_TABLE(Product, products, public,
    autoinc<uint32_t> id;
    std::string name;
    std::string description;
    double price;
    uint32_t stock
);

// Create
SPACETIMEDB_REDUCER(create_product, ctx, 
    std::string name, std::string description, double price, uint32_t stock) {
    
    if (name.empty()) {
        ctx.fail("Product name is required");
        return;
    }
    
    if (price < 0) {
        ctx.fail("Price cannot be negative");
        return;
    }
    
    Product product{
        .name = name,
        .description = description,
        .price = price,
        .stock = stock
    };
    
    auto inserted = ctx.insert(product);
    spacetimedb::log("Created product: " + name + " with ID: " + std::to_string(inserted.id));
}

// Read
SPACETIMEDB_REDUCER(get_product, ctx, uint32_t product_id) {
    auto product = ctx.find<Product>(product_id);
    if (!product) {
        ctx.fail("Product not found");
        return;
    }
    
    spacetimedb::log("Product: " + product->name + ", Price: $" + std::to_string(product->price));
}

// Update
SPACETIMEDB_REDUCER(update_price, ctx, uint32_t product_id, double new_price) {
    if (new_price < 0) {
        ctx.fail("Price cannot be negative");
        return;
    }
    
    auto product = ctx.find<Product>(product_id);
    if (!product) {
        ctx.fail("Product not found");
        return;
    }
    
    product->price = new_price;
    ctx.update(std::move(*product));
    spacetimedb::log("Updated price for: " + product->name);
}

// Delete
SPACETIMEDB_REDUCER(delete_product, ctx, uint32_t product_id) {
    auto product = ctx.find<Product>(product_id);
    if (!product) {
        ctx.fail("Product not found");
        return;
    }
    
    std::string name = product->name;
    ctx.delete_by_id<Product>(product_id);
    spacetimedb::log("Deleted product: " + name);
}

// List with filtering
SPACETIMEDB_REDUCER(list_products_in_stock, ctx) {
    auto products = ctx.select<Product>([](const Product& p) {
        return p.stock > 0;
    });
    
    spacetimedb::log("Products in stock: " + std::to_string(products.size()));
    for (const auto& product : products) {
        spacetimedb::log("- " + product.name + " (" + std::to_string(product.stock) + " units)");
    }
}
```

## Summary

The SpacetimeDB C++ Module Library provides a complete set of table operations that are:
- **Type-safe**: Compile-time checking prevents errors
- **Efficient**: Operations map directly to database primitives
- **Intuitive**: Follows modern C++ patterns and conventions
- **Complete**: Full CRUD operations with advanced features

For more examples, see the `examples/` directory in the repository.