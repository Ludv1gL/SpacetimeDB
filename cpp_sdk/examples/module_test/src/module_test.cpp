// Define all tables using X-Macro pattern
#define SPACETIMEDB_TABLES_LIST \
    X(Person, person, true) \
    X(TestA, test_a, true) \
    X(TestB, test_b, false) \
    X(TestC, test_c, false) \
    X(TestD, test_d, false) \
    X(Point, point, true) \
    X(Player, player, true) \
    X(SimpleTable, simple_table, false) \
    X(EveryPrimitiveStruct, every_primitive_struct, false) \
    X(EveryVecStruct, every_vec_struct, false)

#include <spacetimedb/spacetimedb.h>
#include <optional>
#include <vector>
#include <string>
#include <cstdint>

using namespace spacetimedb;
using SpacetimeDb::sdk::Identity;
using SpacetimeDb::sdk::ConnectionId;
using SpacetimeDb::sdk::Timestamp;
using uint128_t = SpacetimeDb::Types::uint128_t_placeholder;
using uint256_t = SpacetimeDb::Types::uint256_t_placeholder;
using int128_t = SpacetimeDb::Types::int128_t_placeholder;
using int256_t = SpacetimeDb::Types::int256_t_placeholder;

// Enum types from Rust/C#
enum class TestE : uint8_t {
    One = 0,
    Two = 1,
    Three = 2,
    MinusOne = 255
};

// Struct for complex enum variant
struct Complex {
    std::string txt;
    uint32_t num;
};

// Sum type enum (like Rust TestF)
struct TestFOne {};
struct TestFTwo { std::string _0; };
struct TestFThree { std::string _0; uint32_t _1; };
struct TestFMinusOne {};

// Tagged union for TestF
struct TestF {
    enum Tag { One, Two, Three, MinusOne };
    Tag tag;
    union {
        TestFOne one;
        TestFTwo two;
        TestFThree three;
        TestFMinusOne minus_one;
    };
};

// Enum for Foobar (like Rust)
enum class Foobar : uint8_t {
    Bar = 0,
    Baz = 1
};

// Table structs from Rust/C#
struct Person {
    uint32_t id;  // primary_key, auto_inc
    std::string name;
    uint8_t age;  // indexed with BTree
};

struct TestA {
    uint64_t x;
    uint32_t y;
    uint16_t z;
};

struct TestB {
    Identity id;
    std::string name;
    TestA data;
};

struct TestC {
    TestE x;
    TestE y;
    TestE z;
};

struct TestD {
    Identity id;
    std::optional<std::string> name;
};

struct Point {
    int64_t x;
    int64_t y;
};

struct Player {
    uint32_t id;  // primary_key, auto_inc
    Identity owner;  // unique index
    Point location;
    std::string name;
    ConnectionId connection;
    Timestamp created_at;
};

struct SimpleTable {
    Identity id;  // primary_key
    uint8_t data;
};

struct EveryPrimitiveStruct {
    Identity id;  // primary_key
    uint8_t a_u8;
    uint16_t a_u16;
    uint32_t a_u32;
    uint64_t a_u64;
    uint128_t a_u128;
    uint256_t a_u256;
    int8_t a_i8;
    int16_t a_i16;
    int32_t a_i32;
    int64_t a_i64;
    int128_t a_i128;
    int256_t a_i256;
    bool a_bool;
    float a_f32;
    double a_f64;
    std::string a_string;
    std::vector<uint8_t> a_bytes;
};

struct EveryVecStruct {
    Identity id;  // primary_key
    std::vector<uint8_t> a_u8;
    std::vector<uint16_t> a_u16;
    std::vector<uint32_t> a_u32;
    std::vector<uint64_t> a_u64;
    std::vector<uint128_t> a_u128;
    std::vector<uint256_t> a_u256;
    std::vector<int8_t> a_i8;
    std::vector<int16_t> a_i16;
    std::vector<int32_t> a_i32;
    std::vector<int64_t> a_i64;
    std::vector<int128_t> a_i128;
    std::vector<int256_t> a_i256;
    std::vector<bool> a_bool;
    std::vector<float> a_f32;
    std::vector<double> a_f64;
    std::vector<std::string> a_string;
    std::vector<SimpleTable> a_simple_table;
    std::vector<EveryPrimitiveStruct> a_every_primitive_struct;
};

// Tables are automatically registered from the X-macro list above

// Register fields for all structs
SPACETIMEDB_REGISTER_FIELDS(Person,
    SPACETIMEDB_FIELD(Person, id, uint32_t);
    SPACETIMEDB_FIELD(Person, name, std::string);
    SPACETIMEDB_FIELD(Person, age, uint8_t);
)

SPACETIMEDB_REGISTER_FIELDS(TestA,
    SPACETIMEDB_FIELD(TestA, x, uint64_t);
    SPACETIMEDB_FIELD(TestA, y, uint32_t);
    SPACETIMEDB_FIELD(TestA, z, uint16_t);
)

