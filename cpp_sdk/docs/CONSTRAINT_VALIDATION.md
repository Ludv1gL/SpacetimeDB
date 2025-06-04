# SpacetimeDB C++ SDK Constraint Validation System

## Overview

The SpacetimeDB C++ SDK now includes a comprehensive constraint validation system that provides runtime validation for database constraints. This system allows you to enforce data integrity rules before operations reach the database, providing better error messages and preventing invalid data from being inserted.

## Features

### Supported Constraint Types

1. **NOT NULL Constraints** - Ensure required fields are always provided
2. **UNIQUE Constraints** - Prevent duplicate values in columns
3. **PRIMARY KEY Constraints** - Combination of UNIQUE and NOT NULL
4. **CHECK Constraints** - Custom validation logic for complex rules
5. **FOREIGN KEY Constraints** - Referential integrity between tables
6. **DATA TYPE Constraints** - Validate field types and formats
7. **Composite Constraints** - Multi-column unique keys

### Key Components

#### 1. `IConstraintValidator<T>` Interface

Base interface for all constraint validators:

```cpp
template<typename T>
class IConstraintValidator {
public:
    virtual ValidationResult validate(const T& row) const = 0;
    virtual std::string name() const = 0;
};
```

#### 2. `ValidationResult` Class

Holds validation results with detailed error information:

```cpp
ValidationResult result = validator.validate(row);
if (!result.is_valid()) {
    for (const auto& violation : result.violations()) {
        std::cout << violation.message << std::endl;
    }
}
```

#### 3. `ConstraintBuilder<T>` Class

Fluent API for building table constraints:

```cpp
ConstraintBuilder<User> builder;
builder.unique("pk_id", {"id"}, [](const User& u) { return u.id; })
       .not_null("username", [](const User& u) { return &u.username; })
       .check("age_check", [](const User& u) { return u.age >= 18; }, "age >= 18");
```

#### 4. `ValidatedTable<T>` Class

Table wrapper that enforces constraints on operations:

```cpp
ValidatedTable<User> user_table(table_id, "users");
user_table.constraints().add_validator(std::move(validator));

// Insert with automatic validation
try {
    User new_user = user_table.insert(user);
} catch (const UniqueConstraintViolationException& e) {
    std::cerr << "Constraint violation: " << e.what() << std::endl;
}
```

## Usage Examples

### Basic Constraints

```cpp
// Define a table with constraints
struct User {
    uint32_t id;
    std::string username;
    std::string email;
    uint8_t age;
};

// Set up constraints
auto table = std::make_unique<ValidatedTable<User>>(table_id, "users");

ConstraintBuilder<User> builder;
builder
    // Primary key
    .unique("pk_id", {"id"}, [](const User& u) { return u.id; })
    // Unique constraint
    .unique("unique_email", {"email"}, [](const User& u) { return u.email; })
    // Not null
    .not_null("username", [](const User& u) { return &u.username; })
    // Check constraint
    .check("check_age", 
           [](const User& u) { return u.age >= 18 && u.age <= 150; },
           "age BETWEEN 18 AND 150");

table->constraints().add_validator(
    std::make_unique<CompositeValidator<User>>(builder.build())
);
```

### Foreign Key Constraints

```cpp
// Employee table with foreign key to Department
struct Employee {
    uint32_t emp_id;
    std::string name;
    uint32_t dept_id;  // Foreign key
};

// Set up foreign key validator
auto fk_validator = std::make_unique<ForeignKeyValidator<Employee, uint32_t>>(
    "fk_emp_dept",           // Constraint name
    "dept_id",               // Column name
    "departments",           // Referenced table
    "dept_id",               // Referenced column
    [](const Employee& e) { return e.dept_id; },  // Key extractor
    dept_id_cache            // Cache of valid department IDs
);

employee_table->constraints().add_validator(std::move(fk_validator));
```

### Composite Constraints

```cpp
// Product with composite primary key
struct Product {
    std::string sku;
    std::string variant;
    double price;
};

builder.unique("pk_product", {"sku", "variant"},
    [](const Product& p) { 
        return std::make_pair(p.sku, p.variant); 
    });
```

