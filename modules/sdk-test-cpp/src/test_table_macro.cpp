// Test file to check SPACETIMEDB_TABLE macro
#include "spacetimedb/spacetimedb.h"
#include "spacetimedb/algebraic_type_wrapper.h"

// Define a simple table structure
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
    
    // BSATN serialization required by the macro
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        writer.write_u32_le(id);
        writer.write_string(name);
        writer.write_u8(age);
    }
    
    // BSATN deserialization required by the macro
    static Person bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        Person p;
        p.id = reader.read_u32_le();
        p.name = reader.read_string();
        p.age = reader.read_u8();
        return p;
    }
};

// Register the table using the macro
SPACETIMEDB_TABLE(Person, "person", true);

// Test if we can build with just the table macro

// BSATN traits for Person
namespace SpacetimeDb::bsatn {
    template<>
    struct bsatn_traits<Person> {
        static void serialize(Writer& writer, const Person& value) {
            value.bsatn_serialize(writer);
        }
        
        static Person deserialize(Reader& reader) {
            return Person::bsatn_deserialize(reader);
        }
        
        static SpacetimeDb::Internal::AlgebraicType algebraic_type() {
            // Create a product type for Person
            // For now, return a simple product type
            return SpacetimeDb::Internal::AlgebraicType();
        }
    };
}