SPACETIMEDB_REGISTER_FIELDS(TestB,
    SPACETIMEDB_FIELD(TestB, id, Identity);
    SPACETIMEDB_FIELD(TestB, name, std::string);
    SPACETIMEDB_FIELD(TestB, data, TestA);
)

SPACETIMEDB_REGISTER_FIELDS(TestC,
    SPACETIMEDB_FIELD(TestC, x, TestE);
    SPACETIMEDB_FIELD(TestC, y, TestE);
    SPACETIMEDB_FIELD(TestC, z, TestE);
)

SPACETIMEDB_REGISTER_FIELDS(TestD,
    SPACETIMEDB_FIELD(TestD, id, Identity);
    SPACETIMEDB_FIELD(TestD, name, std::optional<std::string>);
)

SPACETIMEDB_REGISTER_FIELDS(Point,
    SPACETIMEDB_FIELD(Point, x, int64_t);
    SPACETIMEDB_FIELD(Point, y, int64_t);
)

SPACETIMEDB_REGISTER_FIELDS(Player,
    SPACETIMEDB_FIELD(Player, id, uint32_t);
    SPACETIMEDB_FIELD(Player, owner, Identity);
    SPACETIMEDB_FIELD(Player, location, Point);
    SPACETIMEDB_FIELD(Player, name, std::string);
    SPACETIMEDB_FIELD(Player, connection, ConnectionId);
    SPACETIMEDB_FIELD(Player, created_at, Timestamp);
)

SPACETIMEDB_REGISTER_FIELDS(SimpleTable,
    SPACETIMEDB_FIELD(SimpleTable, id, Identity);
    SPACETIMEDB_FIELD(SimpleTable, data, uint8_t);
)

SPACETIMEDB_REGISTER_FIELDS(EveryPrimitiveStruct,
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, id, Identity);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_u8, uint8_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_u16, uint16_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_u32, uint32_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_u64, uint64_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_u128, uint128_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_u256, uint256_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_i8, int8_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_i16, int16_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_i32, int32_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_i64, int64_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_i128, int128_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_i256, int256_t);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_bool, bool);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_f32, float);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_f64, double);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_string, std::string);
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a_bytes, std::vector<uint8_t>);
)

SPACETIMEDB_REGISTER_FIELDS(EveryVecStruct,
    SPACETIMEDB_FIELD(EveryVecStruct, id, Identity);
    SPACETIMEDB_FIELD(EveryVecStruct, a_u8, std::vector<uint8_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_u16, std::vector<uint16_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_u32, std::vector<uint32_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_u64, std::vector<uint64_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_u128, std::vector<uint128_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_u256, std::vector<uint256_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_i8, std::vector<int8_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_i16, std::vector<int16_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_i32, std::vector<int32_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_i64, std::vector<int64_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_i128, std::vector<int128_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_i256, std::vector<int256_t>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_bool, std::vector<bool>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_f32, std::vector<float>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_f64, std::vector<double>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_string, std::vector<std::string>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_simple_table, std::vector<SimpleTable>);
    SPACETIMEDB_FIELD(EveryVecStruct, a_every_primitive_struct, std::vector<EveryPrimitiveStruct>);
)

// Reducers from Rust/C#

// Testing various insert patterns
SPACETIMEDB_REDUCER(insert_person, ReducerContext ctx, std::string name, uint8_t age) {
    Person person{0, name, age};  // id will be auto-incremented
    ctx.db.person().insert(person);
}

SPACETIMEDB_REDUCER(insert_test_a, ReducerContext ctx, uint64_t x, uint32_t y, uint16_t z) {
    TestA test{x, y, z};
    ctx.db.test_a().insert(test);
}

SPACETIMEDB_REDUCER(insert_test_b, ReducerContext ctx, std::string name, TestA data) {
    TestB test{Identity(), name, data};
    ctx.db.test_b().insert(test);
}

SPACETIMEDB_REDUCER(insert_test_c, ReducerContext ctx, TestE x, TestE y, TestE z) {
    TestC test{x, y, z};
    ctx.db.test_c().insert(test);
}

SPACETIMEDB_REDUCER(insert_test_d, ReducerContext ctx, std::optional<std::string> name) {
    TestD test{Identity(), name};
    ctx.db.test_d().insert(test);
}

SPACETIMEDB_REDUCER(insert_player, ReducerContext ctx, std::string name, int64_t x, int64_t y) {
    Player player{
        0,  // auto-inc id
        ctx.sender,  // owner
        Point{x, y},
        name,
        ctx.connection_id,  // connection
        ctx.timestamp  // created_at
    };
    ctx.db.player().insert(player);
}

// Query reducers - TODO: Implement when select_all() is available
SPACETIMEDB_REDUCER(query_person_by_name, ReducerContext ctx, std::string name) {
    LOG_INFO("Query person by name: " + name + " (select_all not yet implemented)");
}

