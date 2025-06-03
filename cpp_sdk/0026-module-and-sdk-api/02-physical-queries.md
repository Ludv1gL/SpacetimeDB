# Module and SDK API: Physical Queries

This file contains the normative definition of the user-facing interface for the "Physical query API," i.e. direct access to indices. This supersedes the interface described in [0012: Physical Access Bindings](../0012-physical-access-bindings.md).

We propose that, until future work designs and implements a full-fledged logical query API whose queries are optimized by the query planner, SpacetimeDB modules should only have access to those queries which can be statically optimized and known to be efficient based on the declared table schema, constraints and indices. Past efforts to provide a data-independent logical query API have made it too easy for users to unknowingly write queries which are implemented as full scans, and made it too difficult for users to distinguish between fast index-based queries and slow scan-based queries when examining module source code.

Because we will only be providing the physical query API to modules in SpacetimeDB 1.0, we must make it as ergonomic as possible to succinctly express these operations. As such, we propose method names which will be familiar to Rust or C# programmers, which previously SpacetimeDB has reserved for logical query APIs. Our belief is that the future logical query API will express all operations through a single source-level entry point, likely named `query`, and therefore that it will not conflict with the physical query method names described below.

## Generated for every table

In both the module and the client, every table exposes methods `count` and `iter` on its `TableHandle`. These common methods are encoded on a trait (or interface or common superclass) `Table`.

In the below code, `TableHandle` refers to the generated table handle intermediary, `Row` refers to the user-defined row type, and `table` refers to the user-supplied table name.

```rust
/// Implemented for every `TableHandle` struct generated in the client `module_bindings`
/// and the module macroexpansion.
pub trait Table {
    /// The type of rows stored in this table.
    type Row;

    /// Returns the number of rows of this table in the TX state,
    /// i.e. num(committed_state) + num(insert_table) - num(delete_table).
    ///
    /// This API is new to modules (though it previously existed in the Rust SDK)
    /// and will require a new host function in the ABI.
    fn count(&self) -> u64;

    /// Iterate over all rows in the TX state,
    /// i.e. committed_state ∪ insert_table ∖ delete_table.
    fn iter(&self) -> impl Iterator<Item = Self::Row>;
}
```

Note that this is not an exhaustive listing of the methods on the `Table` trait or generated on the `TableHandle`.

### Only in modules

In addition to reading from the TX state, modules may mutate it. Clients may not mutate the client cache, so the following methods are generated only in the module macroexpansion.

In the presence of constraints, any mutation is a fallible operation. As of writing, SpacetimeDB implements only constraints which cause insertion to be fallible, but it is likely that in the future we will add constraints which make deletion fallible as well, e.g. foreign keys. It is the opinion of the author that users will rarely have a way to meaningfully handle failed mutations, and that the most common intended behavior will be aborting the transaction. As such, for any potentially-fallible mutation we provide two interfaces in Rust: one which may panic, like `insert`, and another prefixed with `try_` which returns a `Result`, like `try_insert`. In languages where error-handling is by exceptions, the split is not necessary; callers of e.g. `insert` may install exception handlers around their call if they intend to handle failure. (Note that even if we wanted Rust to use `catch_unwind` in a similar way, Rust compiled to WASM does not unwind.)

Also note that, as a set-semantic database, we treat the insertion of an already-present row as a successful no-op, unless some other constraint causes it to fail.

In modules, the `Table` trait has the following members in addition to those listed above:

