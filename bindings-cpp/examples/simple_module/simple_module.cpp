#include <spacetimedb/spacetimedb.h>

// Define a simple table structure
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
    void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        id = reader.read_u32_le();
        name = reader.read_string();
        age = reader.read_u8();
    }
};

// Register type with AlgebraicType system
template<>
std::vector<uint8_t> spacetimedb_generate_type<Person>() {
    SpacetimeDb::bsatn::Writer writer;
    writer.write_u8(2);  // Product type
    writer.write_u32_le(3); // 3 fields
    
    // Field 1: id
    writer.write_u8(0);  // Some (field name present)
    writer.write_string("id");
    writer.write_u8(14); // U32
    
    // Field 2: name
    writer.write_u8(0);  // Some
    writer.write_string("name");
    writer.write_u8(4);  // String
    
    // Field 3: age
    writer.write_u8(0);  // Some
    writer.write_string("age");
    writer.write_u8(12); // U8
    
    return writer.take_buffer();
}


// Register the table - new simplified macro
SPACETIMEDB_TABLE(Person, person, true)

// Define a reducer to insert a person (simplified without args for now)
SPACETIMEDB_REDUCER(insert_person, ctx) {
    // Get the person table handle
    person__TableHandle table;
    
    // Insert with auto-generated ID - hardcoded for now
    Person person{0, "Test Person", 25};
    person = table.insert(person);
    
    LOG_INFO("Inserted person with ID: " + std::to_string(person.id));
}

// Define a reducer to list all persons
SPACETIMEDB_REDUCER(list_persons, ctx) {
    // Get the person table handle
    person__TableHandle table;
    
    LOG_INFO("Total persons: " + std::to_string(table.count()));
    
    // Iterate over all persons
    for (const auto& person : table.iter()) {
        LOG_INFO("Person: ID=" + std::to_string(person.id) + 
                 ", Name=" + person.name + 
                 ", Age=" + std::to_string(person.age));
    }
}

// Define init reducer to set up initial data
SPACETIMEDB_REDUCER(init, ctx) {
    LOG_INFO("Initializing module...");
    
    person__TableHandle table;
    table.insert(Person{0, "Alice", 30});
    table.insert(Person{0, "Bob", 25});
    
    LOG_INFO("Initial data loaded");
}