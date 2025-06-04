#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/module.h>

// Define a simple table
struct OneU8 {
    uint8_t n;
    
    // BSATN serialization
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& w) const {
        w.write_u8(n);
    }
    
    // Schema description
    static void write_schema(SpacetimeDb::bsatn::Writer& w) {
        w.write_u32_le(1); // 1 field
        // Field n
        w.write_u8(0); // Some
        w.write_string("n");
        w.write_u8(static_cast<uint8_t>(spacetimedb::AlgebraicTypeTag::U8));
    }
};

// Register table
SPACETIMEDB_TABLE(OneU8, one_u8, true)

// Define a reducer
SPACETIMEDB_REDUCER(insert_one_u8, spacetimedb::ReducerContext ctx, uint8_t n) {
    spacetimedb::log_info("insert_one_u8 called with n=" + std::to_string(n));
    
    OneU8 row{n};
    ctx.db.table<OneU8>("one_u8").insert(row);
    
    spacetimedb::log_info("Successfully inserted value");
}

// Module exports - use the improved SDK's module description generation
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        SpacetimeDb::bsatn::Writer w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace
        auto& registry = spacetimedb::ModuleRegistry::instance();
        w.write_u32_le(registry.get_tables().size());
        
        // Write types for tables
        for (const auto& table : registry.get_tables()) {
            table.write_schema(w);
        }
        
        // Tables
        w.write_u32_le(registry.get_tables().size());
        for (const auto& table : registry.get_tables()) {
            w.write_string(table.name);
            w.write_u32_le(table.type_ref);
            w.write_u32_le(0); // primary_key = empty vec
            w.write_u32_le(0); // indexes
            w.write_u32_le(0); // constraints
            w.write_u32_le(0); // sequences
            w.write_u8(1);     // schedule = None
            w.write_u8(1);     // table_type = User
            w.write_u8(table.is_public ? 0 : 1); // table_access
        }
        
        // Reducers
        w.write_u32_le(registry.get_reducers().size());
        for (const auto& reducer : registry.get_reducers()) {
            w.write_string(reducer.name);
            reducer.write_params(w);
            w.write_u8(1); // lifecycle = None
        }
        
        // Empty types, misc exports, row level security
        w.write_u32_le(0);
        w.write_u32_le(0);
        w.write_u32_le(0);
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        spacetime_bytes_sink_write(sink, buffer.data(), &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id,
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        auto& registry = spacetimedb::ModuleRegistry::instance();
        if (id < registry.get_reducers().size()) {
            spacetimedb::ReducerContext ctx;
            registry.get_reducers()[id].handler(ctx, args, error);
            return 0;
        }
        return -1;
    }
}