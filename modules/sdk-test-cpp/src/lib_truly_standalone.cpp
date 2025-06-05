// Truly standalone C++ module - no std library dependencies
#include <cstdint>
#include <cstring>

// Import functions from SpacetimeDB
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, const uint8_t* row, size_t row_len);
}

// Simple BSATN writer for basic types
class SimpleBsatnWriter {
private:
    uint8_t buffer[1024];
    size_t pos;
    
public:
    SimpleBsatnWriter() : pos(0) {}
    
    void write_u8(uint8_t val) {
        buffer[pos++] = val;
    }
    
    void write_u32_le(uint32_t val) {
        buffer[pos++] = val & 0xFF;
        buffer[pos++] = (val >> 8) & 0xFF;
        buffer[pos++] = (val >> 16) & 0xFF;
        buffer[pos++] = (val >> 24) & 0xFF;
    }
    
    void write_bytes(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            buffer[pos++] = data[i];
        }
    }
    
    void write_string(const char* str) {
        size_t len = strlen(str);
        write_u32_le(len);
        write_bytes((const uint8_t*)str, len);
    }
    
    const uint8_t* get_data() const { return buffer; }
    size_t get_size() const { return pos; }
};

// Helper functions
void write_to_sink(uint32_t sink, const uint8_t* data, size_t len) {
    size_t written = len;
    bytes_sink_write(sink, data, &written);
}

void log_info(const char* message) {
    console_log(2, // INFO
               (const uint8_t*)"module", 6,
               (const uint8_t*)__FILE__, strlen(__FILE__),
               __LINE__,
               (const uint8_t*)message, strlen(message));
}

// Global table ID cache
static uint32_t person_table_id = 0;

// Module exports
extern "C" {

__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    SimpleBsatnWriter writer;
    
    // Write RawModuleDef::V9 (variant 1)
    writer.write_u8(1);
    
    // Typespace with 1 type (Person struct)
    writer.write_u32_le(1);
    
    // Type 0: ProductType (variant 2)
    writer.write_u8(2);
    
    // ProductType with 3 elements
    writer.write_u32_le(3);
    
    // Element 0: id (u32)
    writer.write_u8(0); // Some
    writer.write_string("id");
    writer.write_u8(11); // U32 = 11
    
    // Element 1: name (string)
    writer.write_u8(0); // Some
    writer.write_string("name");
    writer.write_u8(4); // String = 4
    
    // Element 2: age (u8)
    writer.write_u8(0); // Some
    writer.write_string("age");
    writer.write_u8(7); // U8 = 7
    
    // Tables: 1 table (Person)
    writer.write_u32_le(1);
    
    // Table: Person
    writer.write_string("Person");
    writer.write_u32_le(0); // product_type_ref = 0
    writer.write_u32_le(0); // primary_key (empty)
    writer.write_u32_le(0); // indexes (empty)
    writer.write_u32_le(0); // constraints (empty)
    writer.write_u32_le(0); // sequences (empty)
    writer.write_u8(1);     // schedule: None
    writer.write_u8(1);     // table_type: User
    writer.write_u8(0);     // table_access: Public
    
    // Reducers: 2 reducers
    writer.write_u32_le(2);
    
    // Reducer 0: add_person(id: u32, name: string, age: u8)
    writer.write_string("add_person");
    writer.write_u32_le(3); // params: 3 fields
    // Field 0: id
    writer.write_u8(0); // Some
    writer.write_string("id");
    writer.write_u8(11); // U32 = 11
    // Field 1: name
    writer.write_u8(0); // Some
    writer.write_string("name");
    writer.write_u8(4); // String = 4
    // Field 2: age
    writer.write_u8(0); // Some
    writer.write_string("age");
    writer.write_u8(7); // U8 = 7
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 1: list_people()
    writer.write_string("list_people");
    writer.write_u32_le(0); // params: empty
    writer.write_u8(1);     // lifecycle: None
    
    // Empty arrays for the rest
    writer.write_u32_le(0); // types
    writer.write_u32_le(0); // misc_exports
    writer.write_u32_le(0); // row_level_security
    
    // Write to sink
    write_to_sink(sink, writer.get_data(), writer.get_size());
}

__attribute__((export_name("__call_reducer__")))
int32_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    // Get table ID for Person table if not cached
    if (person_table_id == 0) {
        const char* table_name = "Person";
        auto err = table_id_from_name(
            (const uint8_t*)table_name, 
            strlen(table_name), 
            &person_table_id
        );
        if (err != 0) {
            log_info("Failed to get Person table ID");
            return -1;
        }
    }
    
    if (id == 0) { // add_person reducer
        // Read arguments from source
        uint8_t args_buffer[512];
        size_t args_len = sizeof(args_buffer);
        bytes_source_read(args, args_buffer, &args_len);
        
        // Simple argument parsing (assumes correct format)
        size_t pos = 0;
        
        // Read id (u32)
        uint32_t person_id = args_buffer[pos] | 
                           (args_buffer[pos+1] << 8) |
                           (args_buffer[pos+2] << 16) |
                           (args_buffer[pos+3] << 24);
        pos += 4;
        
        // Read name (string)
        uint32_t name_len = args_buffer[pos] | 
                          (args_buffer[pos+1] << 8) |
                          (args_buffer[pos+2] << 16) |
                          (args_buffer[pos+3] << 24);
        pos += 4;
        char name[256];
        for (uint32_t i = 0; i < name_len && i < 255; i++) {
            name[i] = args_buffer[pos + i];
        }
        name[name_len] = '\0';
        pos += name_len;
        
        // Read age (u8)
        uint8_t age = args_buffer[pos];
        
        // Create Person row in BSATN format
        SimpleBsatnWriter writer;
        writer.write_u32_le(person_id);
        writer.write_string(name);
        writer.write_u8(age);
        
        // Insert into table
        auto err = datastore_insert_bsatn(person_table_id, writer.get_data(), writer.get_size());
        
        if (err == 0) {
            log_info("Added person successfully");
        } else {
            log_info("Failed to add person");
            return -1;
        }
        
        return 0;
    }
    else if (id == 1) { // list_people reducer
        log_info("Listing all people in the table");
        // In a real implementation, we would iterate the table here
        return 0;
    }
    
    return -999; // Unknown reducer
}

} // extern "C"