#include "../../sdk/include/spacetimedb/spacetimedb_core.h"
#include <cstdio>

// Simple table for testing
struct OneU8 {
    uint8_t n;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneU8& value) {
        buffer.push_back(value.n);
    }
};

// Test reducer using enhanced context
void test_enhanced_context(SpacetimeDb::ReducerContext ctx) {
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
}

// Init reducer
void init_reducer(SpacetimeDb::ReducerContext ctx) {
    LOG_INFO("Module initialized");
    
    // Get module identity
    auto module_id = ctx.identity();
    
    // Log first few bytes of module identity
    char id_buf[128];
    snprintf(id_buf, sizeof(id_buf), "Module identity starts with: %02x%02x%02x%02x",
             module_id.data[0], module_id.data[1], module_id.data[2], module_id.data[3]);
    LOG_INFO(id_buf);
}

// Required exports for SpacetimeDB modules
extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    std::vector<uint8_t> module_bytes;
    SpacetimeDb::BsatnWriter writer(module_bytes);
    
    // RawModuleDef::V9 tag
    writer.write_u8(1);
    
    // RawModuleDefV9 structure:
    
    // 1. typespace: Typespace with one type
    writer.write_vec_len(1); // 1 type
    
    // Type 0: Product type for OneU8
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_vec_len(1); // 1 element
    
    // Element 0: n field
    writer.write_u8(0); // Option::Some
    writer.write_string("n");
    writer.write_u8(7); // AlgebraicType::U8
    
    writer.write_vec_len(0); // names
    
    // 2. tables: Vec<RawTableDefV9> - one table
    writer.write_vec_len(1);
    
    // Table: one_u8
    writer.write_string("one_u8");      // name
    writer.write_u32(0);                // product_type_ref
    writer.write_u8(1);                 // primary_key: None
    writer.write_vec_len(0);            // indexes
    writer.write_vec_len(0);            // constraints
    writer.write_vec_len(0);            // sequences
    writer.write_u8(1);                 // schedule: None
    writer.write_u8(0);                 // table_type: User
    writer.write_u8(0);                 // table_access: Public
    
    // 3. reducers: Vec<RawReducerDefV9>
    writer.write_vec_len(2); // 2 reducers
    
    // Reducer 0: init
    writer.write_string("init");         // name
    writer.write_vec_len(0);             // params (empty product)
    writer.write_u8(0);                  // lifecycle: Some(Init)
    writer.write_u8(0);                  // lifecycle value: Init
    
    // Reducer 1: test_context
    writer.write_string("test_context"); // name
    writer.write_vec_len(0);             // params (empty product)
    writer.write_u8(1);                  // lifecycle: None
    
    // 4. types: Vec<RawTypeDefV9>
    writer.write_vec_len(0);
    
    // 5. misc_exports: Vec<RawMiscModuleExportV9>
    writer.write_vec_len(0);
    
    // 6. row_level_security: Vec<RawRowLevelSecurityDefV9>
    writer.write_vec_len(0);
    
    // Write to sink
    size_t total_size = module_bytes.size();
    size_t written = 0;
    
    while (written < total_size) {
        size_t chunk_size = std::min(size_t(1024), total_size - written);
        size_t write_size = chunk_size;
        
        uint16_t result = bytes_sink_write(description, 
                                          module_bytes.data() + written, 
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
        std::optional<SpacetimeDb::ConnectionId> conn_id;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            conn_id = SpacetimeDb::ConnectionId(conn_id_0, conn_id_1);
        }
        
        // Create reducer context with all information
        SpacetimeDb::ReducerContext ctx(SpacetimeDb::get_module_db(), 
                                       sender_identity, 
                                       timestamp, 
                                       conn_id);
        
        // Dispatch to the appropriate reducer
        if (id == 0) {
            init_reducer(ctx);
        } else if (id == 1) {
            test_enhanced_context(ctx);
        } else {
            return 1; // Unknown reducer
        }
        
        return 0; // Success
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error in __call_reducer__: ") + e.what());
        return 1;
    }
}