```rust
trait Table {
    // above methods elided.

    /// Inserts `row` into the TX state,
    /// i.e. removes it from the delete table or adds it to the insert table as appropriate.
    ///
    /// The return value is the inserted row, with any auto-incrementing columns replaced with computed values.
    /// The `insert` method always returns the inserted row,
    /// even when the table contains no auto-incrementing columns.
    ///
    /// May panic if inserting the row violates any constraints.
    /// Callers which intend to handle constraint violation errors should instead use [`Self::try_insert`].
    ///
    /// Note that, in languages where error handling is based on exceptions,
    /// no distinction is provided between `Table::insert` and `Table::try_insert`.
    /// A single method `insert` is defined which throws an exception on failure,
    /// and callers may either install handlers around it or allow the exception to bubble up.
    ///
    /// Note on MVCC: because callers have no way to determine if the row was previously present,
    /// two concurrent transactions which delete the same row
    /// may be ordered arbitrarily with respect to one another
    /// while maintaining sequential consistency, assuming no other conflicts.
    fn insert(&self, row: Self::Row) -> Self::Row;

    /// The error type for this table for unique constraint violations.
    /// Will either be [`UniqueConstraintViolation`] if the table has any unique constraints,
    /// or [`std::convert::Infallible`] otherwise.
    type UniqueConstraintViolation;

    /// The error type for this table for auto-increment overflows.
    /// Will either be [`AutoIncOverflow`] if the table has any auto-incrementing columns,
    /// or [`std::convert::Infallible`] otherwise.
    type AutoIncOverflow;

    /// Counterpart to [`Self::insert`] which allows handling failed insertions.
    ///
    /// For tables without any constraints, [`Self::TryInsertError`] will be [`std::convert::Infallible`],
    /// and this will be a more-verbose [`Self::insert`].
    /// For tables with constraints, this method returns an `Err` when the insertion fails rather than panicking.
    ///
    /// Note that, in languages where error handling is based on exceptions,
    /// no distinction is provided between `Table::insert` and `Table::try_insert`.
    /// A single method `insert` is defined which throws an exception on failure,
    /// and callers may either install handlers around it or allow the exception to bubble up.
    fn try_insert(&self, row: Self::Row) -> Result<Self::Row, TryInsertError<Self>>;

    /// Deletes a row equal to `row` from the TX state,
    /// i.e. deletes it from the insert table or adds it to the delete table as appropriate.
    ///
    /// Returns `true` if the row was present and has been deleted,
    /// or `false` if the row was not present and therefore the tables have not changed.
    ///
    /// Unlike [`Self::insert`], there is no need to return the deleted row,
    /// as it must necessarily have been exactly equal to the `row` argument.
    /// No analogue to auto-increment placeholders exists for deletions.
    ///
    /// May panic if deleting the row violates any constraints.
    /// Note that as of writing deletion is infallible, but future work may define new constraints,
    /// e.g. foreign keys, which cause deletion to fail in some cases.
    /// If and when these new constraints are added,
    /// we should define `Self::try_delete` and `TryDeleteError`,
    /// analogous to [`Self::try_insert`] and [`TryInsertError`].
    ///
    /// Note on MVCC: the return value means that logically a `delete` performs a query
    /// to see if the row is present.
    /// As such, two concurrent transactions which delete the same row
    /// cannot be placed in a sequentially-consistent ordering,
    /// and one of them must be retried.
    fn delete(&self, row: Self::Row) -> bool;
}

/// Defined in the module library.
pub enum TryInsertError<Tbl: Table> {
    /// Returned from [`TableHandle::try_insert`] if an attempted insertion
    /// has the same value in a unique column as an already-present row.
    ///
    /// If the table does not have any unique constraints,
    /// `Tbl::UniqueConstraintViolation` will be `Infallible`,
    /// and so this variant will be un-constructable.
    UniqueConstraintViolation(Tbl::UniqueConstraintViolation),

    /// Returned from [`TableHandle::try_insert`] if an attempted insertion
    /// advances an auto-inc sequence past the bounds of the column type.
    ///
    /// If the table does not have any autoinc columns,
    /// `Tbl::AutoIncOverflow` will be `Infallible`,
    /// and so this variant will be un-constructable.
    ///
    /// Note that this is not the current behavior.
    /// As of writing, auto-inc sequences silently truncate on overflow.
    /// Correctly implementing this proposal will require
    /// changing the behavior of `AlgebraicValue::from_sequence_value`
    /// and its caller `MutTxId::insert`.
    AutoIncOverflow(Tbl::AutoIncOverflow),
}
```

## Generated for each unique constraint

