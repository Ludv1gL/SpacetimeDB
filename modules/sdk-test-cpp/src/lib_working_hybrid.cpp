// Working C++ module using hybrid approach - library + direct exports
#include "spacetimedb/spacetimedb.h"
#include <cstring>
#include <vector>

// Define a Person table structure
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
    
    // BSATN serialization
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        writer.write_u32_le(id);
        writer.write_string(name);
        writer.write_u8(age);
    }
    
    // BSATN deserialization
    static Person bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        Person p;
        p.id = reader.read_u32_le();
        p.name = reader.read_string();
        p.age = reader.read_u8();
        return p;
    }
};

// Global table handles - in a real implementation these would be managed better
static uint32_t person_table_id = 0;

// Reducer implementations
void add_person(uint32_t id, const std::string& name, uint8_t age) {
    Person p;
    p.id = id;
    p.name = name;
    p.age = age;
    
    // Serialize the person
    SpacetimeDb::bsatn::Writer writer;
    p.bsatn_serialize(writer);
    auto buffer = writer.get_buffer();
    
    // Insert into table using FFI
    auto err = SpacetimeDb::Internal::FFI::datastore_insert_bsatn(
        person_table_id,
        buffer.data(),
        buffer.size()
    );
    
    if (err == SpacetimeDb::Internal::FFI::Errno::OK) {
        SpacetimeDb::log_info("Added person: " + name);
    } else {
        SpacetimeDb::log_error("Failed to add person: " + name);
    }
}

void list_people() {
    SpacetimeDb::log_info("Listing all people in the table");
    
    // In a real implementation, we would iterate the table
    // For now, just log that we're listing
}

// Module exports
extern "C" {

// Describe module with Person table and two reducers
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    SpacetimeDb::bsatn::Writer writer;
    
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
    writer.write_u32_le(2); // name length
    writer.write_string("id");
    writer.write_u8(14); // U32
    
    // Element 1: name (string)
    writer.write_u8(0); // Some
    writer.write_u32_le(4); // name length
    writer.write_string("name");
    writer.write_u8(4); // String
    
    // Element 2: age (u8)
    writer.write_u8(0); // Some
    writer.write_u32_le(3); // name length
    writer.write_string("age");
    writer.write_u8(12); // U8
    
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
    writer.write_u8(14); // U32
    // Field 1: name
    writer.write_u8(0); // Some
    writer.write_string("name");
    writer.write_u8(4); // String
    // Field 2: age
    writer.write_u8(0); // Some
    writer.write_string("age");
    writer.write_u8(12); // U8
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
    auto buffer = writer.get_buffer();
    size_t written = buffer.size();
    SpacetimeDb::Internal::FFI::bytes_sink_write(sink, buffer.data(), &written);
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
    try {
        // Get table ID for Person table
        if (person_table_id == 0) {
            auto err = SpacetimeDb::Internal::FFI::table_id_from_name(
                reinterpret_cast<const uint8_t*>("Person"), 6, &person_table_id
            );
            if (err != SpacetimeDb::Internal::FFI::Errno::OK) {
                SpacetimeDb::log_error("Failed to get Person table ID");
                return -1;
            }
        }
        
        if (id == 0) { // add_person reducer
            // Deserialize arguments
            size_t args_len = 0;
            SpacetimeDb::Internal::FFI::bytes_source_read(args, nullptr, &args_len);
            
            std::vector<uint8_t> args_buffer(args_len);
            SpacetimeDb::Internal::FFI::bytes_source_read(args, args_buffer.data(), &args_len);
            
            SpacetimeDb::bsatn::Reader reader(args_buffer.data(), args_len);
            
            // Read the three arguments
            uint32_t id = reader.read_u32_le();
            std::string name = reader.read_string();
            uint8_t age = reader.read_u8();
            
            // Call the reducer
            add_person(id, name, age);
            
            return 0;
        }
        else if (id == 1) { // list_people reducer
            list_people();
            return 0;
        }
        
        return -999; // Unknown reducer
    }
    catch (const std::exception& e) {
        SpacetimeDb::log_error(std::string("Reducer error: ") + e.what());
        return -1;
    }
}

} // extern "C"