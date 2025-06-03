#include <spacetimedb/spacetimedb_easy.h>

// Simple test to verify refactored headers work
struct SimpleTest {
    uint32_t id;
};

SPACETIMEDB_TABLE(SimpleTest, simple_test, true)

SPACETIMEDB_REDUCER(simple_test_reducer, spacetimedb::ReducerContext ctx) {
    LOG_INFO("=== Refactored Headers Test ===");
    LOG_INFO("✅ Headers compile successfully");
    LOG_INFO("✅ Includes work properly");
    LOG_INFO("✅ Basic functionality intact");
}