In both modules and clients, for each column of a table whose constraints include uniqueness, the `TableHandle` struct is generated with an "index accessor" method with the same name as that column, which returns an autogenerated type of unspecified name, encoding access to the unique index. The index access type has a method for seeking via the unique index. In modules, methods are additionally provided for deleting and updating via the unique index.

The name of the index accessor may or may not be the same as the name of any object in the database's description or schema, stored in the system tables, or exposed to SQL.

For a table:

```rust
#[spacetimedb::table(name = my_table)]
struct MyRow {
    #[unique]
    my_unique_column: u32,
}
```

We generate roughly:

```rust
// Name may vary. Type may be generic, incl. over a lifetime bound and/or type bounds.
// This type may be defined in the module or SDK library rather than autogenerated.
struct MyTableMyUniqueColumnUniqueIndex {
    // private fields
}

impl TableHandle {
    // May return a shared reference `&MyTableMyUniqueColumnUniqueIndex`.
    fn my_unique_column(&self) -> MyTableMyUniqueColumnUniqueIndex;
}

impl MyTableMyUniqueColumnUniqueIndex {
    /// Finds and returns the row where the value in the unique column matches the supplied `col_val`,
    /// or `None` if no such row is present in the database state.
    //
    // TODO: consider whether we should accept the sought value by ref or by value.
    // Should be consistent with the implementors of `BTreeIndexBounds` (see below).
    // By-value makes passing `Copy` fields more convenient,
    // whereas by-ref makes passing `!Copy` fields more performant.
    // Can we do something smart with `std::borrow::Borrow`?
    fn find(&self, col_val: u32) -> Option<MyRow>;

    // The following only in modules.

    /// Deletes the row where the value in the unique column matches the supplied `col_val`,
    /// if any such row is present in the database state.
    ///
    /// Returns `true` if a row with the specified `col_val` was previously present and has been deleted,
    /// or `false` if no such row was present.
    ///
    /// May panic if deleting the row would violate a constraint,
    /// though as of proposing no such constraints exist.
    fn delete(&self, col_val: u32) -> bool;

    /// Deletes the row where the value in the unique column matches that in the corresponding field of `new_row`,
    /// then inserts the `new_row`.
    ///
    /// Returns the new row as actually inserted, with any auto-inc placeholders substituted for computed values.
    ///
    /// Panics if no row was previously present with the matching value in the unique column,
    /// or if either the delete or the insertion would violate a constraint.
    ///
    /// Implementors are encouraged to include the table name, unique column name, and unique column value
    /// in the panic message when no such row previously existed.
    fn update(&self, new_row: MyRow) -> MyRow;
}
```

Source-level visibility of types and methods should be inherited from the declared unique field, as described in [01: the `DbConnection` Type](./01-db-connection.md).

Unlike explicitly declared BTree indices, unique constraints do not result in methods which permit range queries. This will allow us to change the implementation of unique constraints to use a different index type in the future, e.g. a hash index. Future work may allow users to request a specific index type when declaring the unique constraint; in that case, if a user requests a BTree index for a unique constraint, range queries should be permitted.

Future work may add range query methods to unique constraint indexes by deciding to:
- Specify that unique constraints result in BTree indices unless otherwise specified.
- Allow users to explicitly request a BTree index for a unique constraint, possibly via syntax like `#[unique(btree)]`.
- Generate range query methods anyway, leaving them unstable and potentially breaking them if we change the index type used to implement unique constraints.

## Generated for each declared BTree index

In both modules and clients, for each BTree index declared in the `#[spacetimedb::table]` attribute, the `TableHandle` struct is generated with a method, called the "index accessor," whose name is specified via the `name =` argument to the `index` attribute. Indexes defined on a single column by applying the `#[index(btree)]` attribute to a field in a `#[table]`-annotated `struct` have the same accessor name as that field.

The index accessor returns an autogenerated type of unspecified name, encoding access to the index. The index access type has a method for filtering via the index. In modules, a method is additionally provided for deleting via the index.

