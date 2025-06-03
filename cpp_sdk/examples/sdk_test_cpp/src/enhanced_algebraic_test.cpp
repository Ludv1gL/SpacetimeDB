#include <spacetimedb/spacetimedb_easy.h>
#include <spacetimedb/bsatn/bsatn.h>

// Test struct for product types
struct PersonData {
    uint32_t id;
    std::string name;
    uint8_t age;
};

// Manual BSATN specialization for PersonData
namespace SpacetimeDb::bsatn {
    template<>
    struct bsatn_traits<PersonData> {
        static void serialize(SpacetimeDb::bsatn::Writer& writer, const PersonData& value) {
            // Serialize fields in order: id, name, age
            SpacetimeDb::bsatn::serialize(writer, value.id);
            SpacetimeDb::bsatn::serialize(writer, value.name); 
            SpacetimeDb::bsatn::serialize(writer, value.age);
        }
        
        static PersonData deserialize(SpacetimeDb::bsatn::Reader& reader) {
            PersonData result;
            result.id = SpacetimeDb::bsatn::deserialize<uint32_t>(reader);
            result.name = SpacetimeDb::bsatn::deserialize<std::string>(reader);
            result.age = SpacetimeDb::bsatn::deserialize<uint8_t>(reader);
            return result;
        }
        
        static SpacetimeDb::bsatn::AlgebraicType algebraic_type() {
            std::vector<SpacetimeDb::bsatn::AggregateElement> elements;
            elements.emplace_back("id", 0);    // Will be replaced with proper type registry
            elements.emplace_back("name", 0);  // Will be replaced with proper type registry  
            elements.emplace_back("age", 0);   // Will be replaced with proper type registry
            return SpacetimeDb::bsatn::AlgebraicType::make_product(
                std::make_unique<SpacetimeDb::bsatn::ProductType>(std::move(elements))
            );
        }
    };
}

// Declare table
SPACETIMEDB_TABLE(PersonData, person_data, true)

// Test reducers
SPACETIMEDB_REDUCER(test_enhanced_struct, spacetimedb::ReducerContext ctx, uint32_t id, std::string name, uint8_t age) {
    LOG_INFO("Testing enhanced struct with manual BSATN serialization");
    
    PersonData person{id, name, age};
    ctx.db.table<PersonData>("person_data").insert(person);
    
    LOG_INFO("Enhanced struct inserted successfully");
}

SPACETIMEDB_REDUCER(test_option_types, spacetimedb::ReducerContext ctx, uint32_t id, bool has_name, std::string name_value) {
    LOG_INFO("Testing Option<T> types");
    
    SpacetimeDb::bsatn::Option<std::string> optional_name;
    
    if (has_name) {
        optional_name = SpacetimeDb::bsatn::Option<std::string>(name_value);
        LOG_INFO("Created Some(\"" + name_value + "\")");
    } else {
        optional_name = SpacetimeDb::bsatn::Option<std::string>();
        LOG_INFO("Created None");
    }
    
    // Test the option
    if (optional_name.has_value()) {
        LOG_INFO("Option has value: " + optional_name.value());
    } else {
        LOG_INFO("Option is None");
    }
    
    LOG_INFO("Option types test completed successfully");
}

SPACETIMEDB_REDUCER(test_sum_types, spacetimedb::ReducerContext ctx, uint8_t variant_type) {
    LOG_INFO("Testing Sum types");
    
    // Test Sum<string, uint32_t, bool>
    SpacetimeDb::bsatn::Sum<std::string, uint32_t, bool> test_sum;
    
    switch (variant_type) {
        case 0:
            test_sum = std::string("Hello Sum Types!");
            LOG_INFO("Created string variant with tag: " + std::to_string(test_sum.tag()));
            break;
        case 1:
            test_sum = uint32_t(42);
            LOG_INFO("Created uint32_t variant with tag: " + std::to_string(test_sum.tag()));
            break;
        case 2:
            test_sum = true;
            LOG_INFO("Created bool variant with tag: " + std::to_string(test_sum.tag()));
            break;
        default:
            test_sum = std::string("Default");
            LOG_WARN("Unknown variant, using default");
            break;
    }
    
    // Visit the sum type to demonstrate pattern matching
    test_sum.visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            LOG_INFO("Visiting string: " + value);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            LOG_INFO("Visiting uint32_t: " + std::to_string(value));
        } else if constexpr (std::is_same_v<T, bool>) {
            LOG_INFO("Visiting bool: " + std::string(value ? "true" : "false"));
        }
    });
    
    LOG_INFO("Sum types test completed successfully");
}

SPACETIMEDB_REDUCER(test_vector_types, spacetimedb::ReducerContext ctx, uint32_t count) {
    LOG_INFO("Testing std::vector serialization");
    
    std::vector<uint32_t> numbers;
    std::vector<std::string> names;
    
    for (uint32_t i = 0; i < count; ++i) {
        numbers.push_back(i * 10);
        names.push_back("Item_" + std::to_string(i));
    }
    
    LOG_INFO("Created vector<uint32_t> with " + std::to_string(numbers.size()) + " elements");
    LOG_INFO("Created vector<string> with " + std::to_string(names.size()) + " elements");
    
    // Test BSATN serialization directly
    try {
        SpacetimeDb::bsatn::Writer writer;
        SpacetimeDb::bsatn::serialize(writer, numbers);
        LOG_INFO("std::vector<uint32_t> serialization successful");
        
        SpacetimeDb::bsatn::Writer writer2;
        SpacetimeDb::bsatn::serialize(writer2, names);
        LOG_INFO("std::vector<std::string> serialization successful");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Vector serialization failed: " + std::string(e.what()));
    }
    
    LOG_INFO("Vector types test completed");
}

SPACETIMEDB_REDUCER(init_enhanced_test, spacetimedb::ReducerContext ctx) {
    SpacetimeDB::LogStopwatch timer("enhanced_algebraic_test_init");
    LOG_INFO("Initializing enhanced algebraic types test database");
    
    LOG_INFO("Enhanced algebraic types test database initialized successfully");
}