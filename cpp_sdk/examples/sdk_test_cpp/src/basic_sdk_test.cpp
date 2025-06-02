// Basic SDK test without standard library dependencies
// This demonstrates core SpacetimeDB functionality using only basic C types

extern "C" {
    // Core SpacetimeDB imports from spacetime_10.0
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    unsigned short bytes_sink_write(unsigned int sink, const unsigned char* buffer_ptr, unsigned long* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(
        unsigned char level,
        const unsigned char *target_ptr, unsigned int target_len,
        const unsigned char *filename_ptr, unsigned int filename_len, 
        unsigned int line_number,
        const unsigned char *message_ptr, unsigned int message_len
    );
    
    __attribute__((import_module("spacetime_10.0"), import_name("identity")))
    void identity(unsigned char* out_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    unsigned short table_id_from_name(const unsigned char* name, unsigned int name_len, unsigned int* out);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_insert")))
    unsigned short table_insert(unsigned int table_id, const unsigned char* row_ptr, unsigned long* row_len_ptr);
    
    // Helper function to get string length
    unsigned int str_len(const char* str) {
        unsigned int len = 0;
        while (str[len] != '\0') len++;
        return len;
    }
    
    // Helper function to copy memory
    void mem_copy(unsigned char* dest, const unsigned char* src, unsigned int len) {
        for (unsigned int i = 0; i < len; i++) {
            dest[i] = src[i];
        }
    }
    
    // Create module definition with UserTable
    void create_user_module_definition(unsigned char* buffer, unsigned long* len) {
        // RawModuleDef::V9 with UserTable (id: U32, name: String)
        unsigned char data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            
            // Tables (1 table)
            1, 0, 0, 0,     // tables vector length = 1
            
            // Table definition: UserTable
            9, 0, 0, 0,     // table name length = 9
            'U', 's', 'e', 'r', 'T', 'a', 'b', 'l', 'e',  // table name "UserTable"
            
            // Product type with 2 fields  
            1,              // ProductType tag = 1
            2, 0, 0, 0,     // elements length = 2
            
            // Field 1: id (U32)
            2, 0, 0, 0,     // field name length = 2  
            'i', 'd',       // field name "id"
            8,              // AlgebraicType::U32 = 8
            
            // Field 2: name (String)  
            4, 0, 0, 0,     // field name length = 4
            'n', 'a', 'm', 'e',  // field name "name"
            12,             // AlgebraicType::String = 12
            
            // Table properties
            1,              // is_public = true
            0,              // unique_id = 0
            0, 0, 0, 0,     // indexes (empty vector)
            0, 0, 0, 0,     // constraints (empty vector)
            0, 0, 0, 0,     // sequences (empty vector)
            
            // Reducers (2 reducers)
            2, 0, 0, 0,     // reducers vector length = 2
            
            // Reducer 1: add_user
            8, 0, 0, 0,     // reducer name length = 8
            'a', 'd', 'd', '_', 'u', 's', 'e', 'r',  // reducer name "add_user"
            0,              // ReducerKind::Update = 0
            1, 0, 0, 0,     // args vector length = 1
            // Argument: name (String)
            4, 0, 0, 0,     // arg name length = 4
            'n', 'a', 'm', 'e',  // arg name "name"
            12,             // AlgebraicType::String = 12
            
            // Reducer 2: get_user
            8, 0, 0, 0,     // reducer name length = 8
            'g', 'e', 't', '_', 'u', 's', 'e', 'r',  // reducer name "get_user"
            0,              // ReducerKind::Update = 0
            1, 0, 0, 0,     // args vector length = 1
            // Argument: id (U32)
            2, 0, 0, 0,     // arg name length = 2
            'i', 'd',       // arg name "id"
            8,              // AlgebraicType::U32 = 8
            
            0, 0, 0, 0,     // types (empty vector)
            0, 0, 0, 0,     // misc_exports (empty vector)
            0, 0, 0, 0      // row_level_security (empty vector)
        };
        
        unsigned long data_size = sizeof(data);
        if (*len >= data_size) {
            mem_copy(buffer, data, data_size);
            *len = data_size;
        } else {
            *len = 0; // Buffer too small
        }
    }
    
    // Module export: __describe_module__
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(unsigned int sink) {
        unsigned char buffer[1024];
        unsigned long len = sizeof(buffer);
        
        create_user_module_definition(buffer, &len);
        
        if (len > 0) {
            bytes_sink_write(sink, buffer, &len);
        }
    }
    
    // Module export: __call_reducer__
    __attribute__((export_name("__call_reducer__")))
    short __call_reducer__(
        unsigned int id,
        unsigned long long sender_0, unsigned long long sender_1, 
        unsigned long long sender_2, unsigned long long sender_3,
        unsigned long long conn_id_0, unsigned long long conn_id_1,
        unsigned long long timestamp, 
        unsigned int args_source, 
        unsigned int error_sink
    ) {
        const char* filename = "basic_sdk_test.cpp";
        const char* message = "Basic reducer called";
        
        // Log the reducer call
        console_log(
            1, // info level
            (const unsigned char*)"", 0,
            (const unsigned char*)filename, str_len(filename),
            150, // line number
            (const unsigned char*)message, str_len(message)
        );
        
        // For demonstration, let's try to get the UserTable ID
        const char* table_name = "UserTable";
        unsigned int table_id = 0;
        unsigned short status = table_id_from_name(
            (const unsigned char*)table_name, 
            str_len(table_name),
            &table_id
        );
        
        if (status == 0) {
            // Create a simple user row (id=1, name="test")
            // Format: [id: 4 bytes][name_len: 4 bytes][name_data: variable]
            unsigned char row_data[16];
            
            // id = 1 (U32 little endian)
            row_data[0] = 1; row_data[1] = 0; row_data[2] = 0; row_data[3] = 0;
            
            // name = "test" (String: length + data)
            row_data[4] = 4; row_data[5] = 0; row_data[6] = 0; row_data[7] = 0; // length = 4
            row_data[8] = 't'; row_data[9] = 'e'; row_data[10] = 's'; row_data[11] = 't'; // "test"
            
            unsigned long row_len = 12; // 4 bytes id + 4 bytes length + 4 bytes data
            
            // Insert the row
            table_insert(table_id, row_data, &row_len);
            
            const char* success_msg = "User added successfully";
            console_log(
                1, // info level
                (const unsigned char*)"", 0,
                (const unsigned char*)filename, str_len(filename),
                170, // line number
                (const unsigned char*)success_msg, str_len(success_msg)
            );
        }
        
        return 0; // success
    }
}