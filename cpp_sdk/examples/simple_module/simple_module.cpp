#include <spacetimedb/spacetimedb.h>

// Define a simple table structure
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
};

// Register BSATN serialization
SPACETIMEDB_REGISTER_FIELDS(Person,
    SPACETIMEDB_FIELD(Person, id, uint32_t);
    SPACETIMEDB_FIELD(Person, name, std::string);
    SPACETIMEDB_FIELD(Person, age, uint8_t);
)

// Specialize type generation for Person
template<>
std::vector<uint8_t> spacetimedb_generate_type<Person>() {
    using namespace SpacetimeDb::bsatn;
    Writer writer;
    
    // Write Product type tag (2)
    writer.write_u8(2);
    
    // Write number of fields (3)
    writer.write_u32_le(3);
    
    // TODO: Properly write field information
    // For now, this is a placeholder
    
    return writer.take_buffer();
}

// Register the table - new simplified macro
SPACETIMEDB_TABLE(Person, person, true)

// Define a reducer to insert a person
SPACETIMEDB_REDUCER(insert_person, ctx, std::string name, uint8_t age) {
    // Get the person table handle
    person__TableHandle table;
    
    // Insert with auto-generated ID
    Person person{0, name, age};
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