// Test module without using macros - just direct library usage
#include "spacetimedb/spacetimedb.h"

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
    static Person bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        Person p;
        p.id = reader.read_u32_le();
        p.name = reader.read_string();
        p.age = reader.read_u8();
        return p;
    }
};

// Simple reducer function
void add_person(SpacetimeDb::ReducerContext& ctx, uint32_t id, const std::string& name, uint8_t age) {
    Person p;
    p.id = id;
    p.name = name;
    p.age = age;
    
    // Just log for now
    SpacetimeDb::log_info("Added person: " + name);
}

// Manual module initialization
namespace {
    struct ModuleInit {
        ModuleInit() {
            // Register tables and reducers manually
            // This is what the macros would do
        }
    };
    static ModuleInit module_init;
}