The `index(name = <name>)` macro syntax is used to specify the index accessor name used in module source code, not any other name of any object in the database's description or schema, stored in the system tables, or exposed to SQL. Any other names required by the implementation may be derived from the index accessor name and/or the table name via some deterministic, predictable procedure specified elsewhere. `ModuleDef` may (or may not) need to be modified to track the index accessor name in addition to any other names it already stores, as index accessor names are used in client codegen.

The index access methods are generic enough to permit all the queries that can be computed by the index, i.e. full equality, prefix equality with range or unbounded suffix, and prefix range with unbounded suffix. The constraint is always provided as a single arugment, which may be either a value or range for the first column, or a tuple of values or ranges. The argument is bounded on a trait or interface `BTreeIndexBounds<ColTuple>`, where `ColTuple` is the full tuple of indexed column types. This trait is defined in the module or SDK library, and is implemented for all valid queries for indexes of up to 10 columns. The `#[spacetimedb::table]` macro should error on attempts to define an index on more columns than `BTreeIndexBounds` supports.

For a table:

```rust
#[spacetimedb::table(
    name = my_table,
    index(name = location, btree = [x, y, z]),
)]
struct MyRow {
    x: u32,
    y: u32,
    z: u32,
}
```

We generate roughly:

```rust
// Name may vary. Type may be generic, incl. over a lifetime bound and/or type bounds.
// This type may be defined in the module or SDK library rather than autogenerated.
struct MyTableLocationIndex {
    // private fields
}

impl TableHandle {
    // May return a shared reference `&MyTableLocationIndex`.
    fn location(&self) -> MyTableLocationIndex;
}

impl MyTableLocationIndex {
    /// Returns an iterator over all rows in the database state where the indexed column(s) match the bounds `b`.
    ///
    /// `b` may be:
    /// - A value for the first indexed column.
    /// - A range of values for the first indexed column.
    /// - A tuple of values for any prefix of the indexed columns, optionally terminated by a range for the next.
    fn filter<B: BTreeIndexBounds<(u32, u32, u32)>>(b: B) -> impl Iterator<Item = MyRow>;

    // Following only in modules.

    /// Deletes all rows in the database state where the indexed column(s) match the bounds `b`.
    ///
    /// `b` may be:
    /// - A value for the first indexed column.
    /// - A range of values for the first indexed column.
    /// - A tuple of values for any prefix of the indexed columns, optionally terminated by a range for the next.
    ///
    /// May panic if deleting any one of the rows would violate a constraint,
    /// though as of proposing no such constraints exist.
    fn delete<B: BTreeIndexBounds<(u32, u32, u32)>>(b: B) -> u64;
}
```

Source-level visibility of types and methods should be inherited from the declared unique field, as described in [01: the `DbConnection` Type](./01-db-connection.md).

TODO: `BTreeIndexBounds` should be implemented for `&T` and ranges thereof. It should only be implemented for owned values `T: Copy`. Currently, in the Rust module library it is implemented for both `&T` and `T`, which is leading some users to insert needless `.clone()`s, possibly as recommended by their LSP or compiler. Also accept `&str` for `String` and `&[T]` for `Vec<T>`, if possible.

The `BTreeIndexBounds` trait is not user-facing, and its methods are left unspecified by this proposal, but it is implemented rougly via:

```rust
// Single-column indices
impl<T> BTreeIndexBounds<(T,)> for Range<T> {}
impl<T> BTreeIndexBounds<(T,)> for T {}

// Two-column indices
impl<T, U> BTreeIndexBounds<(T, U)> for Range<T> {}
impl<T, U> BTreeIndexBounds<(T, U)> for T {}
impl<T, U> BTreeIndexBounds<(T, U)> for (T, Range<U>) {}
impl<T, U> BTreeIndexBounds<(T, U)> for (T, U) {}

// Three-column indices
impl<T, U, V> BTreeIndexBounds<(T, U, V)> for Range<T> {}
impl<T, U, V> BTreeIndexBounds<(T, U, V)> for T {}
impl<T, U, V> BTreeIndexBounds<(T, U, V)> for (T, Range<U>) {}
impl<T, U, V> BTreeIndexBounds<(T, U, V)> for (T, U) {}
impl<T, U, V> BTreeIndexBounds<(T, U, V)> for (T, U, Range<V>) {}
impl<T, U, V> BTreeIndexBounds<(T, U, V)> for (T, U, V) {}

// &c up to 10-column indices
```

