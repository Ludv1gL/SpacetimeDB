/**
 * SpacetimeDB C++ SDK Test Module - Step 1 Simple: Just a few types
 * Testing with fewer types to diagnose the issue
 */

// Define tables using X-Macro pattern
#define SPACETIMEDB_TABLES_LIST \
    X(OneI8, one_i8, true) \
    X(OneI32, one_i32, true) \
    X(OneBool, one_bool, true)

#include <spacetimedb/spacetimedb.h>

using namespace SpacetimeDb;

// Table row types
struct OneI8 {
    int8_t n;
    
    // Implement BSATN serialization
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        writer.write_i8(n);
    }
    
    static OneI8 bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        OneI8 result;
        result.n = reader.read_i8();
        return result;
    }
};

struct OneI32 {
    int32_t n;
    
    // Implement BSATN serialization
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        writer.write_i32_le(n);
    }
    
    static OneI32 bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        OneI32 result;
        result.n = reader.read_i32_le();
        return result;
    }
};

struct OneBool {
    bool b;
    
    // Implement BSATN serialization
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        writer.write_bool(b);
    }
    
    static OneBool bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        OneBool result;
        result.b = reader.read_bool();
        return result;
    }
};

// Reducers
SPACETIMEDB_REDUCER(insert_one_i8, ReducerContext ctx, int8_t n) {
    OneI8 row{n};
    ctx.db->one_i8().insert(row);
}

SPACETIMEDB_REDUCER(insert_one_i32, ReducerContext ctx, int32_t n) {
    OneI32 row{n};
    ctx.db->one_i32().insert(row);
}

SPACETIMEDB_REDUCER(insert_one_bool, ReducerContext ctx, bool b) {
    OneBool row{b};
    ctx.db->one_bool().insert(row);
}