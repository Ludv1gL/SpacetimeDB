/**
 * BSATN C++ Test Suite
 * 
 * Comprehensive tests for Binary SpacetimeDB Algebraic Type Notation
 * Verifies compatibility with C# and Rust implementations
 */

#include <spacetimedb/bsatn/bsatn.h>
#include <spacetimedb/library/spacetimedb_library_types.h>
#include <iostream>
#include <cassert>
#include <numeric>
#include <algorithm>
#include <random>
#include <iomanip>

using namespace spacetimedb::bsatn;

// Convenience functions for testing
template<typename T>
std::vector<uint8_t> to_vec(const T& value) {
    std::vector<uint8_t> buffer;
    Writer writer(buffer);
    serialize(writer, value);
    return buffer;
}

template<typename T>
T from_vec(const std::vector<uint8_t>& data) {
    Reader reader(data.data(), data.size());
    return deserialize<T>(reader);
}

// Test configuration
constexpr bool VERBOSE = false;
constexpr int RANDOM_TEST_COUNT = 100;

// Helper macros for concise testing
#define TEST(name) if (VERBOSE) std::cout << "  " << #name << "... "; test_##name(); if (VERBOSE) std::cout << "✓\n"
#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_ROUNDTRIP(value) do { \
    auto serialized = ::spacetimedb::bsatn::to_vec(value); \
    auto result = ::spacetimedb::bsatn::from_vec<decltype(value)>(serialized); \
    ASSERT_EQ(value, result); \
} while(0)

// Test structures
struct BasicStruct {
    int32_t id;
    std::string name;
    Option<float> score;
    
    bool operator==(const BasicStruct& o) const {
        return id == o.id && name == o.name && score == o.score;
    }
};
SPACETIMEDB_BSATN_STRUCT(BasicStruct, id, name, score)

struct ComplexStruct {
    std::vector<BasicStruct> items;
    Option<std::vector<Option<int32_t>>> numbers;
    SumType<int32_t, std::string, BasicStruct> variant;
    
    bool operator==(const ComplexStruct& o) const {
        return items == o.items && numbers == o.numbers && variant == o.variant;
    }
};
SPACETIMEDB_BSATN_STRUCT(ComplexStruct, items, numbers, variant)

// Test implementations
void test_primitives() {
    // Booleans
    ASSERT_ROUNDTRIP(true);
    ASSERT_ROUNDTRIP(false);
    
    // Integers
    ASSERT_ROUNDTRIP(uint8_t(0));
    ASSERT_ROUNDTRIP(uint8_t(255));
    ASSERT_ROUNDTRIP(uint16_t(65535));
    ASSERT_ROUNDTRIP(uint32_t(4294967295U));
    ASSERT_ROUNDTRIP(uint64_t(18446744073709551615ULL));
    ASSERT_ROUNDTRIP(int8_t(-128));
    ASSERT_ROUNDTRIP(int8_t(127));
    ASSERT_ROUNDTRIP(int16_t(-32768));
    ASSERT_ROUNDTRIP(int32_t(-2147483648));
    ASSERT_ROUNDTRIP(int64_t(9223372036854775807LL));
    
    // Floating point
    ASSERT_ROUNDTRIP(0.0f);
    ASSERT_ROUNDTRIP(-0.0f);
    ASSERT_ROUNDTRIP(3.14159f);
    ASSERT_ROUNDTRIP(3.141592653589793);
    
    // Strings
    ASSERT_ROUNDTRIP(std::string(""));
    ASSERT_ROUNDTRIP(std::string("Hello"));
    ASSERT_ROUNDTRIP(std::string("Hello, 世界! 🚀"));
    ASSERT_ROUNDTRIP(std::string(1000, 'x'));  // 1KB string
}

