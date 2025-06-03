# Migration Guide: Removing Hardcoded Accessor Methods

This guide explains how to migrate from hardcoded accessor methods to the new automatic generation pattern in SpacetimeDB C++ SDK.

## Old Pattern (Hardcoded)

Previously, `spacetimedb_easy.h` contained hardcoded accessor methods:

```cpp
class ModuleDatabase {
public:
    TableHandle<OneU8> one_u8() { 
        return table<OneU8>("one_u8"); 
    }
    
    TableHandle<OneU8> another_u8() { 
        return table<OneU8>("another_u8"); 
    }
    // Had to be manually updated for each new table!
};
```

## New Pattern (Integrated Declaration)

The new pattern uses X-Macros to declare tables once and get everything automatically:

```cpp
// Define ALL your tables BEFORE including the header
// Format: X(TypeName, table_name, is_public)
#define SPACETIMEDB_TABLES_LIST \
    X(OneU8, one_u8, true) \
    X(OneU8, another_u8, false)

#include <spacetimedb/spacetimedb_easy.h>

// No need for SPACETIMEDB_TABLE macros anymore!
```

## Migration Steps

### Step 1: Replace SPACETIMEDB_TABLE macros with X-Macro list

Before the `#include <spacetimedb/spacetimedb_easy.h>` line, add:

```cpp
// Include the is_public parameter (third argument)
#define SPACETIMEDB_TABLES_LIST \
    X(TypeName, table_name, true) \
    X(AnotherType, another_table, false)
```

### Step 2: Remove SPACETIMEDB_TABLE declarations

Delete all individual `SPACETIMEDB_TABLE` macro calls - they're no longer needed!

```cpp
// DELETE THESE:
SPACETIMEDB_TABLE(OneU8, one_u8, true)
SPACETIMEDB_TABLE(OneU8, another_u8, false)
```

### Step 3: Keep Type Definitions After Include

Your struct definitions should come AFTER the include:

```cpp
#define SPACETIMEDB_TABLES_LIST \
    X(OneU8, one_u8, true) \
    X(OneU8, another_u8, false)
    
#include <spacetimedb/spacetimedb_easy.h>

struct OneU8 { uint8_t n; };
```

### Step 4: No Reducer Changes Required

Your existing reducer code continues to work unchanged:

```cpp
SPACETIMEDB_REDUCER(insert_one_u8, ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db.one_u8().insert(row);  // Still works!
}
```

## Benefits

1. **No Manual Updates**: Add new tables without modifying `spacetimedb_easy.h`
2. **Type Safety**: Compile-time checking for table names and types
3. **Clean Syntax**: Same clean `ctx.db.table_name()` syntax
4. **Extensible**: Easy to add new tables to any module

## Alternative Access Methods

If you prefer not to use the X-Macro pattern, you can still use:

### Generic Access
```cpp
ctx.db.table<OneU8>("one_u8").insert(row);
```

### Custom Database Class
```cpp
SPACETIMEDB_MODULE_DATABASE(MyDatabase,
    SPACETIMEDB_TABLE_ACCESSOR(OneU8, one_u8)
    SPACETIMEDB_TABLE_ACCESSOR(OneU8, another_u8)
)
```

## Example: Complete Migration

**Before:**
```cpp
#include <spacetimedb/spacetimedb_easy.h>

struct OneU8 { uint8_t n; };

SPACETIMEDB_TABLE(OneU8, one_u8, true)
SPACETIMEDB_TABLE(OneU8, another_u8, false)

SPACETIMEDB_REDUCER(insert_one_u8, ReducerContext ctx, uint8_t n) {
    ctx.db.one_u8().insert({n});
}
```

**After:**
```cpp
// Define tables list with is_public parameter
#define SPACETIMEDB_TABLES_LIST \
    X(OneU8, one_u8, true) \
    X(OneU8, another_u8, false)

#include <spacetimedb/spacetimedb_easy.h>

struct OneU8 { uint8_t n; };

// No SPACETIMEDB_TABLE declarations needed!

SPACETIMEDB_REDUCER(insert_one_u8, ReducerContext ctx, uint8_t n) {
    ctx.db.one_u8().insert({n});  // No change needed!
}
```

## Troubleshooting

### Error: "use of undeclared identifier"
Make sure the type is forward declared by the X-Macro before being used.

### Error: "no member named 'my_table' in 'ModuleDatabase'"
Ensure your table is listed in `SPACETIMEDB_TABLES_LIST`.

### Want to keep old behavior?
You can use generic access: `ctx.db.table<Type>("table_name")`