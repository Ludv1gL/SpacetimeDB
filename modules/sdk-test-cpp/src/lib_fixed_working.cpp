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
    X(OneU8, another_u8, false) \
    X(SignedInt8, signed_i8, true) \
    X(UnsignedInt16, unsigned_u16, true) \
    X(SignedInt16, signed_i16, true) \
    X(UnsignedInt32, unsigned_u32, true) \
    X(SignedInt32, signed_i32, true)

#include <spacetimedb/spacetimedb.h>
#include <climits>  // For INT32_MAX, INT32_MIN
#include <cstdint>  // For UINT64_MAX

using namespace SpacetimeDb;

/**
 * Example table row types.
 * These structs will be automatically serialized using BSATN.
 */
struct OneU8 {
    uint8_t n;
};

// Basic signed and unsigned integer types
struct SignedInt8 {
    int8_t n;
};

struct UnsignedInt16 {
    uint16_t n;
};

struct SignedInt16 {
    int16_t n;
};

struct UnsignedInt32 {
    uint32_t n;
};

struct SignedInt32 {
    int32_t n;
};

// No need for SPACETIMEDB_TABLE declarations anymore!
// Tables are automatically registered from the X-macro list above
// Insert a single value into the public table
SPACETIMEDB_REDUCER(insert_one_u8, ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db->one_u8().insert(row);
}

// Insert a single value into the private table
SPACETIMEDB_REDUCER(insert_another_u8, ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db->another_u8().insert(row);
}

// Initialize the database with default values - demonstrates enhanced logging
SPACETIMEDB_REDUCER(init_db, ReducerContext ctx) {
    LOG_INFO("Database initialized with enhanced logging!");
    
    // Performance measurement example
    {
        LogStopwatch timer("database_initialization");
        ctx.db->one_u8().insert({42});
        ctx.db->another_u8().insert({100});
        LOG_DEBUG("Default values inserted successfully");
    } // Timer automatically ends here
}

// Insert a value with an offset calculation - demonstrates caller info logging
SPACETIMEDB_REDUCER(insert_with_offset, ReducerContext ctx, uint8_t n, uint8_t offset) {
    LOG_DEBUG("Computing offset: " + std::to_string(n) + " + " + std::to_string(offset));
    OneU8 row{static_cast<uint8_t>(n + offset)};
    ctx.db->one_u8().insert(row);
    LOG_TRACE("Inserted value: " + std::to_string(row.n));
}

// Insert a range of values with a specified step - demonstrates performance timing
SPACETIMEDB_REDUCER(insert_range, ReducerContext ctx, uint8_t start, uint8_t end, uint8_t step) {
    LogStopwatch range_timer("bulk_insert_range");
    
    LOG_INFO("Inserting range: " + std::to_string(start) + " to " + std::to_string(end) + 
             " with step " + std::to_string(step));
    
    uint32_t count = 0;
    for (uint8_t i = start; i <= end; i += step) {
        OneU8 row{i};
        ctx.db->one_u8().insert(row);
        count++;
    }
    
    LOG_INFO("Successfully inserted " + std::to_string(count) + " values");
}

// ===== Basic Primitive Type Reducers =====

// Demonstrate signed 8-bit integers with boundary tests
SPACETIMEDB_REDUCER(test_i8_boundaries, ReducerContext ctx) {
    LOG_INFO("Testing i8 boundaries");
    
    // Test minimum, maximum, and zero values
    ctx.db->signed_i8().insert({-128});  // min value
    ctx.db->signed_i8().insert({127});   // max value
    ctx.db->signed_i8().insert({0});     // zero
    ctx.db->signed_i8().insert({-1});    // negative
    
    LOG_DEBUG("Inserted i8 boundary values: -128, 127, 0, -1");
}

// Demonstrate unsigned 16-bit integers
SPACETIMEDB_REDUCER(insert_u16, ReducerContext ctx, uint16_t value) {
    UnsignedInt16 row{value};
    ctx.db->unsigned_u16().insert(row);
    LOG_TRACE("Inserted u16 value: " + std::to_string(value));
}

// Demonstrate signed 16-bit integers with arithmetic
SPACETIMEDB_REDUCER(compute_i16_sum, ReducerContext ctx, int16_t a, int16_t b) {
    int16_t sum = a + b;
    SignedInt16 row{sum};
    ctx.db->signed_i16().insert(row);
    LOG_INFO("Computed i16 sum: " + std::to_string(a) + " + " + std::to_string(b) + " = " + std::to_string(sum));
}

// Demonstrate unsigned 32-bit integers with bit operations
SPACETIMEDB_REDUCER(bitwise_u32, ReducerContext ctx, uint32_t a, uint32_t b) {
    LogStopwatch timer("bitwise_operations");
    
    ctx.db->unsigned_u32().insert({a & b});  // AND
    ctx.db->unsigned_u32().insert({a | b});  // OR
    ctx.db->unsigned_u32().insert({a ^ b});  // XOR
    ctx.db->unsigned_u32().insert({~a});     // NOT a
    
    LOG_DEBUG("Performed bitwise operations on u32 values");
}

// Demonstrate signed 32-bit integers with overflow handling
SPACETIMEDB_REDUCER(safe_multiply_i32, ReducerContext ctx, int32_t a, int32_t b) {
    // Note: In production code, you'd want to check for overflow
    int64_t result = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    
    if (result > INT32_MAX || result < INT32_MIN) {
        LOG_ERROR("Multiplication overflow detected!");
        return;
    }
    
    SignedInt32 row{static_cast<int32_t>(result)};
    ctx.db->signed_i32().insert(row);
    LOG_INFO("Safely multiplied i32 values: " + std::to_string(a) + " * " + std::to_string(b) + " = " + std::to_string(result));
}