Note that only the last supplied column may be a range, so the number of implementations for a particular index tuple is linear in the length of that tuple.

It is the proposer's belief that such implementations can be easily generated using `macro_rules`. In addition to `Range<T>`, the trait should be implemented for `RangeFrom<T>`, `RangeInclusive<T>`, `RangeTo<T>`, `RangeToInclusive<T>` and `RangeFull` in the trailing position.

TODO: consider also implementing for `&T` and ranges of `&T`? Possibly not; very quickly causes explosion in number of implementations. Or maybe *only* implement for references? Erognomic and performance trade-offs abound.

Note that Rust's orphan rule effectively prevents us from generating implementations which are themselves parametric over traits. That is, we must implement separately for each range type, rather than having a single implementation like `impl<T, TBounds: RangeBounds<T> BTreeIndexBounds<(T,)> for TBounds`.

In C#, rather than trait-bounded polymorphism, we generate overloads of `filter` and `delete` for each acceptable combination.

## Example

```rust
#[spacetimedb::table(
    name = player,
    index(name = location, btree = [x, y]),
    index(name = faction, btree = [faction]),
)]
struct Player {
    #[primary_key]
    pub id: Identity,

    x: u32,
    y: u32,

    faction: String,
}

fn find_caller_player(ctx: &ReducerContext) -> Option<Player> {
    ctx.db.player().id().find(ctx.identity)
}

fn all_players_at_point(ctx: &ReducerContext, x: u32, y: u32) -> impl Iterator<Item = Player> {
    ctx.db.player().location().filter((x, y))
}

fn all_players_in_line(ctx: &ReducerContext, x_line: u32) -> impl Iterator<Item = Player> {
    ctx.db.player().location().filter(x_line)
}

fn all_players_in_line_near(ctx: &ReducerContext, x_line: u32, y_near: u32) -> impl Iterator<Item = Player> {
    ctx.db.player().location().filter((x, (y_near - 3)..(y_near + 3)))
}

fn ban_whole_faction(ctx: &ReducerContext, faction: String) -> u64 {
    ctx.db.player().faction().delete(faction)
}
```

## Reserved column and index-accessor names

To prevent name conflicts, a small set of symbols are reserved in the column and index namespaces. Attempting to define a column or an index with one of these names will cause an error at macro-expand time. Implementors are also encouraged to signal an error at publish time if a module circumvents the bindings library and manually constructs a `ModuleDef` containing one of these names in a reserved position.

