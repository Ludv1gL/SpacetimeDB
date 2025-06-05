#ifndef MODULE_BINDINGS_GENERATOR_H
#define MODULE_BINDINGS_GENERATOR_H

// Forward declaration of user-defined reducer
void insert_one_u8(SpacetimeDb::ReducerContext ctx, SpacetimeDb::byte n);

// This macro will be expanded after OneU8 is defined
#define SPACETIMEDB_MODULE_INIT() \
    /* Explicit registration function that directly registers all known types */ \
    void register_all_module_items() { \
        /* Register OneU8 table directly */ \
        register_table_type<OneU8>("one_u8", true); \
        \
        /* Register insert_one_u8 reducer directly */ \
        register_reducer_func("insert_one_u8", insert_one_u8); \
    }

#endif // MODULE_BINDINGS_GENERATOR_H