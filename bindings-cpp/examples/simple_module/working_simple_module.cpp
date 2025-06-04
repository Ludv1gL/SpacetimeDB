#include "minimal_sdk.h"
#include <cstring>

// FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    int16_t bytes_source_read(uint32_t source, uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row, size_t* row_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
}

// Implement LOG_INFO
void LOG_INFO(const std::string& msg) {
    const char* filename = "working_simple_module.cpp";
    console_log(2, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg.c_str(), msg.length());
}

// Define a simple table structure
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
};

// Register the table
SPACETIMEDB_TABLE(Person, person, true)

// Module description
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        MinimalWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace - 1 type (Person)
        w.write_u32_le(1);
        
        // Type 0: Person
        w.write_u8(2); // Product type
        w.write_u32_le(3); // 3 fields
        
        // Field 1: id
        w.write_u8(0); // Some
        w.write_string("id");
        w.write_u8(11); // U32
        
        // Field 2: name
        w.write_u8(0); // Some
        w.write_string("name");
        w.write_u8(4); // String
        
        // Field 3: age
        w.write_u8(0); // Some
        w.write_string("age");
        w.write_u8(7); // U8
        
        // Tables - 1 table
        w.write_u32_le(1);
        
        // Table: person
        w.write_string("person");
        w.write_u32_le(0); // type_ref = 0
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Reducers - 3 reducers
        w.write_u32_le(3);
        
        // Reducer: init
        w.write_string("init");
        w.write_u32_le(0); // 0 params
        w.write_u8(0); // lifecycle = Some(Init)
        w.write_u8(0); // Init variant
        
        // Reducer: insert_person
        w.write_string("insert_person");
        w.write_u32_le(2); // 2 params
        w.write_u8(0); // Some
        w.write_string("name");
        w.write_u8(4); // String
        w.write_u8(0); // Some
        w.write_string("age");
        w.write_u8(7); // U8
        w.write_u8(1); // lifecycle = None
        
        // Reducer: list_persons
        w.write_string("list_persons");
        w.write_u32_le(0); // 0 params
        w.write_u8(1); // lifecycle = None
        
        // Empty types, misc exports, row level security
        w.write_u32_le(0);
        w.write_u32_le(0);
        w.write_u32_le(0);
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        bytes_sink_write(sink, buffer.data(), &len);
    }
    
    // Helper to insert a person
    bool insert_person_row(const std::string& name, uint8_t age, uint32_t id = 0) {
        uint32_t table_id = 0;
        const char* table_name = "person";
        if (table_id_from_name((const uint8_t*)table_name, strlen(table_name), &table_id) != 0) {
            LOG_INFO("Failed to get table ID for person");
            return false;
        }
        
        MinimalWriter w;
        w.write_u32_le(id);
        w.write_string(name);
        w.write_u8(age);
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        
        uint16_t err = datastore_insert_bsatn(table_id, buffer.data(), &len);
        if (err == 0) {
            LOG_INFO("Successfully inserted person: " + name + ", age=" + std::to_string(age));
            return true;
        } else {
            LOG_INFO("Failed to insert person");
            return false;
        }
    }
    
    void init_reducer() {
        LOG_INFO("Initializing working simple module...");
        
        insert_person_row("Alice", 30, 1);
        insert_person_row("Bob", 25, 2);
        
        LOG_INFO("Initial data loaded");
    }
    
    void insert_person_reducer(uint32_t args) {
        LOG_INFO("insert_person_reducer called");
        
        // Read args using the same pattern as minimal_module_test.cpp
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        LOG_INFO("Read " + std::to_string(len) + " bytes of arguments");
        
        // Simple reader like MinimalReader
        size_t pos = 0;
        
        // Read string (name)
        if (pos + 4 > len) {
            LOG_INFO("Not enough data for string length");
            return;
        }
        uint32_t str_len = 0;
        for (int i = 0; i < 4; i++) {
            str_len |= (uint32_t(buffer[pos++]) << (i * 8));
        }
        
        LOG_INFO("Name length: " + std::to_string(str_len));
        
        if (pos + str_len > len) {
            LOG_INFO("Not enough data for string content");
            return;
        }
        std::string name(reinterpret_cast<const char*>(buffer + pos), str_len);
        pos += str_len;
        
        // Read u8 (age)
        if (pos >= len) {
            LOG_INFO("Not enough data for age");
            return;
        }
        uint8_t age = buffer[pos++];
        
        LOG_INFO("Parsed: name='" + name + "', age=" + std::to_string(age));
        insert_person_row(name, age);
    }
    
    void list_persons_reducer() {
        LOG_INFO("List persons reducer called");
        LOG_INFO("Person listing not yet implemented (requires table iteration)");
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id,
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        
        switch (id) {
            case 0: init_reducer(); return 0;
            case 1: insert_person_reducer(args); return 0;
            case 2: list_persons_reducer(); return 0;
            default: return -1;
        }
    }
}

// Implement the reducers using the macro (for testing)
SPACETIMEDB_REDUCER(init, ctx) {
    LOG_INFO("Init reducer called via macro");
}

SPACETIMEDB_REDUCER(insert_person, ctx) {
    LOG_INFO("Insert person reducer called via macro");
    
    // Use the table handle
    Person person{0, "Test Person", 25};
    person_table.insert(person);
    
    LOG_INFO("Inserted person via table handle");
}

SPACETIMEDB_REDUCER(list_persons, ctx) {
    LOG_INFO("List persons reducer called via macro");
    
    LOG_INFO("Total persons: " + std::to_string(person_table.count()));
    
    for (const auto& person : person_table.iter()) {
        LOG_INFO("Person: ID=" + std::to_string(person.id) + 
                 ", Name=" + person.name + 
                 ", Age=" + std::to_string(person.age));
    }
}