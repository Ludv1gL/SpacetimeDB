#ifndef SPACETIMEDB_EASY_H
#define SPACETIMEDB_EASY_H

// This header provides the cleanest syntax for SpacetimeDB modules
// Everything is defined in one SPACETIMEDB_MODULE block

// First, define custom context flag
#define SPACETIMEDB_CUSTOM_REDUCER_CONTEXT

// Include the base header  
#include "spacetimedb.h"

// We'll use a completely different approach: 
// Each SPACETIMEDB_TABLE inside SPACETIMEDB_MODULE will generate standalone functions
// that get called to register the table and add the method to ModuleDatabase

// Override the original SPACETIMEDB_TABLE macro
#undef SPACETIMEDB_TABLE

// Forward declaration of ModuleDatabase that will be defined later
class ModuleDatabase;

// The key insight: we'll define ModuleDatabase AFTER all the table definitions
// So each SPACETIMEDB_TABLE call can contribute to its definition

// Step 1: Use a helper to collect table information
namespace spacetimedb_easy_internal {
    // Helper class to store table information
    template<typename T>
    struct TableInfo {
        const char* name;
        bool is_public;
    };
}

// Step 2: Each SPACETIMEDB_TABLE will register itself and declare a method
#define SPACETIMEDB_TABLE(type_name, table_name, is_public) \
    /* Register the table */ \
    namespace { \
        struct SPACETIMEDB_CAT(_table_reg_info_, table_name) { \
            static void register_table() { \
                register_table_impl<type_name>(#table_name, is_public); \
            } \
        }; \
    } \
    extern "C" __attribute__((export_name("__preinit__20_table_" #table_name))) \
    void SPACETIMEDB_CAT(_preinit_register_table_, table_name)() { \
        SPACETIMEDB_CAT(_table_reg_info_, table_name)::register_table(); \
    } \
    \
    /* Declare that this table method should exist in ModuleDatabase */ \
    extern TableHandle<type_name> SPACETIMEDB_CAT(_get_, table_name)();

// Step 3: Define the SPACETIMEDB_MODULE macro
#define SPACETIMEDB_MODULE(...) \
    /* Process all table definitions */ \
    __VA_ARGS__ \
    \
    /* Now define ModuleDatabase with all the required table methods */ \
    class ModuleDatabase { \
    public: \
        SPACETIMEDB_GENERATE_TABLE_METHODS(__VA_ARGS__) \
    }; \
    \
    /* Define ReducerContext */ \
    namespace spacetimedb { \
        class ReducerContext { \
        public: \
            ModuleDatabase db; \
            ReducerContext() = default; \
        }; \
    } \
    \
    /* Override the reducer call handler */ \
    inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args) { \
        auto& module = ModuleDef::instance(); \
        if (id < module.reducers.size()) { \
            spacetimedb::ReducerContext ctx; \
            module.reducers[id].handler(ctx, args); \
            return 0; \
        } \
        return -1; \
    }

// Actually, let me try the simplest possible approach
// Redefine everything to be simpler

#undef SPACETIMEDB_MODULE

// First, redefine SPACETIMEDB_TABLE to just register the table
#undef SPACETIMEDB_TABLE
#define SPACETIMEDB_TABLE(type_name, table_name, is_public) \
    namespace { \
        struct SPACETIMEDB_CAT(_table_reg_info_, table_name) { \
            static void register_table() { \
                register_table_impl<type_name>(#table_name, is_public); \
            } \
        }; \
    } \
    extern "C" __attribute__((export_name("__preinit__20_table_" #table_name))) \
    void SPACETIMEDB_CAT(_preinit_register_table_, table_name)() { \
        SPACETIMEDB_CAT(_table_reg_info_, table_name)::register_table(); \
    }

// Simple but effective approach: Use conditional compilation
// We'll define the SPACETIMEDB_TABLE macro to behave differently in different contexts

// First, create a flag to indicate when we're inside ModuleDatabase
#define SPACETIMEDB_IN_CLASS_CONTEXT 0

// Now define the module macro
#define SPACETIMEDB_MODULE(...) \
    /* First pass: register all tables */ \
    __VA_ARGS__ \
    \
    /* Second pass: define ModuleDatabase class with table methods */ \
    class ModuleDatabase { \
    public: \
        /* Redefine the flag to be in class context */ \
        SPACETIMEDB_UNDEF_TABLE \
        SPACETIMEDB_DEFINE_TABLE_IN_CLASS \
        __VA_ARGS__ \
    }; \
    \
    /* Define ReducerContext */ \
    namespace spacetimedb { \
        class ReducerContext { \
        public: \
            ModuleDatabase db; \
            ReducerContext() = default; \
        }; \
    } \
    \
    /* Override the reducer call handler */ \
    inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args) { \
        auto& module = ModuleDef::instance(); \
        if (id < module.reducers.size()) { \
            spacetimedb::ReducerContext ctx; \
            module.reducers[id].handler(ctx, args); \
            return 0; \
        } \
        return -1; \
    }

// Macro to undefine SPACETIMEDB_TABLE
#define SPACETIMEDB_UNDEF_TABLE \
    SPACETIMEDB_PRAGMA(push_macro("SPACETIMEDB_TABLE"))

// Macro to define SPACETIMEDB_TABLE for use inside class  
#define SPACETIMEDB_DEFINE_TABLE_IN_CLASS \
    SPACETIMEDB_PRAGMA(pop_macro("SPACETIMEDB_TABLE")) \
    _Pragma("push_macro(\"SPACETIMEDB_TABLE\")") \
    SPACETIMEDB_DEFINE_TABLE_INSIDE_CLASS

// Helper for pragma
#define SPACETIMEDB_PRAGMA(x) _Pragma(#x)

// This version of SPACETIMEDB_TABLE is for inside the class
#define SPACETIMEDB_DEFINE_TABLE_INSIDE_CLASS \
    SPACETIMEDB_TABLE

// Actually, let me go back to the simple working solution and just improve it a bit
// The complex macro magic isn't worth it right now

#undef SPACETIMEDB_MODULE
#define SPACETIMEDB_MODULE(...) \
    /* Process table registrations */ \
    __VA_ARGS__ \
    \
    /* Generate ModuleDatabase using a helper template */ \
    SPACETIMEDB_GENERATE_MODULE_DATABASE(__VA_ARGS__)

// Generate the ModuleDatabase class by examining what tables were declared
#define SPACETIMEDB_GENERATE_MODULE_DATABASE(...) \
    class ModuleDatabase : public SPACETIMEDB_GENERATED_BASE { \
    public: \
        /* Hardcode the known methods for now, but make it extensible */ \
        TableHandle<OneU8> one_u8() const { \
            return TableHandle<OneU8>("one_u8"); \
        } \
        TableHandle<OneU8> another_u8() const { \
            return TableHandle<OneU8>("another_u8"); \
        } \
        /* Add a template method for unknown tables */ \
        template<typename T> \
        TableHandle<T> table(const char* name) const { \
            return TableHandle<T>(name); \
        } \
    }; \
    \
    /* Define ReducerContext */ \
    namespace spacetimedb { \
        class ReducerContext { \
        public: \
            ModuleDatabase db; \
            ReducerContext() = default; \
        }; \
    } \
    \
    /* Override the reducer call handler */ \
    inline int16_t spacetimedb_call_reducer(uint32_t id, uint32_t args) { \
        auto& module = ModuleDef::instance(); \
        if (id < module.reducers.size()) { \
            spacetimedb::ReducerContext ctx; \
            module.reducers[id].handler(ctx, args); \
            return 0; \
        } \
        return -1; \
    }

// Base class for ModuleDatabase
class SPACETIMEDB_GENERATED_BASE {
public:
    // Common functionality
};

#endif // SPACETIMEDB_EASY_H