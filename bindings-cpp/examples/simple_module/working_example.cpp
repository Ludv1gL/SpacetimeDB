#include <spacetimedb/spacetimedb.h>

// Simple table for testing
struct Person {
    uint32_t id;
    uint32_t age;
    
    // Manual BSATN serialization
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const Person& p) {
        // Serialize as product type with 2 u32 fields
        const uint8_t* id_bytes = reinterpret_cast<const uint8_t*>(&p.id);
        const uint8_t* age_bytes = reinterpret_cast<const uint8_t*>(&p.age);
        
        // Write little-endian u32 values
        for (int i = 0; i < 4; ++i) {
            buffer.push_back(id_bytes[i]);
        }
        for (int i = 0; i < 4; ++i) {
            buffer.push_back(age_bytes[i]);
        }
    }
};

// Register table
SPACETIMEDB_TABLE(Person, people, true)

// Simple reducer to test
SPACETIMEDB_REDUCER(add_person, SpacetimeDb::ReducerContext ctx, uint32_t age) {
    LOG_INFO("Adding person with age: " + std::to_string(age));
    
    Person p;
    p.id = 0; // Will be auto-assigned by DB
    p.age = age;
    
    auto table = ctx.db->table<Person>("people");
    bool success = table.insert(p);
    
    if (success) {
        LOG_INFO("Person added successfully");
    } else {
        LOG_ERROR("Failed to add person");
    }
}

// Init reducer
SPACETIMEDB_INIT(on_init) {
    LOG_INFO("Module initialized!");
}

// Required module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t description) {
        // Create minimal valid module def manually
        std::vector<uint8_t> module_def;
        SpacetimeDb::BsatnWriter writer(module_def);
        
        // RawModuleDef::V9 tag
        writer.write_u8(1);
        
        // RawModuleDefV9 structure:
        // - typespace: Typespace
        writer.write_vec_len(1); // types vector with 1 type
        
        // Define AlgebraicType::Product for Person struct
        writer.write_u8(2); // AlgebraicType::Product tag
        writer.write_vec_len(2); // 2 elements in product
        
        // Element 0: id field
        writer.write_u8(0); // Option::Some for name (0 = Some, 1 = None)
        writer.write_string("id");
        writer.write_u8(11); // AlgebraicType::U32 tag
        
        // Element 1: age field  
        writer.write_u8(0); // Option::Some for name (0 = Some, 1 = None)
        writer.write_string("age");
        writer.write_u8(11); // AlgebraicType::U32 tag
        
        // - tables: Vec<RawTableDefV9> (one table)
        writer.write_vec_len(1);
        writer.write_string("people");        // name
        writer.write_u32(0);                  // product_type_ref (refers to type 0 in typespace)
        writer.write_vec_len(0);              // primary_key
        writer.write_vec_len(0);              // indexes 
        writer.write_vec_len(0);              // constraints
        writer.write_vec_len(0);              // sequences
        writer.write_u8(1);                   // schedule (None)
        writer.write_u8(1);                   // table_type (User = 1)
        writer.write_u8(0);                   // table_access (Public)
        
        // - reducers: Vec<RawReducerDefV9> (one reducer)
        writer.write_vec_len(1);
        writer.write_string("add_person");    // name
        writer.write_vec_len(0);              // params (empty ProductType)
        writer.write_u8(1);                   // lifecycle (None)
        
        // - types: Vec<RawTypeDefV9> (empty)
        writer.write_vec_len(0);
        
        // - misc_exports: Vec<RawMiscModuleExportV9> (empty)
        writer.write_vec_len(0);
        
        // - row_level_security: Vec<RawRowLevelSecurityDefV9> (empty)
        writer.write_vec_len(0);
        
        // Write to BytesSink
        size_t len = module_def.size();
        bytes_sink_write(description, module_def.data(), &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(uint32_t id, 
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1, 
                            uint64_t timestamp,
                            uint32_t args, uint32_t error) {
        // For now, just log that reducer was called
        LOG_INFO("Reducer " + std::to_string(id) + " called");
        return 0;
    }
}