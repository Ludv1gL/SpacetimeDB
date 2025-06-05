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

// Test reducer without arguments
SPACETIMEDB_REDUCER(test_basic, SpacetimeDb::ReducerContext ctx) {
    LOG_INFO("test_basic called");
    
    OneU8 row{42};
    bool success = ctx.db->table<OneU8>("one_u8").insert(row);
    
    if (success) {
        LOG_INFO("Successfully inserted value");
    } else {
        LOG_ERROR("Failed to insert value");
    }
}

// Built-in reducers
SPACETIMEDB_INIT(init) {
    LOG_INFO("Database initialized");
}

// Required exports for SpacetimeDB modules
extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    // First, let's manually create a simple module definition to test
    std::vector<uint8_t> module_bytes;
    SpacetimeDb::BsatnWriter writer(module_bytes);
    
    // RawModuleDef::V9 tag
    writer.write_u8(1);
    
    // RawModuleDefV9 structure:
    
    // 1. typespace: Typespace with our OneU8 type
    // types: Vec<AlgebraicType>
    writer.write_vec_len(1); // 1 type
    
    // Type 0: Product type for OneU8
    writer.write_u8(2); // AlgebraicType::Product
    
    // ProductType structure:
    writer.write_vec_len(1); // 1 element in product
    
    // ProductTypeElement 0: n field
    writer.write_u8(0); // Option::Some for name
    writer.write_string("n");
    writer.write_u8(7); // AlgebraicType::U8
    
    // names: Vec<ScopedTypeName>
    writer.write_vec_len(0); // No named types for now
    
    // 2. tables: Vec<RawTableDefV9>
    writer.write_vec_len(1); // 1 table
    
    // Table 0: one_u8
    writer.write_string("one_u8");       // name
    writer.write_u32(0);                 // product_type_ref (Type 0)
    writer.write_u8(1);                  // primary_key: None
    writer.write_vec_len(0);             // indexes
    writer.write_vec_len(0);             // constraints
    writer.write_vec_len(0);             // sequences
    writer.write_u8(1);                  // schedule: None
    writer.write_u8(0);                  // table_type: User
    writer.write_u8(0);                  // table_access: Public
    
    // 3. reducers: Vec<RawReducerDefV9>
    writer.write_vec_len(2); // 2 reducers
    
    // Reducer 0: init
    writer.write_string("init");         // name
    writer.write_vec_len(0);             // params (empty product)
    writer.write_u8(0);                  // lifecycle: Some(Init)
    writer.write_u8(0);                  // lifecycle value: Init
    
    // Reducer 1: test_basic
    writer.write_string("test_basic");   // name
    writer.write_vec_len(0);             // params (empty product)
    writer.write_u8(1);                  // lifecycle: None
    
    // 4. types: Vec<RawTypeDefV9>
    writer.write_vec_len(0); // No named types
    
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
        SpacetimeDb::ReducerContext ctx(SpacetimeDb::get_module_db());
        
        // Simple dispatch based on ID
        if (id == 0) {
            // init reducer
            LOG_INFO("Database initialized");
        } else if (id == 1) {
            // test_basic reducer
            LOG_INFO("test_basic called");
            
            OneU8 row{42};
            auto table_id = ctx.db->table<OneU8>("one_u8").get_table_id();
            
            if (table_id != 0) {
                std::vector<uint8_t> buffer;
                buffer.push_back(row.n);
                size_t buffer_size = buffer.size();
                uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
                
                if (result == 0) {
                    LOG_INFO("Successfully inserted value");
                } else {
                    LOG_ERROR("Failed to insert value");
                }
            } else {
                LOG_ERROR("Failed to get table ID");
            }
        } else {
            return 1; // Unknown reducer
        }
        
        return 0; // Success
    } catch (const std::exception& e) {
        return 1; // Error
    }
}