void test_sdk_types() {
    // Timestamp
    ASSERT_ROUNDTRIP(Timestamp(0));
    ASSERT_ROUNDTRIP(Timestamp(1737582793990ULL));
    
    // Identity
    std::array<uint8_t, IDENTITY_SIZE> zero_id{};
    ASSERT_ROUNDTRIP(Identity(zero_id));
    
    std::array<uint8_t, IDENTITY_SIZE> seq_id{};
    std::iota(seq_id.begin(), seq_id.end(), 0);
    ASSERT_ROUNDTRIP(Identity(seq_id));
    
    // ConnectionId
    ASSERT_ROUNDTRIP(ConnectionId(0));
    ASSERT_ROUNDTRIP(ConnectionId(0x123456789ABCDEF0ULL));
}

void test_collections() {
    // Empty collections
    ASSERT_ROUNDTRIP(std::vector<int32_t>{});
    ASSERT_ROUNDTRIP(std::vector<std::string>{});
    
    // Simple vectors
    ASSERT_ROUNDTRIP((std::vector<int32_t>{1, 2, 3, 4, 5}));
    ASSERT_ROUNDTRIP((std::vector<std::string>{"hello", "world", ""}));
    
    // Nested vectors
    ASSERT_ROUNDTRIP((std::vector<std::vector<int32_t>>{{1, 2}, {3, 4, 5}, {}}));
    
    // Large vector
    std::vector<uint64_t> large(1000);
    std::iota(large.begin(), large.end(), 0);
    ASSERT_ROUNDTRIP(large);
}

void test_options() {
    // Basic Option types
    ASSERT_ROUNDTRIP(Option<int32_t>());
    ASSERT_ROUNDTRIP(Option<int32_t>(42));
    ASSERT_ROUNDTRIP(Option<std::string>());
    ASSERT_ROUNDTRIP(Option<std::string>("test"));
    
    // Option of collections
    ASSERT_ROUNDTRIP(Option<std::vector<int32_t>>());
    ASSERT_ROUNDTRIP((Option<std::vector<int32_t>>(std::vector<int32_t>{1, 2, 3})));
    
    // Nested Options
    std::vector<Option<int32_t>> opt_vec = {
        Option<int32_t>(1),
        Option<int32_t>(),
        Option<int32_t>(3)
    };
    ASSERT_ROUNDTRIP((Option<std::vector<Option<int32_t>>>(opt_vec)));
}

void test_structs() {
    // Empty struct
    ASSERT_ROUNDTRIP((BasicStruct{0, "", Option<float>()}));
    
    // Filled struct
    ASSERT_ROUNDTRIP((BasicStruct{42, "test", Option<float>(3.14f)}));
    
    // Complex nested struct
    ComplexStruct complex;
    complex.items = {
        {1, "first", Option<float>(1.0f)},
        {2, "second", Option<float>()}
    };
    complex.numbers = Option<std::vector<Option<int32_t>>>(
        std::vector<Option<int32_t>>{Option<int32_t>(10), Option<int32_t>()}
    );
    complex.variant = Sum<int32_t, std::string, BasicStruct>(
        BasicStruct{3, "variant", Option<float>(2.5f)}
    );
    ASSERT_ROUNDTRIP(complex);
}

void test_sum_types() {
    using Variant = Sum<int32_t, std::string, std::vector<int32_t>>;
    
    // Each variant type
    ASSERT_ROUNDTRIP(Variant(42));
    ASSERT_ROUNDTRIP(Variant(std::string("hello")));
    ASSERT_ROUNDTRIP((Variant(std::vector<int32_t>{1, 2, 3})));
}

void test_binary_format() {
    // Verify exact binary encoding matches specification
    auto check_encoding = [](auto value, std::initializer_list<uint8_t> expected) {
        auto buf = ::spacetimedb::bsatn::to_vec(value);
        assert(buf.size() == expected.size());
        assert(std::equal(buf.begin(), buf.end(), expected.begin()));
    };
    
    // Primitive encodings
    check_encoding(true, {1});
    check_encoding(false, {0});
    check_encoding(uint8_t(42), {42});
    check_encoding(uint32_t(0x12345678), {0x78, 0x56, 0x34, 0x12});  // Little-endian
    
    // Option encoding
    check_encoding(Option<uint8_t>(), {0});
    check_encoding(Option<uint8_t>(5), {1, 5});
    
    // String encoding (length prefix + UTF-8 bytes)
    check_encoding(std::string("Hi"), {2, 0, 0, 0, 'H', 'i'});
    
    // Vector encoding (length prefix + elements)
    check_encoding(std::vector<uint8_t>{1, 2, 3}, {3, 0, 0, 0, 1, 2, 3});
}

