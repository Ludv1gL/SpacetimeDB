#include <spacetimedb/spacetimedb_easy.h>
#include <spacetimedb/bsatn/bsatn.h>

// Test struct for product types
struct PersonData {
    uint32_t id;
    std::string name;
    uint8_t age;
};

// Test enum for sum types  
enum class StatusCode : uint8_t {
    Success = 0,
    Failed = 1,
    Pending = 2
};

// Test struct with optional fields
struct OptionalData {
    uint32_t required_id;
    SpacetimeDb::bsatn::Option<std::string> optional_name;
    SpacetimeDb::bsatn::Option<uint32_t> optional_value;
};

// Test sum type
using TestResult = SpacetimeDb::bsatn::Sum<std::string, uint32_t, bool>;

// Test collection types
struct CollectionData {
    std::vector<uint32_t> numbers;
    std::vector<std::string> names;
    std::vector<PersonData> people;  // Nested collections
};

// Declare tables to test schema generation
SPACETIMEDB_TABLE(PersonData, person_data, true)
SPACETIMEDB_TABLE(OptionalData, optional_data, true)
SPACETIMEDB_TABLE(CollectionData, collection_data, true)

// Test reducers for algebraic types
SPACETIMEDB_REDUCER(test_product_types, spacetimedb::ReducerContext ctx, uint32_t id, std::string name, uint8_t age) {
    LOG_INFO("Testing product types (structs)");
    
    PersonData person{id, name, age};
    ctx.db.table<PersonData>("person_data").insert(person);
    
    LOG_INFO("Product type inserted successfully");
}

SPACETIMEDB_REDUCER(test_optional_types, spacetimedb::ReducerContext ctx, uint32_t id, bool has_name, std::string name_value, bool has_value, uint32_t optional_value) {
    LOG_INFO("Testing optional/nullable types");
    
    OptionalData data;
    data.required_id = id;
    
    // Test Option<T> assignment
    if (has_name) {
        data.optional_name = SpacetimeDb::bsatn::Option<std::string>(name_value);
    } else {
        data.optional_name = SpacetimeDb::bsatn::Option<std::string>();  // None
    }
    
    if (has_value) {
        data.optional_value = SpacetimeDb::bsatn::Option<uint32_t>(optional_value);
    } else {
        data.optional_value = SpacetimeDb::bsatn::Option<uint32_t>();  // None
    }
    
    ctx.db.table<OptionalData>("optional_data").insert(data);
    
    LOG_INFO("Optional types inserted successfully");
}

SPACETIMEDB_REDUCER(test_collection_types, spacetimedb::ReducerContext ctx, uint32_t count) {
    LOG_INFO("Testing collection types (arrays/vectors)");
    
    CollectionData collections;
    
    // Fill vectors with test data
    for (uint32_t i = 0; i < count; ++i) {
        collections.numbers.push_back(i * 10);
        collections.names.push_back("Name_" + std::to_string(i));
        collections.people.push_back({i, "Person_" + std::to_string(i), static_cast<uint8_t>(20 + i)});
    }
    
    ctx.db.table<CollectionData>("collection_data").insert(collections);
    
    LOG_INFO("Collection types inserted successfully");
}

SPACETIMEDB_REDUCER(test_sum_types, spacetimedb::ReducerContext ctx, uint8_t variant_type) {
    LOG_INFO("Testing sum types (discriminated unions)");
    
    TestResult result;
    
    switch (variant_type) {
        case 0:
            result = std::string("Success message");
            LOG_INFO("Created string variant");
            break;
        case 1:
            result = uint32_t(42);
            LOG_INFO("Created uint32_t variant");
            break;
        case 2:
            result = true;
            LOG_INFO("Created bool variant");
            break;
        default:
            result = std::string("Unknown variant");
            LOG_WARN("Unknown variant type");
            break;
    }
    
    // Log which variant we have
    LOG_INFO("Sum type tag: " + std::to_string(result.tag()));
    
    // Visit the sum type
    result.visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            LOG_INFO("String variant: " + value);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            LOG_INFO("Uint32 variant: " + std::to_string(value));
        } else if constexpr (std::is_same_v<T, bool>) {
            LOG_INFO("Bool variant: " + std::string(value ? "true" : "false"));
        }
    });
    
    LOG_INFO("Sum types test completed successfully");
}

SPACETIMEDB_REDUCER(init_algebraic_test, spacetimedb::ReducerContext ctx) {
    SpacetimeDB::LogStopwatch timer("algebraic_test_init");
    LOG_INFO("Initializing algebraic types test database");
    
    // Initialize tables if needed
    LOG_INFO("Algebraic types test database initialized successfully");
}