#include <spacetimedb/spacetimedb.h>
#include <iostream>

// Simple test struct
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
};

// Register fields for Person
SPACETIMEDB_REGISTER_FIELDS(Person,
    SPACETIMEDB_FIELD(Person, id, uint32_t);
    SPACETIMEDB_FIELD(Person, name, std::string);
    SPACETIMEDB_FIELD(Person, age, uint8_t);
)

// Declare table
SPACETIMEDB_TABLE(Person, person, true)

// Test reducer that demonstrates all table operations
SPACETIMEDB_REDUCER(test_table_operations, spacetimedb::ReducerContext ctx) {
    auto person_table = ctx.db.table<Person>("person");
    
    // 1. Insert some test data
    Person alice = person_table.insert({1, "Alice", 30});
    Person bob = person_table.insert({2, "Bob", 25});
    Person charlie = person_table.insert({3, "Charlie", 35});
    
    LOG_INFO("Inserted 3 people");
    
    // 2. Count rows
    uint64_t count = person_table.count();
    LOG_INFO("Table has " + std::to_string(count) + " rows");
    
    // 3. Iterate over all rows
    LOG_INFO("Iterating over all people:");
    for (const auto& person : person_table.iter()) {
        LOG_INFO("  - " + person.name + " (age " + std::to_string(person.age) + ")");
    }
    
    // 4. Update a row
    Person updated_alice = {1, "Alice Smith", 31};
    if (person_table.update(alice, updated_alice)) {
        LOG_INFO("Updated Alice's record");
    }
    
    // 5. Delete a row
    if (person_table.delete_by_value(bob)) {
        LOG_INFO("Deleted Bob's record");
    }
    
    // 6. Count again
    count = person_table.count();
    LOG_INFO("Table now has " + std::to_string(count) + " rows");
}

// Reducer to clear the table
SPACETIMEDB_REDUCER(clear_people, spacetimedb::ReducerContext ctx) {
    auto person_table = ctx.db.table<Person>("person");
    
    // Delete all rows by iterating
    std::vector<Person> to_delete;
    for (const auto& person : person_table.iter()) {
        to_delete.push_back(person);
    }
    
    for (const auto& person : to_delete) {
        person_table.delete_by_value(person);
    }
    
    LOG_INFO("Cleared all people from table");
}

// Reducer to find people by age range
SPACETIMEDB_REDUCER(find_by_age_range, spacetimedb::ReducerContext ctx, uint8_t min_age, uint8_t max_age) {
    auto person_table = ctx.db.table<Person>("person");
    
    LOG_INFO("Finding people between ages " + std::to_string(min_age) + " and " + std::to_string(max_age));
    
    uint32_t found = 0;
    for (const auto& person : person_table.iter()) {
        if (person.age >= min_age && person.age <= max_age) {
            LOG_INFO("  - Found: " + person.name + " (age " + std::to_string(person.age) + ")");
            found++;
        }
    }
    
    LOG_INFO("Found " + std::to_string(found) + " people in age range");
}