/**
 * SpacetimeDB C++ SDK Example: Clean Module Syntax
 * 
 * This example demonstrates the clean syntax for SpacetimeDB C++ modules using
 * the spacetimedb_easy.h header. It showcases:
 * 
 * - One-time table declaration using X-Macro pattern
 * - Automatic table registration and accessor generation
 * - Using ReducerContext for type-safe database access
 * - Multi-parameter reducers with custom types
 * 
 * To build:
 *   emcc -std=c++20 -s STANDALONE_WASM=1 -s FILESYSTEM=0 \
 *        -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry \
 *        -I../../sdk/include -o module.wasm reducer_clean_ctx.cpp
 * 
 * To publish:
 *   spacetime publish --bin-path module.wasm my-database
 */

// Define all tables in one place using X-Macro pattern
// Format: X(TypeName, table_name, is_public)
// This single declaration:
//   - Forward declares the type
//   - Registers the table with SpacetimeDB
//   - Generates the accessor method ctx.db.table_name()
#define SPACETIMEDB_TABLES_LIST \
    X(OneU8, one_u8, true) \
    X(OneU16, one_u16, true) \
    X(OneU32, one_u32, true) \
    X(OneU64, one_u64, true) \
    X(OneU128, one_u128, true) \
    X(OneU256, one_u256, true) \
    X(OneI8, one_i8, true) \
    X(OneI16, one_i16, true) \
    X(OneI32, one_i32, true) \
    X(OneI64, one_i64, true) \
    X(OneI128, one_i128, true) \
    X(OneI256, one_i256, true) \
    X(OneBool, one_bool, true) \
    X(OneF32, one_f32, true) \
    X(OneF64, one_f64, true) \
    X(OneString, one_string, true) \
    X(OneIdentity, one_identity, true) \
    X(OneConnectionId, one_connection_id, true) \
    X(OneTimestamp, one_timestamp, true) \
    X(OneSimpleEnum, one_simple_enum, true)

#include <spacetimedb/spacetimedb.h>
#include <climits>  // For INT32_MAX, INT32_MIN, etc.
#include <cstdint>  // For UINT64_MAX, etc.
#include <limits>   // For std::numeric_limits

using namespace SpacetimeDb;

/**
 * Simple enum for testing enum support (like Rust/C# tests)
 */
enum class SimpleEnum : uint8_t {
    Zero = 0,
    One = 1,
    Two = 2
};

/**
 * Table row types for testing all primitive types.
 * These match the Rust and C# SDK test structures.
 */
struct OneU8 {
    uint8_t n;
};

struct OneU16 {
    uint16_t n;
};

struct OneU32 {
    uint32_t n;
};

struct OneU64 {
    uint64_t n;
};

struct OneU128 {
    U128 n;
};

struct OneU256 {
    U256 n;
};

struct OneI8 {
    int8_t n;
};

struct OneI16 {
    int16_t n;
};

struct OneI32 {
    int32_t n;
};

struct OneI64 {
    int64_t n;
};

struct OneI128 {
    I128 n;
};

struct OneI256 {
    I256 n;
};

struct OneBool {
    bool b;
};

struct OneF32 {
    float f;
};

struct OneF64 {
    double f;
};

struct OneString {
    std::string s;
};

struct OneIdentity {
    Identity i;
};

struct OneConnectionId {
    ConnectionId a;
};

struct OneTimestamp {
    Timestamp t;
};

struct OneSimpleEnum {
    SimpleEnum e;
};



// ===== REDUCER DEFINITIONS =====
// These match the Rust and C# SDK test reducers for parity

// Unsigned integer reducers
SPACETIMEDB_REDUCER(insert_one_u8, ReducerContext ctx, uint8_t n) {
    ctx.db->one_u8().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_u16, ReducerContext ctx, uint16_t n) {
    ctx.db->one_u16().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_u32, ReducerContext ctx, uint32_t n) {
    ctx.db->one_u32().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_u64, ReducerContext ctx, uint64_t n) {
    ctx.db->one_u64().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_u128, ReducerContext ctx, U128 n) {
    ctx.db->one_u128().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_u256, ReducerContext ctx, U256 n) {
    ctx.db->one_u256().insert({n});
}

// Signed integer reducers
SPACETIMEDB_REDUCER(insert_one_i8, ReducerContext ctx, int8_t n) {
    ctx.db->one_i8().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_i16, ReducerContext ctx, int16_t n) {
    ctx.db->one_i16().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_i32, ReducerContext ctx, int32_t n) {
    ctx.db->one_i32().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_i64, ReducerContext ctx, int64_t n) {
    ctx.db->one_i64().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_i128, ReducerContext ctx, I128 n) {
    ctx.db->one_i128().insert({n});
}

SPACETIMEDB_REDUCER(insert_one_i256, ReducerContext ctx, I256 n) {
    ctx.db->one_i256().insert({n});
}

// Boolean reducer
SPACETIMEDB_REDUCER(insert_one_bool, ReducerContext ctx, bool b) {
    ctx.db->one_bool().insert({b});
}

// Floating point reducers
SPACETIMEDB_REDUCER(insert_one_f32, ReducerContext ctx, float f) {
    ctx.db->one_f32().insert({f});
}

SPACETIMEDB_REDUCER(insert_one_f64, ReducerContext ctx, double f) {
    ctx.db->one_f64().insert({f});
}

// String reducer
SPACETIMEDB_REDUCER(insert_one_string, ReducerContext ctx, std::string s) {
    ctx.db->one_string().insert({s});
}

// Identity reducer - commented out due to complex type parameter issue
// SPACETIMEDB_REDUCER(insert_one_identity, ReducerContext ctx, Identity i) {
//     ctx.db->one_identity().insert({i});
// }

