#ifndef SPACETIMEDB_EASY_H
#define SPACETIMEDB_EASY_H

// This header provides the cleanest syntax for SpacetimeDB modules
// Uses x-macro pattern for automatic table registration and accessor generation

// First, define custom context flag
#define SPACETIMEDB_CUSTOM_REDUCER_CONTEXT

// Include the base header  
#include "spacetimedb.h"
#include "sdk/logging.h"     // For enhanced logging features
#include "sdk/exceptions.h"  // For rich error handling
#include "sdk/index_management.h"  // For index management features
#include <cstring>  // For std::strcmp

// Override the original SPACETIMEDB_TABLE macro
#undef SPACETIMEDB_TABLE

// Simple table name registry for runtime queries
namespace spacetimedb {
namespace detail {
    // Fixed-size array for table names
    // Increase this if you have more than 64 tables
    constexpr size_t MAX_TABLES = 64;
    inline const char* table_names[MAX_TABLES] = {};
    inline size_t table_count = 0;
    
    // Register a table name
    inline void register_table_name(const char* name) {
        if (table_count < MAX_TABLES) {
            table_names[table_count++] = name;
        }
    }
}
}

// Base class with generic table access methods
class ModuleDatabaseBase {
public:
    // Generic table accessor - works with any type
    template<typename T>
    TableHandle<T> table(const char* name) {
        return TableHandle<T>(name);
    }
    
    // Helper to get table by name with type checking
    template<typename T>
    TableHandle<T> get(const char* table_name) {
        return table<T>(table_name);
    }
    
    // Check if a table exists
    bool has_table(const char* name) const {
        using namespace spacetimedb::detail;
        for (size_t i = 0; i < table_count; ++i) {
            if (table_names[i] && std::strcmp(table_names[i], name) == 0) {
                return true;
            }
        }
        return false;
    }
    
    // Get table count
    size_t get_table_count() const {
        return spacetimedb::detail::table_count;
    }
};

// X-Macro pattern: Define your tables here
// Format: X(TypeName, table_name, is_public)
// This list will be used to generate table registrations and accessor methods
#ifndef SPACETIMEDB_TABLES_LIST
#define SPACETIMEDB_TABLES_LIST \
    /* Default empty list - user should define their own */
#endif

// Forward declare types from the X-macro list
#define X(TypeName, table_name, is_public) \
    struct TypeName;
SPACETIMEDB_TABLES_LIST
#undef X

// Generate ModuleDatabase class with accessor methods
class ModuleDatabase : public ModuleDatabaseBase {
public:
    // Generate accessor methods from the X-macro list
    #define X(TypeName, table_name, is_public) \
        TableHandle<TypeName> table_name() { \
            return table<TypeName>(#table_name); \
        }
    SPACETIMEDB_TABLES_LIST
    #undef X
};

// Define custom ReducerContext
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

// Generate table registration functions from the X-macro list
#define X(TypeName, table_name, is_public) \
    __attribute__((export_name("__preinit__20_table_" #table_name))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_table_, table_name)() { \
        register_table_impl<TypeName>(#table_name, is_public); \
        spacetimedb::detail::register_table_name(#table_name); \
    }
SPACETIMEDB_TABLES_LIST
#undef X

// Keep the original SPACETIMEDB_TABLE macro for backward compatibility
#define SPACETIMEDB_TABLE(type_name, table_name, is_public) \
    __attribute__((export_name("__preinit__20_table_" #table_name))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_table_, table_name)() { \
        register_table_impl<type_name>(#table_name, is_public); \
        spacetimedb::detail::register_table_name(#table_name); \
    }

// Helper macro to define table accessor methods
#define SPACETIMEDB_TABLE_ACCESSOR(type_name, method_name) \
    TableHandle<type_name> method_name() { \
        return table<type_name>(#method_name); \
    }

// Alternative: User can define a custom database class
#define SPACETIMEDB_MODULE_DATABASE(class_name, ...) \
    class class_name : public ModuleDatabaseBase { \
    public: \
        __VA_ARGS__ \
    }; \
    /* Redefine ReducerContext to use the custom database */ \
    namespace spacetimedb { \
        class ReducerContext { \
        public: \
            class_name db; \
            ReducerContext() = default; \
        }; \
    }

// Usage:
//
// Method 1: One-time declaration with X-Macro pattern
// In your module file, before including spacetimedb_easy.h:
//
//    #define SPACETIMEDB_TABLES_LIST \
//        X(OneU8, one_u8, true) \
//        X(OneU8, another_u8, false)
//    
//    #include <spacetimedb/spacetimedb_easy.h>
//    
//    struct OneU8 { uint8_t n; };
//    
//    // No need for SPACETIMEDB_TABLE declarations anymore!
//    // Tables are automatically registered from the X-macro list
//
// Method 2: Traditional approach (still supported)
//
//    #include <spacetimedb/spacetimedb_easy.h>
//    struct OneU8 { uint8_t n; };
//    SPACETIMEDB_TABLE(OneU8, one_u8, true)
//
// Method 3: Generic access (always available)
//
//    ctx.db.table<OneU8>("one_u8").insert(row);

#endif // SPACETIMEDB_EASY_H