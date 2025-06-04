#include <spacetimedb/spacetimedb.h>

// Simple table without strings
struct OneU8 { 
    uint8_t n; 
    void serialize(spacetimedb::Writer& w) const { w.write_u8(n); }
    void deserialize(spacetimedb::Reader& r) { n = r.read_u8(); }
};

struct OneU32 { 
    uint32_t n; 
    void serialize(spacetimedb::Writer& w) const { w.write_u32(n); }
    void deserialize(spacetimedb::Reader& r) { n = r.read_u32(); }
};

// Mark types as having custom serialization
namespace spacetimedb {
    template<> struct has_custom_serialize<OneU8> : std::true_type {};
    template<> struct has_custom_serialize<OneU32> : std::true_type {};
}

// Table declarations
SPACETIMEDB_TABLE(OneU8, one_u8, true);
SPACETIMEDB_TABLE(OneU32, one_u32, true);

// Reducers
void insert_one_u8_impl(spacetimedb::ReducerContext& ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db.table<OneU8>("one_u8").insert(row);
}

void insert_one_u32_impl(spacetimedb::ReducerContext& ctx, uint32_t n) {
    OneU32 row{n};
    ctx.db.table<OneU32>("one_u32").insert(row);
}

void count_one_u8_impl(spacetimedb::ReducerContext& ctx) {
    auto count = ctx.db.table<OneU8>("one_u8").count();
    // Can't log without strings, but the reducer will still work
}

// Register all reducers
namespace {
    struct ReducerRegistrations {
        ReducerRegistrations() {
            using namespace spacetimedb;
            register_reducer<uint8_t>("insert_one_u8", 
                std::function<void(ReducerContext&, uint8_t)>(insert_one_u8_impl));
            register_reducer<uint32_t>("insert_one_u32", 
                std::function<void(ReducerContext&, uint32_t)>(insert_one_u32_impl));
            register_reducer<>("count_one_u8", 
                std::function<void(ReducerContext&)>(count_one_u8_impl));
        }
    };
    static ReducerRegistrations reducer_registrations;
}

// Module description
extern "C" void __describe_module__(uint8_t* buffer, size_t* len) {
    std::vector<uint8_t> buf;
    
    // RawModuleDef::V9 tag
    buf.push_back(1);
    
    // Helper to write u32
    auto write_u32 = [&buf](uint32_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
    };
    
    // Helper to write string
    auto write_string = [&buf, &write_u32](const char* s) {
        uint32_t len = std::strlen(s);
        write_u32(len);
        for (uint32_t i = 0; i < len; i++) {
            buf.push_back(s[i]);
        }
    };
    
    // 1. Typespace (empty)
    write_u32(0);
    
    // 2. Tables (2 tables)
    write_u32(2);
    
    // Table: one_u8
    {
        write_string("one_u8");
        // Product type
        buf.push_back(7);
        // Product elements (1)
        write_u32(1);
        // Element 0: name=Some("n"), type=U8
        buf.push_back(0); // Some
        write_string("n");
        buf.push_back(7); // U8
        
        // Indexes, Constraints, Sequences (all empty)
        write_u32(0);
        write_u32(0);
        write_u32(0);
        // Schedule (None)
        buf.push_back(1);
        // Primary key (None)
        buf.push_back(1);
        // Access (Public)
        buf.push_back(0);
    }
    
    // Table: one_u32
    {
        write_string("one_u32");
        // Product type
        buf.push_back(7);
        // Product elements (1)
        write_u32(1);
        // Element 0: name=Some("n"), type=U32
        buf.push_back(0); // Some
        write_string("n");
        buf.push_back(11); // U32
        
        // Indexes, Constraints, Sequences (all empty)
        write_u32(0);
        write_u32(0);
        write_u32(0);
        // Schedule (None)
        buf.push_back(1);
        // Primary key (None)
        buf.push_back(1);
        // Access (Public)
        buf.push_back(0);
    }
    
    // 3. Reducers (3 reducers)
    write_u32(3);
    
    // Reducer: insert_one_u8
    {
        write_string("insert_one_u8");
        // Product type for parameters
        buf.push_back(7); // Product
        write_u32(1); // 1 parameter
        // Parameter: name=Some("n"), type=U8
        buf.push_back(0); // Some
        write_string("n");
        buf.push_back(7); // U8
        // Lifecycle flags (None)
        buf.push_back(0);
    }
    
    // Reducer: insert_one_u32
    {
        write_string("insert_one_u32");
        // Product type for parameters
        buf.push_back(7); // Product
        write_u32(1); // 1 parameter
        // Parameter: name=Some("n"), type=U32
        buf.push_back(0); // Some
        write_string("n");
        buf.push_back(11); // U32
        // Lifecycle flags (None)
        buf.push_back(0);
    }
    
    // Reducer: count_one_u8
    {
        write_string("count_one_u8");
        // Product type for parameters
        buf.push_back(7); // Product
        write_u32(0); // 0 parameters
        // Lifecycle flags (None)
        buf.push_back(0);
    }
    
    // 4. Types (empty)
    write_u32(0);
    
    // 5. MiscExports (empty)
    write_u32(0);
    
    // Copy to output buffer
    if (*len >= buf.size()) {
        std::memcpy(buffer, buf.data(), buf.size());
        *len = buf.size();
    }
}

// Implementation of __call_reducer__
extern "C" void __call_reducer__(const char* name, size_t name_len, const uint8_t* args, size_t args_len) {
    std::string reducer_name(name, name_len);
    spacetimedb::ReducerRegistry::instance().dispatch(reducer_name, args, args_len);
}