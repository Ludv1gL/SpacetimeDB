# SpacetimeDB C++ Module Library - Table Operations Guide

This guide explains the table operations available in the SpacetimeDB C++ Module Library.

## Table Handle

The `TableHandle<T>` class provides type-safe access to database tables. You get a table handle through the `ModuleDatabase` instance available in the `ReducerContext`:

```cpp
auto my_table = ctx.db.table<MyType>("table_name");
```

## Available Operations

### 1. Insert

Insert a new row into the table. Returns the inserted row (which may have auto-generated fields like auto-increment IDs).

```cpp
MyType row = {/* field values */};
MyType inserted = table.insert(row);
```

### 2. Count

Get the number of rows in the table.

```cpp
uint64_t row_count = table.count();
```

### 3. Iterate (iter)

Iterate over all rows in the table. Supports both explicit iteration and range-for loops.

```cpp
// Explicit iteration
auto iter = table.iter();
while (iter.has_next()) {
    MyType row = iter.next();
    // Process row
}

// Range-for loop (recommended)
for (const auto& row : table.iter()) {
    // Process row
}
```

### 4. Delete

Delete rows from the table. Currently supports deletion by value matching.

```cpp
MyType row_to_delete = {/* values */};
bool deleted = table.delete_by_value(row_to_delete);
```

### 5. Update

Update a row by replacing it with a new value. This is implemented as delete + insert.

```cpp
MyType old_row = {/* old values */};
MyType new_row = {/* new values */};
bool updated = table.update(old_row, new_row);
```

## Example Usage

```cpp
struct Person {
    uint32_t id;
    uint8_t age;
};

SPACETIMEDB_TABLE(Person, person, true)

SPACETIMEDB_REDUCER(manage_people, spacetimedb::ReducerContext ctx) {
    auto person_table = ctx.db.table<Person>("person");
    
    // Insert
    Person alice = person_table.insert({1, 30});
    
    // Count
    uint64_t count = person_table.count();
    
    // Iterate
    for (const auto& person : person_table.iter()) {
        if (person.age > 25) {
            // Process adult
        }
    }
    
    // Update
    Person updated_alice = {1, 31};
    person_table.update(alice, updated_alice);
    
    // Delete
    person_table.delete_by_value(updated_alice);
}
```

## Implementation Status

- ✅ **insert()** - Fully implemented with auto-increment support
- ✅ **count()** - Implemented (currently uses iteration internally)
- ✅ **iter()** - Basic implementation (simplified BSATN parsing)
- ✅ **delete_by_value()** - Implemented using first column matching
- ✅ **update()** - Implemented as delete + insert

## Known Limitations

1. **Iterator Deserialization**: The current iterator implementation uses simplified deserialization. Full BSATN parsing support is needed for complex types.

2. **Delete Operations**: Currently only supports delete by value using the first column. More sophisticated deletion (by primary key, by predicate) will be added.

3. **Count Performance**: The count operation currently iterates through all rows. A dedicated FFI function for counting would be more efficient.

4. **Update Atomicity**: Updates are implemented as delete + insert, which is not atomic. True update support would require a dedicated FFI function.

## Future Enhancements

- Index-based queries
- Predicate-based filtering
- Batch operations
- Transaction support
- More efficient count implementation
- Proper BSATN deserialization in iterators