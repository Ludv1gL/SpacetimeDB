#define SPACETIMEDB_CUSTOM_REDUCER_CONTEXT
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

// Define a test table with primitive types
struct TestA {
    uint64_t x;
    uint32_t y;
    uint16_t z;
    
    // BSATN serialization
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        writer.write_u64_le(x);
        writer.write_u32_le(y);
        writer.write_u16_le(z);
    }
    
    // BSATN deserialization
    void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        x = reader.read_u64_le();
        y = reader.read_u32_le();
        z = reader.read_u16_le();
    }
};

// Register type with AlgebraicType system for Person
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

// Register type with AlgebraicType system for TestA
template<>
std::vector<uint8_t> spacetimedb_generate_type<TestA>() {
    SpacetimeDb::bsatn::Writer writer;
    writer.write_u8(2);  // Product type
    writer.write_u32_le(3); // 3 fields
    
    // Field 1: x
    writer.write_u8(0);  // Some (field name present)
    writer.write_string("x");
    writer.write_u8(16); // U64
    
    // Field 2: y
    writer.write_u8(0);  // Some
    writer.write_string("y");
    writer.write_u8(14); // U32
    
    // Field 3: z
    writer.write_u8(0);  // Some
    writer.write_string("z");
    writer.write_u8(13); // U16
    
    return writer.take_buffer();
}


// Register the tables - new simplified macro
SPACETIMEDB_TABLE(Person, person, true)
SPACETIMEDB_TABLE(TestA, test_a, true)

// Define reducers using the simple pattern that works

// Insert person reducer - hardcoded for now like working example
SPACETIMEDB_REDUCER(insert_person, ctx) {
    // Get the person table handle
    person__TableHandle table;
    
    // Insert with auto-generated ID - hardcoded for now
    Person person{0, "Test Person", 25};
    person = table.insert(person);
    
    LOG_INFO("Inserted person with ID: " + std::to_string(person.id));
}

// Insert test_a reducer
SPACETIMEDB_REDUCER(insert_test_a, ctx) {
    // Get the test_a table handle
    test_a__TableHandle table;
    
    // Insert test data
    TestA test{100, 200, 300};
    test = table.insert(test);
    
    LOG_INFO("Inserted test_a: x=" + std::to_string(test.x) + 
             ", y=" + std::to_string(test.y) + 
             ", z=" + std::to_string(test.z));
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

// Define a reducer to list all test_a records
SPACETIMEDB_REDUCER(list_test_a, ctx) {
    // Get the test_a table handle
    test_a__TableHandle table;
    
    LOG_INFO("Total test_a records: " + std::to_string(table.count()));
    
    // Iterate over all test_a records
    for (const auto& test : table.iter()) {
        LOG_INFO("TestA: x=" + std::to_string(test.x) + 
                 ", y=" + std::to_string(test.y) + 
                 ", z=" + std::to_string(test.z));
    }
}

// Define init reducer to set up initial data
SPACETIMEDB_REDUCER(init, ctx) {
    LOG_INFO("Initializing module...");
    
    person__TableHandle personTable;
    personTable.insert(Person{0, "Alice", 30});
    personTable.insert(Person{0, "Bob", 25});
    
    test_a__TableHandle testTable;
    testTable.insert(TestA{1000, 2000, 3000});
    testTable.insert(TestA{4000, 5000, 6000});
    
    LOG_INFO("Initial data loaded");
}