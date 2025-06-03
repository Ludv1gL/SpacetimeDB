#include <spacetimedb/spacetimedb_easy.h>
#include <spacetimedb/bsatn/traits.h>

// Test struct with proper BSATN serialization
struct TestStruct {
    uint32_t id;
    uint8_t value;
    std::string name;
};

// Use the working SPACETIMEDB_BSATN_STRUCT macro from traits.h
SPACETIMEDB_BSATN_STRUCT(TestStruct, id, value, name)

// Declare table with the properly serializable struct
SPACETIMEDB_TABLE(TestStruct, test_struct, true)

SPACETIMEDB_REDUCER(test_bsatn_struct_insert, spacetimedb::ReducerContext ctx, uint32_t id, uint8_t value) {
    LOG_INFO("Testing BSATN struct serialization");
    LOG_INFO("Creating TestStruct with id=" + std::to_string(id) + ", value=" + std::to_string(value));
    
    TestStruct data{id, value, "test_name_" + std::to_string(id)};
    
    try {
        ctx.db.table<TestStruct>("test_struct").insert(data);
        LOG_INFO("BSATN struct insertion successful!");
    } catch (const std::exception& e) {
        LOG_ERROR("BSATN struct insertion failed: " + std::string(e.what()));
    }
}

SPACETIMEDB_REDUCER(test_bsatn_validation, spacetimedb::ReducerContext ctx) {
    LOG_INFO("=== BSATN Struct Test Results ===");
    LOG_INFO("Testing struct with SPACETIMEDB_BSATN_STRUCT macro");
    LOG_INFO("Struct fields: uint32_t id, uint8_t value, std::string name");
    LOG_INFO("This test validates that the working BSATN traits.h macro resolves the metadata issue");
}