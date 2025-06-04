#include "../../sdk/include/spacetimedb/spacetimedb_autogen.h"
#include <cstdio>

// Define table structures
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const Person& value) {
        // Serialize id (little-endian)
        buffer.push_back(value.id & 0xFF);
        buffer.push_back((value.id >> 8) & 0xFF);
        buffer.push_back((value.id >> 16) & 0xFF);
        buffer.push_back((value.id >> 24) & 0xFF);
        
        // Serialize name (length + string)
        uint32_t name_len = value.name.length();
        buffer.push_back(name_len & 0xFF);
        buffer.push_back((name_len >> 8) & 0xFF);
        buffer.push_back((name_len >> 16) & 0xFF);
        buffer.push_back((name_len >> 24) & 0xFF);
        buffer.insert(buffer.end(), value.name.begin(), value.name.end());
        
        // Serialize age
        buffer.push_back(value.age);
    }
};

// Register the Person type with its fields
SPACETIMEDB_REGISTER_TYPE(Person,
    SPACETIMEDB_FIELD(Person, id),
    SPACETIMEDB_FIELD(Person, name),
    SPACETIMEDB_FIELD(Person, age)
)

// Required exports for SpacetimeDB modules
extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    std::vector<uint8_t> module_bytes;
    spacetimedb::BsatnWriter writer(module_bytes);
    
    // RawModuleDef::V9 tag
    writer.write_u8(1);
    
    // RawModuleDefV9 structure:
    
    // 1. typespace: Typespace with one type (Person)
    writer.write_vec_len(1); // 1 type
    
    // Type 0: Product type for Person
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_vec_len(3); // 3 fields
    
    // Field 0: id
    writer.write_u8(0); // Option::Some
    writer.write_string("id");
    writer.write_u8(9); // AlgebraicType::U32
    
    // Field 1: name
    writer.write_u8(0); // Option::Some
    writer.write_string("name");
    writer.write_u8(15); // AlgebraicType::String
    
    // Field 2: age
    writer.write_u8(0); // Option::Some
    writer.write_string("age");
    writer.write_u8(7); // AlgebraicType::U8
    
    writer.write_vec_len(0); // names
    
    // 2. tables: Vec<RawTableDefV9> - one table
    writer.write_vec_len(1);
    
    // Table: person
    writer.write_string("person");      // name
    writer.write_u32(0);                // product_type_ref (Type 0)
    writer.write_u8(1);                 // primary_key: None
    writer.write_vec_len(0);            // indexes
    writer.write_vec_len(0);            // constraints
    writer.write_vec_len(0);            // sequences
    writer.write_u8(1);                 // schedule: None
    writer.write_u8(0);                 // table_type: User
    writer.write_u8(0);                 // table_access: Public
    
    // 3. reducers: Vec<RawReducerDefV9>
    writer.write_vec_len(2); // 2 reducers
    
    // Reducer 0: init
    writer.write_string("init");         // name
    writer.write_vec_len(0);             // params (empty product)
    writer.write_u8(0);                  // lifecycle: Some(Init)
    writer.write_u8(0);                  // lifecycle value: Init
    
    // Reducer 1: add_person
    writer.write_string("add_person");   // name
    writer.write_vec_len(2);             // params: name (String), age (U8)
    
    // Param 0: name
    writer.write_u8(0); // Option::Some
    writer.write_string("name");
    writer.write_u8(15); // AlgebraicType::String
    
    // Param 1: age
    writer.write_u8(0); // Option::Some
    writer.write_string("age");
    writer.write_u8(7); // AlgebraicType::U8
    
    writer.write_u8(1);                  // lifecycle: None
    
    // 4. types: Vec<RawTypeDefV9>
    writer.write_vec_len(0);
    
    // 5. misc_exports: Vec<RawMiscModuleExportV9>
    writer.write_vec_len(0);
    
    // 6. row_level_security: Vec<RawRowLevelSecurityDefV9>
    writer.write_vec_len(0);
    
    // Write to sink
    size_t total_size = module_bytes.size();
    size_t written = 0;
    
    while (written < total_size) {
        size_t chunk_size = std::min(size_t(1024), total_size - written);
        size_t write_size = chunk_size;
        
        uint16_t result = bytes_sink_write(description, 
                                          module_bytes.data() + written, 
                                          &write_size);
        
        if (result != 0) {
            break;
        }
        
        written += write_size;
    }
}

extern "C" __attribute__((export_name("__call_reducer__"))) 
int16_t __call_reducer__(uint32_t id, 
                        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                        uint64_t conn_id_0, uint64_t conn_id_1, 
                        uint64_t timestamp,
                        uint32_t args, uint32_t error) {
    try {
        // Construct identity from parameters
        auto sender_identity = identity_from_params(sender_0, sender_1, sender_2, sender_3);
        
        // Construct connection ID if valid
        std::optional<spacetimedb::ConnectionId> conn_id;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            conn_id = spacetimedb::ConnectionId(conn_id_0, conn_id_1);
        }
        
        // Create reducer context
        spacetimedb::ReducerContext ctx(spacetimedb::get_module_db(), 
                                       sender_identity, 
                                       timestamp, 
                                       conn_id);
        
        // Dispatch reducers
        if (id == 0) {
            // init reducer
            LOG_INFO("Autogen final module initialized");
            
            // The autogen system has registered the Person type
            LOG_INFO("Person type registered with autogen system");
            
            // Add a default person
            Person admin{1, "Admin", 30};
            auto table_id = ctx.db->table<Person>("person").get_table_id();
            
            if (table_id != 0) {
                std::vector<uint8_t> buffer;
                Person::spacetimedb_serialize(buffer, admin);
                size_t buffer_size = buffer.size();
                uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
                
                if (result == 0) {
                    LOG_INFO("Created default admin user");
                } else {
                    LOG_ERROR("Failed to create admin user");
                }
            }
        } else if (id == 1) {
            // add_person reducer
            LOG_INFO("add_person reducer called");
            
            // Parse arguments from BytesSource
            spacetimedb::BsatnReader reader(args);
            std::string name = reader.read_string();
            uint8_t age = reader.read_u8();
            
            // Create person with auto-incrementing ID
            static uint32_t next_id = 2; // Start from 2 since admin is 1
            Person person{next_id++, name, age};
            
            // Insert into table
            auto table_id = ctx.db->table<Person>("person").get_table_id();
            if (table_id != 0) {
                std::vector<uint8_t> buffer;
                Person::spacetimedb_serialize(buffer, person);
                size_t buffer_size = buffer.size();
                uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
                
                if (result == 0) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Added person: %s (age %d) with ID %d", 
                             name.c_str(), age, person.id);
                    LOG_INFO(msg);
                } else {
                    LOG_ERROR("Failed to add person");
                }
            }
        } else {
            return 1; // Unknown reducer
        }
        
        return 0; // Success
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error in __call_reducer__: ") + e.what());
        return 1;
    }
}