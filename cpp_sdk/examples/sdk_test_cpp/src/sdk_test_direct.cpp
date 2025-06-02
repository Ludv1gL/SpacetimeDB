// Direct implementation without SDK macros to test basic functionality
#include <cstdint>
#include <cstring>

extern "C" {
    // Import from spacetime ABI
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // Minimal module description
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Create minimal but valid RawModuleDef::V9
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            
            // Typespace with one type
            1, 0, 0, 0,     // typespace length = 1
            
            // Type 0: Product with one u8 field
            0,              // AlgebraicType::Product = 0
            1, 0, 0, 0,     // elements count = 1
            1,              // Some(name)
            1, 0, 0, 0,     // name length = 1
            'n',            // field name
            1,              // AlgebraicType::U8 = 1
            
            // Tables
            1, 0, 0, 0,     // tables length = 1
            
            // Table: one_u8
            6, 0, 0, 0,     // name length = 6
            'o', 'n', 'e', '_', 'u', '8',  // table name
            0, 0, 0, 0,     // product_type_ref = 0
            0, 0, 0, 0,     // primary_key (empty vec)
            0, 0, 0, 0,     // indexes (empty vec)
            0, 0, 0, 0,     // constraints (empty vec)
            0, 0, 0, 0,     // sequences (empty vec)
            0,              // schedule: None = 0
            0,              // table_type: User = 0
            0,              // table_access: Public = 0
            
            // Reducers
            1, 0, 0, 0,     // reducers length = 1
            
            // Reducer: insert_one_u8
            13, 0, 0, 0,    // name length = 13
            'i', 'n', 's', 'e', 'r', 't', '_', 'o', 'n', 'e', '_', 'u', '8',
            1, 0, 0, 0,     // args count = 1
            1,              // Some(name)
            1, 0, 0, 0,     // name length = 1  
            'n',            // arg name
            1,              // AlgebraicType::U8 = 1
            
            // Types (empty)
            0, 0, 0, 0,     // types length = 0
            
            // MiscExports (empty)
            0, 0, 0, 0,     // misc_exports length = 0
            
            // RowLevelSecurity (empty) 
            0, 0, 0, 0      // row_level_security length = 0
        };
        
        size_t len = sizeof(data);
        bytes_sink_write(sink, data, &len);
    }
    
    // Reducer implementation
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        // For now, just return success
        return 0;
    }
}