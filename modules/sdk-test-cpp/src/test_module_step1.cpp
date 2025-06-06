/**
 * SpacetimeDB C++ SDK Test Module - Step 1: Basic Types
 * 
 * This module tests basic primitive types: i8, i16, i32, i64, f32, f64, bool
 * Using the macro-based approach with SpacetimeDB headers
 */

// Define all tables in one place using X-Macro pattern
#define SPACETIMEDB_TABLES_LIST \
    X(OneI8, one_i8, true) \
    X(OneI16, one_i16, true) \
    X(OneI32, one_i32, true) \
    X(OneI64, one_i64, true) \
    X(OneF32, one_f32, true) \
    X(OneF64, one_f64, true) \
    X(OneBool, one_bool, true)

#include <spacetimedb/spacetimedb.h>

using namespace SpacetimeDb;

// Table row types - simple structs with BSATN serialization
struct OneI8 {
    int8_t n;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_i8(n);
    }
    
    static OneI8 bsatn_deserialize(bsatn::Reader& reader) {
        OneI8 result;
        result.n = reader.read_i8();
        return result;
    }
};

struct OneI16 {
    int16_t n;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_i16_le(n);
    }
    
    static OneI16 bsatn_deserialize(bsatn::Reader& reader) {
        OneI16 result;
        result.n = reader.read_i16_le();
        return result;
    }
};

struct OneI32 {
    int32_t n;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_i32_le(n);
    }
    
    static OneI32 bsatn_deserialize(bsatn::Reader& reader) {
        OneI32 result;
        result.n = reader.read_i32_le();
        return result;
    }
};

struct OneI64 {
    int64_t n;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_i64_le(n);
    }
    
    static OneI64 bsatn_deserialize(bsatn::Reader& reader) {
        OneI64 result;
        result.n = reader.read_i64_le();
        return result;
    }
};

struct OneF32 {
    float f;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_f32_le(f);
    }
    
    static OneF32 bsatn_deserialize(bsatn::Reader& reader) {
        OneF32 result;
        result.f = reader.read_f32_le();
        return result;
    }
};

struct OneF64 {
    double f;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_f64_le(f);
    }
    
    static OneF64 bsatn_deserialize(bsatn::Reader& reader) {
        OneF64 result;
        result.f = reader.read_f64_le();
        return result;
    }
};

struct OneBool {
    bool b;
    
    void bsatn_serialize(bsatn::Writer& writer) const {
        writer.write_bool(b);
    }
    
    static OneBool bsatn_deserialize(bsatn::Reader& reader) {
        OneBool result;
        result.b = reader.read_bool();
        return result;
    }
};

// Reducers for each type
SPACETIMEDB_REDUCER(insert_one_i8, ReducerContext ctx, int8_t n) {
    OneI8 row{n};
    ctx.db->one_i8().insert(row);
    LOG_INFO("Inserted i8 value: " + std::to_string(n));
}

SPACETIMEDB_REDUCER(insert_one_i16, ReducerContext ctx, int16_t n) {
    OneI16 row{n};
    ctx.db->one_i16().insert(row);
    LOG_INFO("Inserted i16 value: " + std::to_string(n));
}

SPACETIMEDB_REDUCER(insert_one_i32, ReducerContext ctx, int32_t n) {
    OneI32 row{n};
    ctx.db->one_i32().insert(row);
    LOG_INFO("Inserted i32 value: " + std::to_string(n));
}

SPACETIMEDB_REDUCER(insert_one_i64, ReducerContext ctx, int64_t n) {
    OneI64 row{n};
    ctx.db->one_i64().insert(row);
    LOG_INFO("Inserted i64 value: " + std::to_string(n));
}

SPACETIMEDB_REDUCER(insert_one_f32, ReducerContext ctx, float f) {
    OneF32 row{f};
    ctx.db->one_f32().insert(row);
    LOG_INFO("Inserted f32 value: " + std::to_string(f));
}

SPACETIMEDB_REDUCER(insert_one_f64, ReducerContext ctx, double f) {
    OneF64 row{f};
    ctx.db->one_f64().insert(row);
    LOG_INFO("Inserted f64 value: " + std::to_string(f));
}

SPACETIMEDB_REDUCER(insert_one_bool, ReducerContext ctx, bool b) {
    OneBool row{b};
    ctx.db->one_bool().insert(row);
    LOG_INFO("Inserted bool value: " + std::string(b ? "true" : "false"));
}

// Test reducer that inserts sample values
SPACETIMEDB_REDUCER(test_all_types, ReducerContext ctx) {
    LOG_INFO("Testing all primitive types...");
    
    // Insert test values
    ctx.db->one_i8().insert({-128});
    ctx.db->one_i8().insert({127});
    ctx.db->one_i8().insert({0});
    
    ctx.db->one_i16().insert({-32768});
    ctx.db->one_i16().insert({32767});
    
    ctx.db->one_i32().insert({-2147483648});
    ctx.db->one_i32().insert({2147483647});
    
    ctx.db->one_i64().insert({-9223372036854775807LL});
    ctx.db->one_i64().insert({9223372036854775807LL});
    
    ctx.db->one_f32().insert({3.14159f});
    ctx.db->one_f32().insert({-1.23456f});
    
    ctx.db->one_f64().insert({2.718281828459045});
    ctx.db->one_f64().insert({-3.141592653589793});
    
    ctx.db->one_bool().insert({true});
    ctx.db->one_bool().insert({false});
    
    LOG_INFO("All test values inserted successfully!");
}