void test_error_handling() {
    // Buffer underrun
    try {
        std::vector<uint8_t> small = {1, 2};
        ::spacetimedb::bsatn::from_vec<uint64_t>(small);
        assert(false);
    } catch (...) {
        // Expected
    }
    
    // Invalid Option tag
    try {
        std::vector<uint8_t> bad = {99};  // Invalid tag
        ::spacetimedb::bsatn::from_vec<Option<int32_t>>(bad);
        assert(false);
    } catch (...) {
        // Expected
    }
}

void test_random() {
    std::mt19937 gen(42);  // Fixed seed for reproducibility
    std::uniform_int_distribution<int32_t> int_dist(-1000, 1000);
    std::uniform_real_distribution<float> float_dist(-100.0f, 100.0f);
    std::uniform_int_distribution<int> bool_dist(0, 1);
    std::uniform_int_distribution<int> size_dist(0, 20);
    
    for (int i = 0; i < RANDOM_TEST_COUNT; ++i) {
        BasicStruct s;
        s.id = int_dist(gen);
        
        // Random string
        int len = size_dist(gen);
        s.name.clear();
        for (int j = 0; j < len; ++j) {
            s.name += char('a' + (gen() % 26));
        }
        
        // Random optional
        if (bool_dist(gen)) {
            s.score = Option<float>(float_dist(gen));
        }
        
        ASSERT_ROUNDTRIP(s);
    }
}

// Main test runner
int main(int argc, char** argv) {
    bool verbose = argc > 1 && std::string(argv[1]) == "-v";
    if (verbose) const_cast<bool&>(VERBOSE) = true;
    
    std::cout << "BSATN C++ Test Suite\n";
    std::cout << "====================\n\n";
    
    try {
        if (VERBOSE) std::cout << "Testing primitives:\n";
        TEST(primitives);
        
        if (VERBOSE) std::cout << "\nTesting SDK types:\n";
        TEST(sdk_types);
        
        if (VERBOSE) std::cout << "\nTesting collections:\n";
        TEST(collections);
        
        if (VERBOSE) std::cout << "\nTesting Option types:\n";
        TEST(options);
        
        if (VERBOSE) std::cout << "\nTesting structs:\n";
        TEST(structs);
        
        if (VERBOSE) std::cout << "\nTesting sum types:\n";
        TEST(sum_types);
        
        if (VERBOSE) std::cout << "\nTesting binary format:\n";
        TEST(binary_format);
        
        if (VERBOSE) std::cout << "\nTesting error handling:\n";
        TEST(error_handling);
        
        if (VERBOSE) std::cout << "\nTesting random data:\n";
        TEST(random);
        
        std::cout << "\n✅ All tests passed!\n\n";
        
        if (!VERBOSE) {
            std::cout << "Test Summary:\n";
            std::cout << "• Primitives (bool, int, float, string) ✓\n";
            std::cout << "• SDK types (Identity, Timestamp, ConnectionId) ✓\n";
            std::cout << "• Collections (vector, nested) ✓\n";
            std::cout << "• Option types (nullable) ✓\n";
            std::cout << "• Structs (SPACETIMEDB_BSATN_STRUCT) ✓\n";
            std::cout << "• Sum types (discriminated unions) ✓\n";
            std::cout << "• Binary format compliance ✓\n";
            std::cout << "• Error handling ✓\n";
            std::cout << "• " << RANDOM_TEST_COUNT << " random test cases ✓\n";
            std::cout << "\nRun with -v for verbose output.\n";
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << "\n";
        return 1;
    }
}