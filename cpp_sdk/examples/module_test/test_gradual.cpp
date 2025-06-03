// Test module with gradually increasing complexity
#define SPACETIMEDB_TABLES_LIST \
    X(SimpleStruct, simple_struct, true) \
    X(ComplexStruct, complex_struct, true)

#include <spacetimedb/spacetimedb.h>
#include <string>
#include <vector>

using namespace spacetimedb;

// Simple struct with basic types
struct SimpleStruct {
    uint32_t id;
    std::string name;
    uint8_t value;
};

// More complex struct
struct ComplexStruct {
    uint64_t id;
    std::string text;
    std::vector<uint8_t> data;  // This might be the issue
};

SPACETIMEDB_REDUCER(add_simple, ReducerContext ctx, uint32_t id, std::string name, uint8_t value) {
    SimpleStruct row{id, name, value};
    ctx.db.simple_struct().insert(row);
}

SPACETIMEDB_REDUCER(add_complex, ReducerContext ctx, uint64_t id, std::string text) {
    std::vector<uint8_t> data{1, 2, 3};
    ComplexStruct row{id, text, data};
    ctx.db.complex_struct().insert(row);
}