SPACETIMEDB_REDUCER(query_players_by_owner, ReducerContext ctx) {
    LOG_INFO("Query players by owner (select_all not yet implemented)");
}

// Delete reducers - TODO: Implement when remove() is available
SPACETIMEDB_REDUCER(delete_person_by_id, ReducerContext ctx, uint32_t id) {
    LOG_INFO("Delete person by id: " + std::to_string(id) + " (remove not yet implemented)");
}

SPACETIMEDB_REDUCER(delete_players_by_owner, ReducerContext ctx) {
    LOG_INFO("Delete players by owner (remove not yet implemented)");
}

// Testing every primitive type
SPACETIMEDB_REDUCER(insert_every_primitive, ReducerContext ctx, std::string data) {
    EveryPrimitiveStruct every{
        Identity(),
        1,    // u8
        2,    // u16
        3,    // u32
        4,    // u64
        uint128_t{5},   // u128
        uint256_t{6},   // u256
        -1,   // i8
        -2,   // i16
        -3,   // i32
        -4,   // i64
        int128_t{0, -5},   // i128 (low, high)
        int256_t{0, 0, 0, -6},   // i256 (d0, d1, d2, d3)
        true, // bool
        1.5f, // f32
        2.5,  // f64
        data, // string
        std::vector<uint8_t>{1, 2, 3}  // bytes
    };
    ctx.db.every_primitive_struct().insert(every);
}

// Testing vector types
SPACETIMEDB_REDUCER(insert_every_vec, ReducerContext ctx) {
    EveryVecStruct every{
        Identity(),
        std::vector<uint8_t>{1, 2},
        std::vector<uint16_t>{3, 4},
        std::vector<uint32_t>{5, 6},
        std::vector<uint64_t>{7, 8},
        std::vector<uint128_t>{uint128_t{9}, uint128_t{10}},
        std::vector<uint256_t>{uint256_t{11}, uint256_t{12}},
        std::vector<int8_t>{-1, -2},
        std::vector<int16_t>{-3, -4},
        std::vector<int32_t>{-5, -6},
        std::vector<int64_t>{-7, -8},
        std::vector<int128_t>{int128_t{0, -9}, int128_t{0, -10}},
        std::vector<int256_t>{int256_t{0, 0, 0, -11}, int256_t{0, 0, 0, -12}},
        std::vector<bool>{true, false},
        std::vector<float>{1.5f, 2.5f},
        std::vector<double>{3.5, 4.5},
        std::vector<std::string>{"hello", "world"},
        std::vector<SimpleTable>{{Identity(), 42}},
        std::vector<EveryPrimitiveStruct>{}  // empty for now
    };
    ctx.db.every_vec_struct().insert(every);
}

// Clear tables reducer - TODO: Implement when remove() is available
SPACETIMEDB_REDUCER(clear_all_tables, ReducerContext ctx) {
    LOG_INFO("Clear all tables (remove not yet implemented)");
}

// Testing reducer with all argument types
SPACETIMEDB_REDUCER(test_all_types, ReducerContext ctx,
    uint8_t a_u8, uint16_t a_u16, uint32_t a_u32, uint64_t a_u64,
    int8_t a_i8, int16_t a_i16, int32_t a_i32, int64_t a_i64,
    bool a_bool, float a_f32, double a_f64,
    std::string a_string, std::vector<uint8_t> a_bytes) {
    
    LOG_INFO("Received all types:");
    LOG_INFO("  u8=" + std::to_string(a_u8) + ", u16=" + std::to_string(a_u16) + 
             ", u32=" + std::to_string(a_u32) + ", u64=" + std::to_string(a_u64));
    LOG_INFO("  i8=" + std::to_string(a_i8) + ", i16=" + std::to_string(a_i16) + 
             ", i32=" + std::to_string(a_i32) + ", i64=" + std::to_string(a_i64));
    LOG_INFO("  bool=" + std::to_string(a_bool) + ", f32=" + std::to_string(a_f32) + 
             ", f64=" + std::to_string(a_f64));
    LOG_INFO("  string='" + a_string + "', bytes.size=" + std::to_string(a_bytes.size()));
}

// Special reducers
SPACETIMEDB_REDUCER(test_connection_info, ReducerContext ctx) {
    LOG_INFO("Connection info:");
    // TODO: Access sender, connection_id, timestamp when available in ReducerContext
    LOG_INFO("  placeholder test for connection info");
}

// Init reducer
SPACETIMEDB_REDUCER(init, ReducerContext ctx) {
    LOG_INFO("Module initialized");
    
    // Insert some initial data
    Person alice{0, "Alice", 30};
    Person bob{0, "Bob", 25};
    ctx.db.person().insert(alice);
    ctx.db.person().insert(bob);
    
    LOG_INFO("Inserted initial persons");
}

// Note: Scheduled reducers like repeating_test from Rust would need additional
// SDK support for the schedule attribute, which isn't implemented yet in C++