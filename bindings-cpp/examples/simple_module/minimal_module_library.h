#ifndef MINIMAL_SDK_H
#define MINIMAL_SDK_H

#include <cstdint>
#include <vector>
#include <string>
#include <functional>

// Forward declarations to avoid conflicts
namespace SpacetimeDb {
    class ReducerContext;
}

// Simple BSATN writer
class MinimalWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t v) { buffer.push_back(v); }
    void write_u16_le(uint16_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
    }
    void write_u32_le(uint32_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
        buffer.push_back((v >> 16) & 0xFF);
        buffer.push_back((v >> 24) & 0xFF);
    }
    void write_string(const std::string& s) {
        write_u32_le(s.length());
        for (char c : s) buffer.push_back(c);
    }
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
};

// Simple ReducerContext
namespace SpacetimeDb {
    class ReducerContext {
    public:
        // Minimal implementation for testing
    };
}

// Simple table registration
template<typename T>
std::vector<uint8_t> generate_type_description() {
    MinimalWriter w;
    w.write_u8(2); // Product type
    w.write_u32_le(3); // 3 fields for Person
    
    // Field 1: id
    w.write_u8(0);
    w.write_string("id");
    w.write_u8(11); // U32
    
    // Field 2: name
    w.write_u8(0);
    w.write_string("name");
    w.write_u8(4); // String
    
    // Field 3: age
    w.write_u8(0);
    w.write_string("age");
    w.write_u8(7); // U8
    
    return w.take_buffer();
}

// Simple table handle
template<typename T>
class TableHandle {
public:
    T insert(const T& row) {
        // TODO: Implement actual insert
        return row;
    }
    
    size_t count() {
        // TODO: Implement actual count
        return 0;
    }
    
    // Simple iterator placeholder
    class iterator {
    public:
        iterator() = default;
        bool operator!=(const iterator& other) const { return false; }
        iterator& operator++() { return *this; }
        T operator*() const { return T{}; }
    };
    
    iterator begin() { return iterator{}; }
    iterator end() { return iterator{}; }
    
    // Range iteration placeholder
    struct iter_result {
        iterator begin() { return iterator{}; }
        iterator end() { return iterator{}; }
    };
    
    iter_result iter() { return iter_result{}; }
};

// Logging function
void LOG_INFO(const std::string& msg);

// Macro to register a table (simplified)
#define SPACETIMEDB_TABLE(Type, name, is_public) \
    using name##__TableHandle = TableHandle<Type>; \
    static name##__TableHandle name##_table;

// Macro to register a reducer (simplified)
#define SPACETIMEDB_REDUCER(name, ctx_param) \
    void name##_impl(const SpacetimeDb::ReducerContext& ctx_param); \
    void name##_impl(const SpacetimeDb::ReducerContext& ctx_param)

#endif // MINIMAL_SDK_H