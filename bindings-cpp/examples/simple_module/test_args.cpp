#include "../../sdk/include/spacetimedb/spacetimedb_core.h"

// Simple table for testing
struct OneU8 {
    uint8_t n;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneU8& value) {
        buffer.push_back(value.n);
    }
};

// Register the table
SPACETIMEDB_TABLE(OneU8, one_u8, true)

// Reducer that takes a uint8_t argument
SPACETIMEDB_REDUCER(insert_one_u8, SpacetimeDb::ReducerContext ctx) {
    LOG_INFO("insert_one_u8 called");
    
    OneU8 row{42}; // Hard-coded for now
    bool success = ctx.db->table<OneU8>("one_u8").insert(row);
    
    if (success) {
        LOG_INFO("Successfully inserted value");
    } else {
        LOG_ERROR("Failed to insert value");
    }
}

// Built-in reducers
SPACETIMEDB_INIT(init_db) {
    LOG_INFO("Database initialized");
}

// Required exports for SpacetimeDB modules
extern "C" __attribute__((export_name("__describe_module__"))) void __describe_module__(uint32_t description) {
    std::vector<uint8_t> module_bytes = SpacetimeDb::ModuleRegistry::instance().build_module_def();
    
    // Write bytes to description sink
    size_t total_size = module_bytes.size();
    size_t written = 0;
    
    while (written < total_size) {
        size_t chunk_size = std::min(size_t(1024), total_size - written);
        size_t write_size = chunk_size;
        
        uint16_t result = bytes_sink_write(description, 
                                          module_bytes.data() + written, 
                                          &write_size);
        
        if (result != 0) {
            break; // Error writing
        }
        
        written += write_size;
    }
}

extern "C" __attribute__((export_name("__call_reducer__"))) int16_t __call_reducer__(uint32_t id, 
                                   uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                                   uint64_t conn_id_0, uint64_t conn_id_1, 
                                   uint64_t timestamp,
                                   uint32_t args, uint32_t error) {
    try {
        SpacetimeDb::ReducerContext ctx(SpacetimeDb::get_module_db());
        
        bool success = SpacetimeDb::ReducerDispatcher::instance().call_reducer(id, ctx, args);
        return success ? 0 : 1;
    } catch (const std::exception& e) {
        return 1; // Error
    }
}