We reserve the names of all methods currently proposed to be generated on `TableHandle`s in all case conventions used by supported SpacetimeDB languages (`snake_case` in Rust, `camelCase` in TypeScript and `PascalCase` in C#), the names used for managing callbacks in our SDKs (see [03: Table and Type Definitions](./03-table-and-type-definitions.md)) as well as the names of two methods we expect to define in the future, `try_delete` and `query`.

- `insert`.
- `Insert`.
- `try_insert`.
- `TryInsert`.
- `tryInsert`.
- `delete`.
- `Delete`.
- `try_delete`.
- `TryDelete`.
- `tryDelete`
- `count`.
- `Count`.
- `iter`.
- `Iter`.
- `query`.
- `Query`.
- `on_insert`.
- `OnInsert`.
- `onInsert`.
- `on_delete`.
- `OnDelete`.
- `onDelete`.
- `on_update`.
- `OnUpdate`.
- `onUpdate`.
- `remove_on_insert`.
- `RemoveOnInsert`
- `removeOnInsert`.
- `remove_on_delete`.
- `RemoveOnDelete`.
- `removeOnDelete`.
- `remove_on_update`.
- `RemoveOnUpdate`.
- `removeOnUpdate`.

## Type bounds

TODO: Nah. Burden to implement consistent ordering and equality predicates in clients for arbitrary types is too high in the short term. Still don't recognize known type names, but do use trait bounds. Specifically, add bounds to the `.filter`, `.find`, `.delete`, `.update` methods of `UniqueIndex` and `BTreeIndex` accessors that the field or key is of a type for which we implement consistent ordering in all client SDKs. Likely for now this is integers, bools, strings, identity and perhaps bytestrings. Simple enums would also be nice, if we can make it work. Perhaps have `#[derive(SpacetimeType)]` on enums check if they're C-like and if so implement the filterable trait?

Previous versions of SpacetimeDB have restricted query method generation and/or constraint application based either on trait bounds or by recognizing a small set of known types by name during macroexpansion. This has proven error-prone and confusing to users, e.g. because it inconsisently handles type aliases. The restriction is also unnecessary, as SpacetimeDB internally can implement indices and queries on arbitrary column types. We thus propose not to restrict constraints, indices or query methods based on type.

## Implementation in clients

Client-side indexing support is not widespread in SpacetimeDB SDKs as of writing, and will likely not become widespread until after this proposal is implemented. The interfaces described above are implemented and exposed in clients, even where they must be asymptotically slower due to the absence of indices.

### Consistent ordering

Client implementations are encouraged to use the same ordering in range queries to the extent convenient, but are not required to properly implement all edge cases. In particular:

- Non-ASCII strings may be ordered differently due to variations in string encoding and differing levels of Unicode strictness in the implementation.
- `NaN`, `Infinity` and denormalized floats may order arbitrarily. Implementors are reminded that IEEE 754-2008 defines a comparison function *totalOrder* which implements a total ordering on normalized floats and all NaNs, which is implemented in Rust as [`f32::total_cmp`](https://doc.rust-lang.org/std/primitive.f32.html#method.total_cmp) and [`f64::total_cmp`](https://doc.rust-lang.org/std/primitive.f64.html#method.total_cmp).

### Rust SDK: floats aren't `Ord`

The SpacetimeDB host's query engine can trivially implement range queries on arbitrary types because `AlgebraicValue` has a total ordering. The same should be relatively easy in C# and TypeScript clients: implement `IComparable` for each generated `SpacetimeType` in C#, or add a custom comparison method to each generated type in TypeScript.

In Rust, things are somewhat hairier, as the standard library assumes that the `Ord` trait is the only way to perform total-ordering comparisons, and some `SpacetimeType`s (notably `f32` and `f64`) are not `Ord`. The proposers imagine that we will define our own trait, `SpacetimeOrd`, which defines a custom total-ordering relationship. (We may also simply provide a `.ord` method on the `SpacetimeType` trait, if that is deemed easier.) The SDK library will implement this trait for all primitive and library `SpacetimeType`s. A wrapper newtype `OrdViaSpacetime<T>(pub T)` will be provided with an implementation `impl<T: SpacetimeOrd> Ord for OrdViaSpacetime<T>`. Anywhere the SDK or codegen wants to compare rows or column values (e.g. when inserting into a `BTreeMap`), it will wrap the native types in `OrdViaSpacetime`.

## Future work

### Other index types

Future work will define index types other than BTrees. Most appealing are likely hash indices, which provide faster point queries at the expense of not implementing range queries; and spatial indices, which would allow games to look up entities near a location in the game world.

We find it likely that we will want to implement unique constraints via hash indices rather than BTree indices, and so have proposed not to expose range queries via unique constraint indices.

We imagine that these index types will be declared using a macro syntax similar to BTree indices, but but that they will generate different methods, or at least methods with different signatures. For indices which can answer multiple different types of queries, the method will be generic bounded on a trait analogous to the proposed `BTreeIndexBounds` (or in C#, have overloads for the different implemented queries). For indices which can answer only a single type of query, e.g. hash indices, the method will not be generic (or overloaded), and no trait is necessary.

### Logical query API

Future work will define some interface for modules to construct query expressions and send them to the host for optimization and evaluation. The specific interface is not yet defined. We expect queries to be constructed through a method `query` generated on the `TableHandle`, which may accept a query expression as an argument or return a builder. To support this, we reserve the word `query` in the `TableHandle` method namespace.
