/**
 * Minimal SpacetimeDB C++ SDK Test
 * 
 * Tests basic SDK functionality with proper includes.
 */

#include "spacetimedb/spacetimedb.h"

// Simple table structure
struct Person {
    uint32_t id;
    std::string name;
    int32_t age;
};

// Register the table
SPACETIMEDB_REGISTER_TABLE(Person, person, Private);

// Define field mappings
SPACETIMEDB_REGISTER_TYPE(Person,
    SPACETIMEDB_FIELD(Person, id),
    SPACETIMEDB_FIELD(Person, name),
    SPACETIMEDB_FIELD(Person, age)
)

// Simple reducer
SPACETIMEDB_REDUCER(add_person, (uint32_t id, const std::string& name, int32_t age)) {
    Person p{id, name, age};
    Person::Insert(p);
    Log::info("Added person: " + name);
}

// Init reducer
SPACETIMEDB_REDUCER(__init__) {
    Log::info("Minimal SDK test module initialized");
}

// Count reducer
SPACETIMEDB_REDUCER(count_people) {
    auto count = Person::Count();
    Log::info("Total people: " + std::to_string(count));
}