#include "../../sdk/include/spacetimedb/spacetimedb_core.h"
#include <cstdio>

// Simple table for testing
struct OneU8 {
    uint8_t n;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneU8& value) {
        buffer.push_back(value.n);
    }
};

// Register the table
SPACETIMEDB_TABLE(OneU8, one_u8, true)

// Test reducer using enhanced context
SPACETIMEDB_REDUCER(test_context, spacetimedb::ReducerContext ctx) {
    LOG_INFO("Testing enhanced reducer context");
    
    // Log the timestamp
    char ts_buf[64];
    snprintf(ts_buf, sizeof(ts_buf), "Timestamp: %llu microseconds since epoch", 
             (unsigned long long)ctx.timestamp);
    LOG_INFO(ts_buf);
    
    // Check if we have a connection ID
    if (ctx.connection_id.has_value()) {
        char conn_buf[128];
        snprintf(conn_buf, sizeof(conn_buf), "Connection ID: %llx:%llx", 
                 (unsigned long long)ctx.connection_id->high,
                 (unsigned long long)ctx.connection_id->low);
        LOG_INFO(conn_buf);
    } else {
        LOG_INFO("No connection ID");
    }
    
    // Check if sender is the module itself
    if (ctx.sender == ctx.identity()) {
        LOG_INFO("Sender is the module itself");
    } else {
        LOG_INFO("Sender is a client");
    }
    
    // Insert a row with timestamp as value
    OneU8 row{static_cast<uint8_t>(ctx.timestamp % 256)};
    bool success = ctx.db->table<OneU8>("one_u8").insert(row);
    
    if (success) {
        LOG_INFO("Successfully inserted row");
    } else {
        LOG_ERROR("Failed to insert row");
    }
}

// Built-in init reducer
SPACETIMEDB_INIT(init) {
    LOG_INFO("Module initialized");
    
    // Get module identity
    auto module_id = spacetimedb::Identity::module_identity();
    
    // Log first few bytes of module identity
    char id_buf[128];
    snprintf(id_buf, sizeof(id_buf), "Module identity starts with: %02x%02x%02x%02x",
             module_id.data[0], module_id.data[1], module_id.data[2], module_id.data[3]);
    LOG_INFO(id_buf);
}

// Required exports for SpacetimeDB modules
extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    // Build the module definition
    auto module_def = spacetimedb::ModuleRegistry::instance().build_module_def();
    
    // Write to sink
    size_t total_size = module_def.size();
    size_t written = 0;
    
    while (written < total_size) {
        size_t chunk_size = std::min(size_t(1024), total_size - written);
        size_t write_size = chunk_size;
        
        uint16_t result = bytes_sink_write(description, 
                                          module_def.data() + written, 
                                          &write_size);
        
        if (result != 0) {
            break;
        }
        
        written += write_size;
    }
}

extern "C" __attribute__((export_name("__call_reducer__"))) 
int16_t __call_reducer__(uint32_t id, 
                        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                        uint64_t conn_id_0, uint64_t conn_id_1, 
                        uint64_t timestamp,
                        uint32_t args, uint32_t error) {
    try {
        // Construct identity from parameters
        auto sender_identity = identity_from_params(sender_0, sender_1, sender_2, sender_3);
        
        // Construct connection ID if valid
        std::optional<spacetimedb::ConnectionId> conn_id;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            conn_id = spacetimedb::ConnectionId(conn_id_0, conn_id_1);
        }
        
        // Create reducer context with all information
        spacetimedb::ReducerContext ctx(spacetimedb::get_module_db(), 
                                       sender_identity, 
                                       timestamp, 
                                       conn_id);
        
        // Dispatch to the appropriate reducer
        if (spacetimedb::ReducerDispatcher::instance().call_reducer(id, ctx, args)) {
            return 0; // Success
        } else {
            return 1; // Unknown reducer
        }
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error in __call_reducer__: ") + e.what());
        return 1;
    }
}