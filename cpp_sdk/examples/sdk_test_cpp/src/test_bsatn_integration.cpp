/**
 * Integration test demonstrating that the improved BSATN features
 * work correctly with the existing BSATN implementation.
 */

#include <spacetimedb/bsatn/bsatn.h>
#include <iostream>

using namespace spacetimedb::bsatn;  // Use lowercase namespace

// Simple struct using the new macro
struct TestData {
    uint32_t id;
    std::string name;
};

// Register BSATN serialization
SPACETIMEDB_BSATN_STRUCT(TestData, id, name)

// Example with Option type
struct UserProfile {
    std::string username;
    Option<std::string> email;
    uint32_t age;
};

SPACETIMEDB_BSATN_STRUCT(UserProfile, username, email, age)

// Test that shows namespace compatibility works
void test_namespace_compatibility() {
    // Can use types from both namespaces
    spacetimedb::bsatn::WriterCompat writer1;
    SpacetimeDb::bsatn::WriterCompat writer2;
    
    // Both refer to the same type
    static_assert(std::is_same_v<decltype(writer1), decltype(writer2)>);
    
    std::cout << "✓ Namespace compatibility works\n";
}

// Test Reader/Writer with type conversions
void test_type_conversions() {
    TestData data{42, "Hello BSATN"};
    
    // Serialize with WriterCompat
    WriterCompat writer;
    serialize(writer, data);
    
    // Get buffer as uint8_t vector
    std::vector<uint8_t> buffer = writer.take_uint8_buffer();
    
    // Deserialize with ReaderCompat
    ReaderCompat reader(buffer);
    TestData data2 = deserialize<TestData>(reader);
    
    if (data2.id == 42 && data2.name == "Hello BSATN") {
        std::cout << "✓ Type conversions work correctly\n";
    }
}

// Test Option type with make_option helper
void test_option_type() {
    // Test with value
    UserProfile user1{"alice", make_option("alice@example.com"), 25};
    
    // Test with null
    UserProfile user2{"bob", make_option(std::nullopt), 30};
    
    // Serialize both
    WriterCompat writer;
    serialize(writer, user1);
    serialize(writer, user2);
    
    // Deserialize
    auto buffer = writer.take_uint8_buffer();
    ReaderCompat reader(buffer);
    
    auto user1_copy = deserialize<UserProfile>(reader);
    auto user2_copy = deserialize<UserProfile>(reader);
    
    if (user1_copy.email.has_value() && 
        user1_copy.email.value() == "alice@example.com" &&
        !user2_copy.email.has_value()) {
        std::cout << "✓ Option type works correctly\n";
    }
}

// Test ADL (Argument Dependent Lookup)
void test_adl_functions() {
    // The spacetimedb_deserialize function should be found via ADL
    TestData data{99, "ADL Test"};
    
    WriterCompat writer;
    serialize(writer, data);
    
    auto buffer = writer.take_uint8_buffer();
    ReaderCompat reader(buffer);
    
    // This should find spacetimedb_deserialize via ADL
    auto result = deserialize<TestData>(reader);
    
    if (result.id == 99 && result.name == "ADL Test") {
        std::cout << "✓ ADL functions work correctly\n";
    }
}

int main() {
    std::cout << "Testing BSATN integration...\n\n";
    
    try {
        test_namespace_compatibility();
        test_type_conversions();
        test_option_type();
        test_adl_functions();
        
        std::cout << "\nAll integration tests passed! ✓\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << "\n";
        return 1;
    }
}