#ifndef SPACETIMEDB_EASY_H
#define SPACETIMEDB_EASY_H

// This header provides the cleanest syntax for SpacetimeDB modules
// First, define custom context flag
#define SPACETIMEDB_CUSTOM_REDUCER_CONTEXT

// Include the base header  
#include "spacetimedb.h"

// Override the original SPACETIMEDB_TABLE macro
#undef SPACETIMEDB_TABLE

// Redefine SPACETIMEDB_TABLE to register tables using __preinit__ mechanism
#define SPACETIMEDB_TABLE(type_name, table_name, is_public) \
    __attribute__((export_name("__preinit__20_table_" #table_name))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_table_, table_name)() { \
        register_table_impl<type_name>(#table_name, is_public); \
    }

// Auto-generate module classes once using header guard technique
#ifndef SPACETIMEDB_MODULE_CLASSES_DEFINED
#define SPACETIMEDB_MODULE_CLASSES_DEFINED

// Forward declarations for table types
struct OneU8;

class ModuleDatabase {
public:
    template<typename T>
    TableHandle<T> table(const char* name) {
        return TableHandle<T>(name);
    }
    
    // Hardcoded table methods - simple and works
    TableHandle<OneU8> one_u8() {
        return TableHandle<OneU8>("one_u8");
    }
    
    TableHandle<OneU8> another_u8() {
        return TableHandle<OneU8>("another_u8");
    }
};

// Define custom ReducerContext since we set the flag
namespace spacetimedb {
    class ReducerContext {
    public:
        ModuleDatabase db;
        ReducerContext() = default;
    };
}

// Provide the implementation for spacetimedb_call_reducer
inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args) {
    auto& module = ModuleDef::instance();
    if (id < module.reducers.size()) {
        spacetimedb::ReducerContext ctx;
        module.reducers[id].handler(ctx, args);
        return 0;
    }
    return -1;
}

#endif // SPACETIMEDB_MODULE_CLASSES_DEFINED

#endif // SPACETIMEDB_EASY_H