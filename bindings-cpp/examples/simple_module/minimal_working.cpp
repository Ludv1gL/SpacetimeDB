#include "../../sdk/include/spacetimedb/spacetimedb_core.h"

// Required exports for SpacetimeDB modules
extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    std::vector<uint8_t> module_bytes;
    spacetimedb::BsatnWriter writer(module_bytes);
    
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
    writer.write_vec_len(0);            // primary_key: empty ColList
    writer.write_vec_len(0);            // indexes
    writer.write_vec_len(0);            // constraints
    writer.write_vec_len(0);            // sequences
    writer.write_u8(1);                 // schedule: None
    writer.write_u8(0);                 // table_type: User
    writer.write_u8(0);                 // table_access: Public
    
    // 3. reducers: Vec<RawReducerDefV9> - empty  
    writer.write_vec_len(0);
    
    // 4. types: Vec<RawTypeDefV9> - empty
    writer.write_vec_len(0);
    
    // 5. misc_exports: Vec<RawMiscModuleExportV9> - empty
    writer.write_vec_len(0);
    
    // 6. row_level_security: Vec<RawRowLevelSecurityDefV9> - empty
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
    return 1; // No reducers
}