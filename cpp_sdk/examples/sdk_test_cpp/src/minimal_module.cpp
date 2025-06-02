// Minimal SpacetimeDB module to test compilation
#include <cstdint>
#include <cstring>
#include <vector>

extern "C" {
    // Host functions we import
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row_ptr, size_t* row_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name_ptr, size_t name_len, uint32_t* out);
    
    // Module exports
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        std::vector<uint8_t> w;
        
        // RawModuleDef::V9
        w.push_back(1);
        
        // Typespace - 1 type
        w.push_back(1); w.push_back(0); w.push_back(0); w.push_back(0);
        
        // Type 0: Product type for OneU8
        w.push_back(2);   // AlgebraicType::Product
        w.push_back(1); w.push_back(0); w.push_back(0); w.push_back(0);  // 1 field
        
        // Field: n  
        w.push_back(0);   // Option::Some
        w.push_back(1); w.push_back(0); w.push_back(0); w.push_back(0);  // string length
        w.push_back('n');
        w.push_back(7);   // AlgebraicType::U8
        
        // Tables - 1 table
        w.push_back(1); w.push_back(0); w.push_back(0); w.push_back(0);
        
        // Table: one_u8
        w.push_back(6); w.push_back(0); w.push_back(0); w.push_back(0);  // string length
        w.push_back('o'); w.push_back('n'); w.push_back('e'); w.push_back('_'); w.push_back('u'); w.push_back('8');
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);  // product_type_ref = 0
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);  // primary_key: empty
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);  // indexes: empty
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);  // constraints: empty
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);  // sequences: empty
        w.push_back(1);   // Option::None for schedule
        w.push_back(1);   // StTableType::User = 1
        w.push_back(0);   // StAccess::Public = 0
        
        // Reducers - 1 reducer
        w.push_back(1); w.push_back(0); w.push_back(0); w.push_back(0);
        
        // Reducer: insert_one_u8
        w.push_back(13); w.push_back(0); w.push_back(0); w.push_back(0);  // string length
        const char* name = "insert_one_u8";
        for (int i = 0; i < 13; i++) w.push_back(name[i]);
        
        // params: ProductType
        w.push_back(1); w.push_back(0); w.push_back(0); w.push_back(0);  // elements.len() = 1
        
        // ProductTypeElement for arg n
        w.push_back(0);   // Option::Some for name
        w.push_back(1); w.push_back(0); w.push_back(0); w.push_back(0);  // string length
        w.push_back('n');
        w.push_back(7);   // AlgebraicType::U8
        
        // lifecycle: Option<Lifecycle>
        w.push_back(1);   // Option::None
        
        // Types - empty
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);
        
        // MiscExports - empty
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);
        
        // RowLevelSecurity - empty
        w.push_back(0); w.push_back(0); w.push_back(0); w.push_back(0);
        
        size_t len = w.size();
        bytes_sink_write(sink, w.data(), &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        if (id == 0) {  // insert_one_u8 reducer
            // Get table ID
            const char* table_name = "one_u8";
            uint32_t table_id;
            if (table_id_from_name((const uint8_t*)table_name, strlen(table_name), &table_id) != 0) {
                return -1;
            }
            
            // Read args
            uint8_t n = 42;  // Default value
            if (args_source != 0) {
                size_t buf_len = 1;
                bytes_source_read(args_source, &n, &buf_len);
            }
            
            // Insert row
            size_t row_len = 1;
            datastore_insert_bsatn(table_id, &n, &row_len);
            
            return 0;
        }
        
        return -1;
    }
}