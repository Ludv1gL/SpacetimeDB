/**
 * Simple SpacetimeDB C++ SDK Test Module
 * 
 * Basic test to verify core functionality works before full test.
 */

#include <spacetimedb/spacetimedb.h>

using namespace spacetimedb;

// Simple struct to test basic functionality
struct SimpleData {
    uint32_t id;
    std::string name;
    int32_t value;
};

// Register the type
namespace {
    struct SimpleData_field_registrar {
        SimpleData_field_registrar() {
            spacetimedb::register_type<SimpleData>("SimpleData", {
                {"id", offsetof(SimpleData, id), sizeof(uint32_t)},
                {"name", offsetof(SimpleData, name), sizeof(std::string)},
                {"value", offsetof(SimpleData, value), sizeof(int32_t)}
            });
        }
    };
    static SimpleData_field_registrar SimpleData_field_registrar_instance;
}

// Register as table
SPACETIMEDB_TABLE(SimpleData, "simple_data", true)

// Simple reducer - no arguments
SPACETIMEDB_REDUCER(test_simple, UserDefined, ctx)
{
    LOG_INFO("Simple test reducer called");
    
    SimpleData data{1, "test", 42};
    auto table = get_SimpleData_table();
    table.insert(data);
}

// Init reducer
SPACETIMEDB_INIT(init, ctx)
{
    LOG_INFO("Simple SDK test module initialized");
}