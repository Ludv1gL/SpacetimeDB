#include <cstdint>
#include <cstddef>

// Enhanced SDK test that demonstrates more functionality while avoiding WASI dependencies
extern "C" {
    // Import core SpacetimeDB functions from spacetime_10.0
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(
        uint8_t level,
        const uint8_t *target_ptr, uint32_t target_len,
        const uint8_t *filename_ptr, uint32_t filename_len, 
        uint32_t line_number,
        const uint8_t *message_ptr, uint32_t message_len
    );
    
    __attribute__((import_module("spacetime_10.0"), import_name("identity")))
    void identity(uint8_t* out_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* out);
    
    // Import table operations
    __attribute__((import_module("spacetime_10.0"), import_name("table_insert")))
    uint16_t table_insert(uint32_t table_id, const uint8_t* row_ptr, size_t* row_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_scan")))
    uint32_t table_scan(uint32_t table_id, uint8_t* filter_ptr, uint32_t filter_len);
    
    // Helper function to create BSATN module definition
    void create_module_definition(uint8_t* buffer, size_t* len) {
        // Create RawModuleDef::V9 with one table and one reducer
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            
            // Tables (1 table)
            1, 0, 0, 0,     // tables vector length = 1
            
            // Table definition: TestTable with id and data fields
            9, 0, 0, 0,     // table name length = 9
            'T', 'e', 's', 't', 'T', 'a', 'b', 'l', 'e',  // table name "TestTable"
            
            // Product type with 2 fields  
            1,              // ProductType tag = 1
            2, 0, 0, 0,     // elements length = 2
            
            // Field 1: id (U32)
            2, 0, 0, 0,     // field name length = 2  
            'i', 'd',       // field name "id"
            8,              // AlgebraicType::U32 = 8
            
            // Field 2: data (U32)
            4, 0, 0, 0,     // field name length = 4
            'd', 'a', 't', 'a',  // field name "data"
            8,              // AlgebraicType::U32 = 8
            
            // Table properties
            1,              // is_public = true
            0,              // unique_id = 0
            0, 0, 0, 0,     // indexes (empty vector)
            0, 0, 0, 0,     // constraints (empty vector)
            0, 0, 0, 0,     // sequences (empty vector)
            
            // Reducers (1 reducer)
            1, 0, 0, 0,     // reducers vector length = 1
            
            // Reducer definition: add_data
            8, 0, 0, 0,     // reducer name length = 8
            'a', 'd', 'd', '_', 'd', 'a', 't', 'a',  // reducer name "add_data"
            
            0,              // ReducerKind::Update = 0
            1, 0, 0, 0,     // args vector length = 1
            
            // Argument: data (U32)
            4, 0, 0, 0,     // arg name length = 4
            'd', 'a', 't', 'a',  // arg name "data"
            8,              // AlgebraicType::U32 = 8
            
            0, 0, 0, 0,     // types (empty vector)
            0, 0, 0, 0,     // misc_exports (empty vector)
            0, 0, 0, 0      // row_level_security (empty vector)
        };
        
        // Copy the data to the buffer
        size_t data_size = sizeof(data);
        if (*len >= data_size) {
            for (size_t i = 0; i < data_size; i++) {
                buffer[i] = data[i];
            }
            *len = data_size;
        } else {
            *len = 0; // Buffer too small
        }
    }
    
    // Module exports
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        
        create_module_definition(buffer, &len);
        
        if (len > 0) {
            bytes_sink_write(sink, buffer, &len);
        }
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
        // Log reducer call
        const char* filename = "enhanced_test.cpp";
        const char* message = "add_data reducer called";
        
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, 17, // strlen("enhanced_test.cpp") 
            100, // line number
            (const uint8_t*)message, 22 // strlen("add_data reducer called")
        );
        
        // Get table ID for TestTable
        const char* table_name = "TestTable";
        uint32_t table_id = 0;
        uint16_t status = table_id_from_name(
            (const uint8_t*)table_name, 
            9, // strlen("TestTable")
            &table_id
        );
        
        if (status == 0) {
            // Create a simple row to insert (id=1, data=42)
            uint8_t row_data[] = {
                1, 0, 0, 0,     // id = 1 (U32 little endian)
                42, 0, 0, 0     // data = 42 (U32 little endian)
            };
            size_t row_len = sizeof(row_data);
            
            // Insert row into table
            table_insert(table_id, row_data, &row_len);
        }
        
        return 0; // success
    }
}