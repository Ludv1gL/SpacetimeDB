/**
 * Minimal test to verify basic compilation
 */

#define SPACETIMEDB_TABLES_LIST \
    X(SimpleRow, simple_row, true)

#include "spacetimedb/spacetimedb.h"

using namespace SpacetimeDb;

// Simple struct like OneU8
struct SimpleRow {
    uint32_t id;
};

// Basic reducer
SPACETIMEDB_REDUCER(test_minimal, ReducerContext ctx, uint32_t id) {
    SimpleRow row{id};
    ctx.db->simple_row().insert(row);
    LOG_INFO("Inserted simple row with id: " + std::to_string(id));
}