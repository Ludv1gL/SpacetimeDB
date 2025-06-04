#include "../../../sdk/include/spacetimedb/spacetimedb_core.h"
#include <cstdio>

// =============================================================================
// MINIMAL TEST TABLE
// =============================================================================

struct OneU8 {
    uint8_t n;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneU8& value) {
        buffer.push_back(value.n);
    }
};
SPACETIMEDB_TABLE(OneU8, one_u8, true)

// =============================================================================
// MINIMAL REDUCERS
// =============================================================================

SPACETIMEDB_REDUCER(insert_one_u8, spacetimedb::ReducerContext ctx) {
    OneU8 row{42};
    
    auto table_id = ctx.db->table<OneU8>("one_u8").get_table_id();
    if (table_id != 0) {
        std::vector<uint8_t> buffer;
        OneU8::spacetimedb_serialize(buffer, row);
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
        if (result == 0) {
            LOG_INFO("Successfully inserted u8 value 42");
        } else {
            LOG_ERROR("Failed to insert u8 value");
        }
    }
}

SPACETIMEDB_REDUCER(no_op_succeeds, spacetimedb::ReducerContext ctx) {
    LOG_INFO("No-op reducer succeeded");
}

SPACETIMEDB_INIT(init) {
    LOG_INFO("C++ SDK minimal test module initialized");
}

// =============================================================================
// MODULE EXPORTS
// =============================================================================

std::vector<uint8_t> build_minimal_module_definition() {
    std::vector<uint8_t> module_bytes;
    spacetimedb::BsatnWriter writer(module_bytes);
    
    // RawModuleDef::V9 tag
    writer.write_u8(1);
    
    // 1. typespace: 1 type (OneU8)
    writer.write_vec_len(1); // 1 type
    
    // Type 0: OneU8
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_vec_len(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(7); // U8
    
    // names: empty
    writer.write_vec_len(0);
    
    // 2. tables: 1 table
    writer.write_vec_len(1);
    
    // Table 0: one_u8
    writer.write_string("one_u8");
    writer.write_u32(0); // type ref 0
    writer.write_u8(1);  // primary_key: None
    writer.write_vec_len(0); // indexes
    writer.write_vec_len(0); // constraints
    writer.write_vec_len(0); // sequences
    writer.write_u8(1);  // schedule: None
    writer.write_u8(0);  // table_type: User
    writer.write_u8(0);  // table_access: Public
    
    // 3. reducers: 3 reducers
    writer.write_vec_len(3);
    
    // Reducer 0: init
    writer.write_string("init");
    writer.write_vec_len(0); // params
    writer.write_u8(0); // Some(Init)
    writer.write_u8(0); // Init lifecycle
    
    // Reducer 1: no_op_succeeds
    writer.write_string("no_op_succeeds");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // Reducer 2: insert_one_u8
    writer.write_string("insert_one_u8");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // 4. types: empty
    writer.write_vec_len(0);
    
    // 5. misc_exports: empty
    writer.write_vec_len(0);
    
    // 6. row_level_security: empty
    writer.write_vec_len(0);
    
    return module_bytes;
}

extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    auto module_def = build_minimal_module_definition();
    
    LOG_INFO("Module definition size: " + std::to_string(module_def.size()) + " bytes");
    
    size_t total_size = module_def.size();
    size_t written = 0;
    
    while (written < total_size) {
        size_t chunk_size = std::min(size_t(1024), total_size - written);
        size_t write_size = chunk_size;
        
        uint16_t result = bytes_sink_write(description, 
                                          module_def.data() + written, 
                                          &write_size);
        
        if (result != 0) {
            LOG_ERROR("Failed to write module definition");
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
        
        // Dispatch to reducers
        if (!spacetimedb::ReducerDispatcher::instance().call_reducer(id, ctx, args)) {
            LOG_ERROR("Unknown reducer ID: " + std::to_string(id));
            return 1;
        }
        
        return 0; // Success
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error in __call_reducer__: ") + e.what());
        return 1;
    }
}