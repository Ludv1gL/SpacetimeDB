/**
 * Test program to verify the refactored BSATN implementation
 */

#include <spacetimedb/bsatn/bsatn.h>
#include "spacetimedb/types.h"
#include <iostream>
#include <cassert>

using namespace SpacetimeDb::bsatn;

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

// Test struct
struct TestStruct {
    int32_t id;
    std::string name;
    Option<float> value;
    
    bool operator==(const TestStruct& o) const {
        return id == o.id && name == o.name && value == o.value;
    }
};
SPACETIMEDB_BSATN_STRUCT(TestStruct, id, name, value)

void test_basic_serialization() {
    std::cout << "Testing basic serialization..." << std::endl;
    
    // Test primitives
    {
        auto data = to_vec(uint32_t(42));
        auto result = from_vec<uint32_t>(data);
        assert(result == 42);
    }
    
    // Test string
    {
        std::string str = "Hello, BSATN!";
        auto data = to_vec(str);
        auto result = from_vec<std::string>(data);
        assert(result == str);
    }
    
    // Test Option
    {
        Option<int32_t> some(42);
        auto data = to_vec(some);
        auto result = from_vec<Option<int32_t>>(data);
        assert(result.has_value() && result.value() == 42);
        
        Option<int32_t> none;
        data = to_vec(none);
        result = from_vec<Option<int32_t>>(data);
        assert(!result.has_value());
    }
    
    // Test struct
    {
        TestStruct s{123, "test", Option<float>(3.14f)};
        auto data = to_vec(s);
        auto result = from_vec<TestStruct>(data);
        assert(result == s);
    }
    
    // Test SDK types
    {
        Identity id(std::array<uint8_t, IDENTITY_SIZE>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
        auto data = to_vec(id);
        auto result = from_vec<Identity>(data);
        assert(result == id);
        
        Timestamp ts(1234567890);
        data = to_vec(ts);
        auto ts_result = from_vec<Timestamp>(data);
        assert(ts_result == ts);
        
        ConnectionId conn(0xDEADBEEF);
        data = to_vec(conn);
        auto conn_result = from_vec<ConnectionId>(data);
        assert(conn_result == conn);
    }
    
    std::cout << "✅ All basic serialization tests passed!" << std::endl;
}

void test_sum_types() {
    std::cout << "Testing sum types..." << std::endl;
    
    using TestSum = Sum<int32_t, std::string, TestStruct>;
    
    // Test each variant
    {
        TestSum v1(42);
        auto data = to_vec(v1);
        auto result = from_vec<TestSum>(data);
        assert(result.is<int32_t>() && result.get<int32_t>() == 42);
    }
    
    {
        TestSum v2(std::string("hello"));
        auto data = to_vec(v2);
        auto result = from_vec<TestSum>(data);
        assert(result.is<std::string>() && result.get<std::string>() == "hello");
    }
    
    {
        TestSum v3(TestStruct{99, "sum", Option<float>(2.71f)});
        auto data = to_vec(v3);
        auto result = from_vec<TestSum>(data);
        assert(result.is<TestStruct>() && result.get<TestStruct>().id == 99);
    }
    
    std::cout << "✅ All sum type tests passed!" << std::endl;
}

void test_collections() {
    std::cout << "Testing collections..." << std::endl;
    
    // Vector of primitives
    {
        std::vector<int32_t> vec = {1, 2, 3, 4, 5};
        auto data = to_vec(vec);
        auto result = from_vec<std::vector<int32_t>>(data);
        assert(result == vec);
    }
    
    // Vector of structs
    {
        std::vector<TestStruct> vec = {
            {1, "first", Option<float>(1.0f)},
            {2, "second", Option<float>()},
            {3, "third", Option<float>(3.0f)}
        };
        auto data = to_vec(vec);
        auto result = from_vec<std::vector<TestStruct>>(data);
        assert(result == vec);
    }
    
    // Nested vectors
    {
        std::vector<std::vector<int32_t>> nested = {{1, 2}, {3, 4, 5}, {}};
        auto data = to_vec(nested);
        auto result = from_vec<std::vector<std::vector<int32_t>>>(data);
        assert(result == nested);
    }
    
    std::cout << "✅ All collection tests passed!" << std::endl;
}

void test_binary_format() {
    std::cout << "Testing binary format compliance..." << std::endl;
    
    // Check exact encoding
    {
        auto data = to_vec(uint32_t(0x12345678));
        assert(data.size() == 4);
        assert(data[0] == 0x78);  // Little-endian
        assert(data[1] == 0x56);
        assert(data[2] == 0x34);
        assert(data[3] == 0x12);
    }
    
    {
        auto data = to_vec(std::string("Hi"));
        assert(data.size() == 6);
        assert(data[0] == 2);  // Length (little-endian)
        assert(data[1] == 0);
        assert(data[2] == 0);
        assert(data[3] == 0);
        assert(data[4] == 'H');
        assert(data[5] == 'i');
    }
    
    std::cout << "✅ Binary format tests passed!" << std::endl;
}

int main() {
    std::cout << "=== Refactored BSATN Test Suite ===" << std::endl;
    std::cout << std::endl;
    
    try {
        test_basic_serialization();
        test_sum_types();
        test_collections();
        test_binary_format();
        
        std::cout << std::endl;
        std::cout << "🎉 All tests passed! The refactored BSATN implementation is working correctly." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
}