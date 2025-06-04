#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/bsatn/bsatn.h>

// Simple struct with primitive types only
struct SimpleData {
    uint32_t id;
    uint8_t value;
};

// Minimal working BSATN specialization - just for compilation
namespace SpacetimeDb::bsatn {
    template<>
    struct bsatn_traits<SimpleData> {
        static void serialize(SpacetimeDb::bsatn::Writer& writer, const SimpleData& value) {
            SpacetimeDb::bsatn::serialize(writer, value.id);
            SpacetimeDb::bsatn::serialize(writer, value.value);
        }
        
        static SimpleData deserialize(SpacetimeDb::bsatn::Reader& reader) {
            SimpleData result;
            result.id = SpacetimeDb::bsatn::deserialize<uint32_t>(reader);
            result.value = SpacetimeDb::bsatn::deserialize<uint8_t>(reader);
            return result;
        }
        
        // Simple algebraic type - empty product type for now
        static SpacetimeDb::bsatn::AlgebraicType algebraic_type() {
            std::vector<SpacetimeDb::bsatn::AggregateElement> elements;
            // Use empty elements - this should at least allow module description generation
            return SpacetimeDb::bsatn::AlgebraicType::make_product(
                std::make_unique<SpacetimeDb::bsatn::ProductType>(std::move(elements))
            );
        }
    };
}

// Declare table
SPACETIMEDB_TABLE(SimpleData, simple_data, true)

// Test reducers
SPACETIMEDB_REDUCER(test_working_struct, spacetimedb::ReducerContext ctx, uint32_t id, uint8_t value) {
    LOG_INFO("Testing working struct serialization");
    
    SimpleData data{id, value};
    ctx.db.table<SimpleData>("simple_data").insert(data);
    
    LOG_INFO("Working struct inserted successfully");
}

SPACETIMEDB_REDUCER(init_working_test, spacetimedb::ReducerContext ctx) {
    SpacetimeDB::LogStopwatch timer("working_test_init");
    LOG_INFO("Initializing working struct test database");
    
    LOG_INFO("Working struct test database initialized successfully");
}