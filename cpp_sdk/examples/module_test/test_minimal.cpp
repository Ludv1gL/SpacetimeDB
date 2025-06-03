// Absolutely minimal module to test module description
#define SPACETIMEDB_TABLES_LIST \
    X(MinimalTable, minimal_table, true)

#include <spacetimedb/spacetimedb.h>

struct MinimalTable {
    uint8_t value;
};

using namespace spacetimedb;

SPACETIMEDB_REDUCER(add_value, ReducerContext ctx, uint8_t val) {
    MinimalTable row{val};
    ctx.db.minimal_table().insert(row);
}