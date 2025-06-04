#include "../../sdk/include/spacetimedb/spacetimedb_autogen.h"
#include <cstdio>

// Define a simple table structure
struct OneU8 {
    uint8_t n;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneU8& value) {
        buffer.push_back(value.n);
    }
};

// Register the OneU8 type with its fields
SPACETIMEDB_REGISTER_TYPE(OneU8,
    SPACETIMEDB_FIELD(OneU8, n)
)

// Required exports for SpacetimeDB modules
extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    // Manually register table and reducer for this test
    spacetimedb::AutogenModuleRegistry::instance().register_table<OneU8>("one_u8", true);
    spacetimedb::AutogenModuleRegistry::instance().register_reducer("init", {}, 0);
    spacetimedb::AutogenModuleRegistry::instance().register_reducer("test_insert");
    
    // Build the module definition
    auto module_def = spacetimedb::AutogenModuleRegistry::instance().build_module_def();
    
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
        
        // Create reducer context
        spacetimedb::ReducerContext ctx(spacetimedb::get_module_db(), 
                                       sender_identity, 
                                       timestamp, 
                                       conn_id);
        
        // Simple dispatch
        if (id == 0) {
            // init reducer
            LOG_INFO("Autogen simple module initialized");
            
            // Test table insertion
            OneU8 row{42};
            auto table_id = ctx.db->table<OneU8>("one_u8").get_table_id();
            
            if (table_id != 0) {
                std::vector<uint8_t> buffer;
                buffer.push_back(row.n);
                size_t buffer_size = buffer.size();
                uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
                
                if (result == 0) {
                    LOG_INFO("Successfully inserted value 42");
                } else {
                    LOG_ERROR("Failed to insert value");
                }
            } else {
                LOG_ERROR("Failed to get table ID");
            }
        } else if (id == 1) {
            // test_insert reducer
            LOG_INFO("test_insert reducer called");
        } else {
            return 1; // Unknown reducer
        }
        
        return 0; // Success
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error in __call_reducer__: ") + e.what());
        return 1;
    }
}