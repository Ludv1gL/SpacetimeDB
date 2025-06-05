// Standalone working C++ module with tables and reducers
// No library dependencies - direct FFI implementation
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// Import functions from SpacetimeDB
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, const uint8_t* row, uint32_t row_len);
}

// Helper functions
void write_to_sink(uint32_t sink, const uint8_t* data, size_t len) {
    size_t written = len;
    bytes_sink_write(sink, data, &written);
}

void log_info(const std::string& message) {
    console_log(2, // INFO level
               (const uint8_t*)"module", 6,
               (const uint8_t*)__FILE__, strlen(__FILE__),
               __LINE__,
               (const uint8_t*)message.c_str(), message.length());
}

void log_error(const std::string& message) {
    console_log(0, // ERROR level
               (const uint8_t*)"module", 6,
               (const uint8_t*)__FILE__, strlen(__FILE__),
               __LINE__,
               (const uint8_t*)message.c_str(), message.length());
}

// Simple BSATN writer
class BsatnWriter {
    std::vector<uint8_t> buffer;
    
public:
    void write_u8(uint8_t value) {
        buffer.push_back(value);
    }
    
    void write_u32_le(uint32_t value) {
        buffer.push_back(value & 0xFF);
        buffer.push_back((value >> 8) & 0xFF);
        buffer.push_back((value >> 16) & 0xFF);
        buffer.push_back((value >> 24) & 0xFF);
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.length());
        buffer.insert(buffer.end(), str.begin(), str.end());
    }
    
    void write_bytes(const uint8_t* data, size_t len) {
        buffer.insert(buffer.end(), data, data + len);
    }
    
    const std::vector<uint8_t>& get_buffer() const { return buffer; }
};

// Simple BSATN reader
class BsatnReader {
    const uint8_t* data;
    size_t size;
    size_t pos;
    
public:
    BsatnReader(const uint8_t* d, size_t s) : data(d), size(s), pos(0) {}
    
    uint8_t read_u8() {
        if (pos >= size) return 0;
        return data[pos++];
    }
    
    uint32_t read_u32_le() {
        if (pos + 4 > size) return 0;
        uint32_t value = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24);
        pos += 4;
        return value;
    }
    
    std::string read_string() {
        uint32_t len = read_u32_le();
        if (pos + len > size) return "";
        std::string str((const char*)(data + pos), len);
        pos += len;
        return str;
    }
};

// Global table ID cache
static uint32_t person_table_id = 0;

// Module exports
extern "C" {

// Describe module with Person table and two reducers
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    BsatnWriter writer;
    
    // Write RawModuleDef::V9 (variant 1)
    writer.write_u8(1);
    
    // Typespace with 1 type (Person struct)
    writer.write_u32_le(1);
    
    // Type 0: ProductType for Person (variant 2)
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
    // params: ProductType with 3 fields
    writer.write_u32_le(3);
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
    writer.write_u32_le(0); // params: empty ProductType
    writer.write_u8(1);     // lifecycle: None
    
    // Empty arrays for the rest
    writer.write_u32_le(0); // types
    writer.write_u32_le(0); // misc_exports
    writer.write_u32_le(0); // row_level_security
    
    // Write to sink
    const auto& buffer = writer.get_buffer();
    write_to_sink(sink, buffer.data(), buffer.size());
}

// Handle reducer calls
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
            reinterpret_cast<const uint8_t*>(table_name), 
            strlen(table_name), 
            &person_table_id
        );
        if (err != 0) {
            log_error("Failed to get Person table ID");
            return -1;
        }
    }
    
    if (id == 0) { // add_person reducer
        // Read arguments from source
        size_t args_len = 0;
        bytes_source_read(args, nullptr, &args_len);
        
        std::vector<uint8_t> args_buffer(args_len);
        bytes_source_read(args, args_buffer.data(), &args_len);
        
        BsatnReader reader(args_buffer.data(), args_len);
        
        // Read the three arguments
        uint32_t person_id = reader.read_u32_le();
        std::string name = reader.read_string();
        uint8_t age = reader.read_u8();
        
        // Create Person row in BSATN format
        BsatnWriter writer;
        writer.write_u32_le(person_id);
        writer.write_string(name);
        writer.write_u8(age);
        
        // Insert into table
        const auto& row_buffer = writer.get_buffer();
        auto err = datastore_insert_bsatn(person_table_id, row_buffer.data(), row_buffer.size());
        
        if (err == 0) {
            log_info("Added person: " + name + " (id=" + std::to_string(person_id) + ", age=" + std::to_string(age) + ")");
        } else {
            log_error("Failed to add person: " + name);
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