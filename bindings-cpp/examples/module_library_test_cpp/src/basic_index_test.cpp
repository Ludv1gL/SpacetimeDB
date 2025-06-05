#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/bsatn/bsatn.h>

// Test struct for index functionality
struct IndexedData {
    uint32_t id;
    std::string name;
    uint8_t category;
};

// Manual BSATN specialization
namespace SpacetimeDb::bsatn {
    template<>
    struct bsatn_traits<IndexedData> {
        static void serialize(SpacetimeDb::bsatn::Writer& writer, const IndexedData& value) {
            SpacetimeDb::bsatn::serialize(writer, value.id);
            SpacetimeDb::bsatn::serialize(writer, value.name);
            SpacetimeDb::bsatn::serialize(writer, value.category);
        }
        
        static IndexedData deserialize(SpacetimeDb::bsatn::Reader& reader) {
            IndexedData result;
            result.id = SpacetimeDb::bsatn::deserialize<uint32_t>(reader);
            result.name = SpacetimeDb::bsatn::deserialize<std::string>(reader);
            result.category = SpacetimeDb::bsatn::deserialize<uint8_t>(reader);
            return result;
        }
        
        static SpacetimeDb::bsatn::AlgebraicType algebraic_type() {
            std::vector<SpacetimeDb::bsatn::AggregateElement> elements;
            return SpacetimeDb::bsatn::AlgebraicType::make_product(
                std::make_unique<SpacetimeDb::bsatn::ProductType>(std::move(elements))
            );
        }
    };
}

// Declare table with index hints (for future implementation)
SPACETIMEDB_TABLE(IndexedData, indexed_data, true)
// Future: SPACETIMEDB_INDEX_BTREE(IndexedData, name)
// Future: SPACETIMEDB_INDEX_UNIQUE(IndexedData, id)

// Test reducers for index functionality
SPACETIMEDB_REDUCER(insert_indexed_data, SpacetimeDb::ReducerContext ctx, uint32_t id, std::string name, uint8_t category) {
    LOG_INFO("Inserting indexed data: id=" + std::to_string(id) + ", name=" + name + ", category=" + std::to_string(category));
    
    IndexedData data{id, name, category};
    ctx.db.table<IndexedData>("indexed_data").insert(data);
    
    LOG_INFO("Indexed data inserted successfully");
}

SPACETIMEDB_REDUCER(test_index_operations, SpacetimeDb::ReducerContext ctx, std::string search_name) {
    LOG_INFO("Testing index operations (placeholder)");
    LOG_INFO("Would search for name: " + search_name);
    
    // Future index operations will go here:
    // auto name_index = ctx.db.table<IndexedData>("indexed_data").index_name();
    // auto results = name_index.Filter(search_name);
    // LOG_INFO("Found " + std::to_string(results.size()) + " results");
    
    LOG_INFO("Index operations test completed (infrastructure ready)");
}

SPACETIMEDB_REDUCER(test_range_queries, SpacetimeDb::ReducerContext ctx, uint32_t min_id, uint32_t max_id) {
    LOG_INFO("Testing range queries (placeholder)");
    LOG_INFO("Would search for id range: " + std::to_string(min_id) + " to " + std::to_string(max_id));
    
    // Future range query operations:
    // auto id_index = ctx.db.table<IndexedData>("indexed_data").index_id();
    // SpacetimeDb::Range<uint32_t> range(
    //     SpacetimeDb::Bound<uint32_t>::Inclusive(min_id),
    //     SpacetimeDb::Bound<uint32_t>::Inclusive(max_id)
    // );
    // auto results = id_index.FilterRange(range);
    // LOG_INFO("Found " + std::to_string(results.size()) + " results in range");
    
    LOG_INFO("Range queries test completed (infrastructure ready)");
}

SPACETIMEDB_REDUCER(init_index_test, SpacetimeDb::ReducerContext ctx) {
    SpacetimeDB::LogStopwatch timer("index_test_init");
    LOG_INFO("Initializing index management test database");
    
    LOG_INFO("Index test infrastructure:");
    LOG_INFO("✅ Index management headers included");
    LOG_INFO("✅ BTreeIndex and UniqueIndex classes defined");
    LOG_INFO("✅ Range and Bound types available");
    LOG_INFO("✅ IndexIterator for efficient queries ready");
    LOG_INFO("🚧 Index registration and FFI integration pending");
    
    LOG_INFO("Index management test database initialized successfully");
}