// ConnectionId reducer
SPACETIMEDB_REDUCER(insert_one_connection_id, ReducerContext ctx, ConnectionId a) {
    ctx.db->one_connection_id().insert({a});
}

// Timestamp reducer
SPACETIMEDB_REDUCER(insert_one_timestamp, ReducerContext ctx, Timestamp t) {
    ctx.db->one_timestamp().insert({t});
}

// SimpleEnum reducer
SPACETIMEDB_REDUCER(insert_one_simple_enum, ReducerContext ctx, SimpleEnum e) {
    ctx.db->one_simple_enum().insert({e});
}

// ===== TEST REDUCERS =====

// Test reducers that demonstrate enhanced features
SPACETIMEDB_REDUCER(test_type_boundaries, ReducerContext ctx) {
    LOG_INFO("Testing type boundaries for all primitive types");
    
    LogStopwatch timer("type_boundary_tests");
    
    // Test unsigned boundaries
    ctx.db->one_u8().insert({0});
    ctx.db->one_u8().insert({UINT8_MAX});
    
    ctx.db->one_u16().insert({0});
    ctx.db->one_u16().insert({UINT16_MAX});
    
    ctx.db->one_u32().insert({0});
    ctx.db->one_u32().insert({UINT32_MAX});
    
    ctx.db->one_u64().insert({0});
    ctx.db->one_u64().insert({UINT64_MAX});
    
    // Test signed boundaries
    ctx.db->one_i8().insert({INT8_MIN});
    ctx.db->one_i8().insert({INT8_MAX});
    
    ctx.db->one_i16().insert({INT16_MIN});
    ctx.db->one_i16().insert({INT16_MAX});
    
    ctx.db->one_i32().insert({INT32_MIN});
    ctx.db->one_i32().insert({INT32_MAX});
    
    ctx.db->one_i64().insert({INT64_MIN});
    ctx.db->one_i64().insert({INT64_MAX});
    
    // Test floating point special values
    ctx.db->one_f32().insert({0.0f});
    ctx.db->one_f32().insert({-0.0f});
    ctx.db->one_f32().insert({std::numeric_limits<float>::infinity()});
    ctx.db->one_f32().insert({-std::numeric_limits<float>::infinity()});
    ctx.db->one_f32().insert({std::numeric_limits<float>::quiet_NaN()});
    
    ctx.db->one_f64().insert({0.0});
    ctx.db->one_f64().insert({-0.0});
    ctx.db->one_f64().insert({std::numeric_limits<double>::infinity()});
    ctx.db->one_f64().insert({-std::numeric_limits<double>::infinity()});
    ctx.db->one_f64().insert({std::numeric_limits<double>::quiet_NaN()});
    
    LOG_INFO("Boundary test completed");
}

// Insert built-in context values - matches Rust's insert_primitives_as_strings
SPACETIMEDB_REDUCER(insert_primitives_as_strings, ReducerContext ctx) {
    LOG_INFO("Inserting context values as strings");
    
    // Insert identity
    ctx.db->one_identity().insert({ctx.sender});
    
    // Insert connection ID  
    ctx.db->one_connection_id().insert({ctx.connection_id.value_or(ConnectionId{})});
    
    // Insert timestamp
    ctx.db->one_timestamp().insert({ctx.timestamp});
    
    // Insert all enum values
    ctx.db->one_simple_enum().insert({SimpleEnum::Zero});
    ctx.db->one_simple_enum().insert({SimpleEnum::One});
    ctx.db->one_simple_enum().insert({SimpleEnum::Two});
    
    LOG_DEBUG("Context values inserted");
}

// Initialize with sample data
SPACETIMEDB_REDUCER(populate_with_sample_data, ReducerContext ctx) {
    LOG_INFO("Initializing database with sample data");
    
    // Insert sample values for each type
    ctx.db->one_u8().insert({42});
    ctx.db->one_u16().insert({1000});
    ctx.db->one_u32().insert({1000000});
    ctx.db->one_u64().insert({1000000000000ULL});
    
    ctx.db->one_i8().insert({-42});
    ctx.db->one_i16().insert({-1000});
    ctx.db->one_i32().insert({-1000000});
    ctx.db->one_i64().insert({-1000000000000LL});
    
    ctx.db->one_bool().insert({true});
    ctx.db->one_bool().insert({false});
    
    ctx.db->one_f32().insert({3.14159f});
    ctx.db->one_f64().insert({2.71828182845904523536});
    
    ctx.db->one_string().insert({"Hello, SpacetimeDB!"});
    ctx.db->one_string().insert({"C++ Module Library"});
    
    LOG_INFO("Sample data initialization complete");
}

// Insert caller's identity - matches C#'s insert_caller_one_identity  
// Commented out - writes Identity from context instead
// SPACETIMEDB_REDUCER(insert_caller_one_identity, ReducerContext ctx) {
//     ctx.db->one_identity().insert({ctx.sender});
// }

// Insert caller's connection ID - matches C#'s insert_caller_one_connection_id
SPACETIMEDB_REDUCER(insert_caller_one_connection_id, ReducerContext ctx) {
    if (ctx.connection_id.has_value()) {
        ctx.db->one_connection_id().insert({ctx.connection_id.value()});
    } else {
        LOG_INFO("No connection ID available in reducer context");
    }
}

// Insert call timestamp - matches C#'s insert_call_timestamp
SPACETIMEDB_REDUCER(insert_call_timestamp, ReducerContext ctx) {
    ctx.db->one_timestamp().insert({ctx.timestamp});
}

// No-op reducer that always succeeds - matches C#'s no_op_succeeds
SPACETIMEDB_REDUCER(no_op_succeeds, ReducerContext ctx) {
    // This reducer does nothing but succeeds
}