### Validation Transactions

For bulk operations where you might want to temporarily disable validation:

```cpp
{
    // Disable validation for bulk insert
    ValidationTransaction<User> txn(user_table, false);
    
    // Bulk operations without validation
    for (const auto& user : bulk_users) {
        user_table.insert(user);  // No validation
    }
}  // Validation automatically re-enabled
```

## Advanced Features

### Custom Validators

Create your own validators by implementing `IConstraintValidator<T>`:

```cpp
template<typename T>
class EmailFormatValidator : public IConstraintValidator<T> {
private:
    std::string column_name_;
    std::function<const std::string*(const T&)> getter_;
    
public:
    ValidationResult validate(const T& row) const override {
        ValidationResult result;
        const std::string* email = getter_(row);
        
        if (email && !is_valid_email(*email)) {
            result.add_violation(ConstraintViolation(
                ConstraintViolation::DATA_TYPE,
                "email_format",
                column_name_,
                "Invalid email format"
            ));
        }
        return result;
    }
};
```

### Constraint Caching

The system includes caching for expensive validations like unique constraints:

```cpp
// Unique validator with cache
auto cache = std::make_shared<UniqueConstraintCache<std::string>>();
auto validator = std::make_unique<UniqueValidator<User, std::string>>(
    "unique_email", 
    {"email"}, 
    [](const User& u) { return u.email; },
    cache  // Shared cache
);

// Cache is automatically updated on insert/delete
```

### Error Handling

The system provides detailed error information:

```cpp
auto result = table->try_insert(row);
if (result.has_value()) {
    // Handle specific error types
    switch (result->type) {
        case InsertError::UniqueViolation:
            std::cerr << "Duplicate key: " << result->message << std::endl;
            break;
        case InsertError::AutoIncOverflow:
            std::cerr << "Auto-increment overflow" << std::endl;
            break;
    }
}
```

## Best Practices

1. **Define Constraints Early**: Set up all constraints when initializing tables
2. **Use Caching**: For unique constraints on large tables, use caching
3. **Batch Validation**: Validate multiple rows before inserting for better performance
4. **Clear Error Messages**: Provide descriptive constraint expressions for debugging
5. **Transaction Safety**: Use `ValidationTransaction` for bulk operations
6. **Foreign Key Management**: Keep foreign key caches synchronized with parent tables

## Performance Considerations

- Validation adds overhead to insert/update operations
- Caching can significantly improve performance for unique constraints
- Check constraints should be kept simple and fast
- Consider disabling validation for bulk imports and re-enabling after

## Integration with SpacetimeDB

The constraint validation system integrates seamlessly with SpacetimeDB's table operations:

```cpp
// In your reducer
SPACETIMEDB_REDUCER(add_user, spacetimedb::ReducerContext ctx, 
                    std::string username, std::string email, uint8_t age) {
    
    // Get or create validated table
    auto& user_table = get_validated_user_table();
    
    // Create and validate user
    User new_user{0, username, email, age};  // ID will be auto-generated
    
    try {
        User inserted = user_table.insert(new_user);
        std::cout << "User created with ID: " << inserted.id << std::endl;
    } catch (const UniqueConstraintViolationException& e) {
        std::cerr << "User already exists: " << e.what() << std::endl;
    }
}
```

## Future Enhancements

Planned features for the constraint validation system:

1. **Deferred Constraints**: Validate at transaction commit instead of immediately
2. **Cascade Operations**: Automatic handling of dependent records
3. **Constraint Metadata**: Runtime introspection of table constraints
4. **Performance Profiling**: Built-in metrics for constraint validation
5. **Schema Migration**: Automatic constraint updates during schema changes

## Examples

See the complete examples in:
- `constraint_validation_example.cpp` - Basic constraint validation
- `foreign_key_validation.cpp` - Advanced foreign key relationships

Build and run the examples:
```bash
./build_constraint_example.sh
spacetime publish constraint_example --bin-path constraint_validation_example.wasm
spacetime call constraint_example test_constraints
```