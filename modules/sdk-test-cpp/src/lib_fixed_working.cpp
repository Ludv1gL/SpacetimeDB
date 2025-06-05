// Working C++ module with proper types and library support
#include "spacetimedb/spacetimedb.h"

// Define a Person table
SPACETIMEDB_TABLE(Person)
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
};

// Register the Person table
SPACETIMEDB_REGISTER_TABLE(Person);

// Add person reducer
SPACETIMEDB_REDUCER(add_person)
void add_person(ReducerContext& ctx, uint32_t id, std::string name, uint8_t age) {
    Person person{id, name, age};
    Person::insert(ctx, person);
    spacetimedb::log(SpacetimeLogLevel::Info, 
        "Added person: " + name + " (id=" + std::to_string(id) + ", age=" + std::to_string(age) + ")");
}

// List people reducer
SPACETIMEDB_REDUCER(list_people)
void list_people(ReducerContext& ctx) {
    spacetimedb::log(SpacetimeLogLevel::Info, "Listing all people:");
    for (const auto& person : Person::table(ctx)) {
        spacetimedb::log(SpacetimeLogLevel::Info, 
            "  Person: " + person.name + " (id=" + std::to_string(person.id) + ", age=" + std::to_string(person.age) + ")");
    }
}