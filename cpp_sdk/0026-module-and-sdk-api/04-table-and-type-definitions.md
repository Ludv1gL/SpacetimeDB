# Module and SDK API: Table and Type Definitions

The syntax for type and table definitions is defined in the [macro design proposal (in review)](https://github.com/clockworklabs/SpacetimeDBPrivate/blob/centril/proposals/spacetimedb-macro/proposals/0009-spacetimedb-macro.md). Most user-facing aspects of the macroexpansion are defined in [01: `DbConnection`](./01-db-connection.md) and [02: Physical Queries](./02-physical-queries.md).

## `Debug` implementation

All `SpacetimeType`s implement `std::fmt::Debug`, so that methods described in [02: Physical Queries](./02-physical-queries.md) may generate better diagnostics in panic messages.

## Bounding indexes, constraints and filtering on types (Don't do it)

Constraint and index declaration macros do not have logic to restrict types to which they're applied, though warnings are permitted at the implementor's discretion. Such warnings should fire only for a set of known types, as opposed to currently, where they are suppressed only for a set of known types. The current implementation misbehaves in the presence of type aliases.

## Client language translation of module types

### Compound types

#### User-defined compound types

Each type defined by a module and referenced in its client-facing interface (i.e. as a table, a column of a table or a reducer argument) results in a generated file which defines a corresponding type in the client. The generated type's name is the same as the module type's name, case-converted as appropriate for the client language. The file or namespace which defines it is the module type's name, case-converted as appropriate, and suffixed with `_type`. The defined type is re-exported from the root of the `module_bindings` namespace.

We remain intentionally vague as to how compound types (sums and products) are translated. For languages which SpacetimeDB supports in both module and client contexts, we require that a compound type defined in a module of that language will generate a compound type with the same structure in a client of that type, with its components translated according to the below rules for non-compound types.

#### Optional and nullable values

SATS equates `Option<T>` in languages which use optional values with `T?` or `T | null` in types which use nullable values. A nullable `T` in a nullable-language module translates to `Option<T>` in an optional-language client, and an `Option<T>` in an optional-language client translates to a nullable `T` in a nullable-language client.

Client codegen implementors must take care not to translate `Option<Option<T>>` to `T??`, as this is generally not allowed in nullable languages. Any number of nested `Option`s around a type `T` results in `T?`.

### Primitive, builtin or non-compound types

#### Integral types

Client languages with specified-width integral types translate each integral type listed in the module to an integral type with the same precision. Where necessary, these integral types are defined by the SDK library. E.g. the Rust SDK defines types for 256-bit signed and unsigned integers, and C# SDK additionally defines types for 128-bit signed and unsigned integers.

Client languages without specified-width integral types translate each integral type listed in the module to the smallest numeric type which can store the type specified in the module with full precision. E.g. in TypeScript we translate any integer 32 bits or smaller to `number`, and any larger to `bigint` (or `BigInt`). Serializing an out-of-bounds integer value from a wider or arbitrary-width type to a fixed-width SATS integer aborts, panics or throws an error.

#### Floating-point types

Client languages with specified-width floating-point types translate each floating-point type listed in the module to a native floating-point type with the same precision. As of writing, SpacetimeDB supports only 32-bit and 64-bit IEEE 754 floats, which have native support in both Rust and C#.

Client languages without specified-width floating-point types translate any floating-point type listed in the module to a native floating-point type with at least IEEE 754 double-float precision.  e.g. in TypeScript we translate `F32` and `F64` to `number`. Serializing a value which cannot be precisely represented as a SATS `F32` may truncate or round at the implementor's discretion.

#### Booleans

We translate `Bool` to the client language's native boolean type.

#### Strings

We translate `String` to the client language's native heap-allocated variable-length owned string type.

#### Arrays

We translate `Array<T>` types to a client-appropriate heap-allocated variable-length homogeneous owned array or vector type, i.e. `Vec<T>` in Rust, `List<T>` in C#, and `T[]` in TypeScript.

#### Maps

The SATS `Map<K, V>` type is removed.

### Special types

The SpacetimeDB module library defines several types which, as of writing, are implemented as SATS compound types, but which receive special handling. This section is intended to apply to *any* `SpacetimeType` implementor defined in the module bindings crate or library. A non-exhaustive list of these types follows:

- `Identity`.
- `Address`.
- `ScheduleAt`.

Each of these types receives special handling by client codegen, and generates as a reference to a named type defined by the SDK library.

### Timestamps

The module library type `Timestamp` is removed. Instead, each module and SDK library contains an implementation of `SpacetimeType` for its language-appropriate point-in-time type: `SystemTime` in Rust, `DateTimeOffset` in C#, and `Date` in TypeScript. These all correspond to a SATS type which receives special handling in the same manner as the above "Special types" defined by the module bindings library, and result in codegen with the language-appropriate point-in-time type.

### Durations

Instances where the client SDK or module library must represent a duration or time span use a language-appropriate type: `Duration` in Rust or `TimeSpan` in C#. In languages without a common type which serves this purpose, e.g. TypeScript, the SDK library exports a type `Duration` which serves this purpose. As with timestamps, all of these duration types share a SATS type and are translated as equivalent during codegen.

`ScheduleAt::Interval` stores a duration object rather than a `u64`.

## Table codegen

As described above, for each type with at least one `#[spacetimedb::table]` annotation in the module, the CLI generates a file containing a type definition. This is *just* a type definition, and this file does not contain any table-related operators.

For each `#[spacetimedb::table(name = foo)]` annotation in the module (potentially multiple for a single type), the CLI generates a file whose name is the appropriate case-conversion of the table name, suffixed with `_table`. This file contains a `TableHandle` struct as described in [01: `DbConnection`](./01-db-connection.md), and an extension trait or method on `RemoteTables` named with the appropriate case-conversion of the table name which returns said `TableHandle`. The extension trait or method is re-exported from the root of the `module_bindings` namespace, but the `TableHandle` type is not.

## Clients: Row callbacks

In clients, interfaces are provided on generated `TableHandle` structs for managing row callbacks. Future work may implement the same interface in modules as triggers.

As described in [01: `DbConnection`: Note on cross-language implementation](./01-db-connection.md#note-on-cross-language-implementation), these interfaces are permitted to vary between implementation languages more than most proposed interfaces.

### For all tables: `on_insert` and `on_delete`

All tables expose two callbacks, `on_insert` and `on_delete`, each of which is of type `FnMut(&EventContext, &Row)`, where `Row` is the table row type, and `EventContext` is the autogenerated module-specific `EventContext` type, as described in [06: SDK Callbacks](./06-sdk-callbacks.md). `on_insert` is invoked whenever a row is added to the client cache (except during primary-key updates; see below), and `on_delete` whenver a row is removed from the client cache (same caveat re: primary-key updates).

#### Rust

In Rust, methods for managing `on_insert` and `on_delete` callbacks are provided on the `Table` trait in clients.

```rust
trait Table {
    // other members elided

    /// The module-specific autogenerated `EventContext` type.
    type EventContext;
    /// The generated row type.
    type Row;

    type InsertCallbackId;

    /// Multiple insert callbacks on the same reducer can coexist. Invocation order is undefined.
    fn on_insert(&self, callback: impl FnMut(&Self::EventContext, &Self::Row)) -> Self::InsertCallbackId;

    type DeleteCallbackId;

    /// Multiple delete callbacks on the same table can coexist. Invocation order is undefined.
    fn on_delete(&self, callback: impl FnMut(&Self::EventContext, &Self::Row)) -> Self::DeleteCallbackId;

    /// May panic if `callback_id` does not refer to a registered insert callback.
    fn remove_on_insert(&self, callback_id: Self::InsertCallbackId);

    /// May panic if `callback_id` does not refer to a registered delete callback.
    fn remove_on_delete(&self, callback_id: Self::DeleteCallbackId);
}
```

#### C#

In C#, the abstract class `Table` has `event` fields `OnInsert` and `OnDelete`.

TODO: C# definition

#### TypeScript

TODO: TypeScript design.

### For tables with a declared primary key: `on_update`

Tables with a declared primary key column additionally expose a callback `on_update`, of type `FnMut(&EventContext, &Row, &Row)`. This is called whenever, within a single transaction, a delete and an insert on the same table have the same value in the primary key column. The first `&Row` argument is the old row which has been deleted, and the second `&Row` argument is the new row which has been inserted. Note that an update can never result from a subscription being applied or removed, only during an incremental update as the result of a transaction.

#### Rust

In Rust, methods for managing `on_update` callbacks are provided by a trait `TableWithPrimaryKey` which has `Table` as a supertrait. `TableWithPrimaryKey` is only implemented for tables with primary keys.

```rust
pub trait TableWithPrimaryKey: Table {
    type UpdateCallbackId;

    /// Multiple update callbacks on the same table can coexist. Invocation order is undefined.
    fn on_update(&self, callback: impl FnMut(&Self::EventContext, &Self::Row, &Self::Row)) -> Self::CallbackId;

    /// May panic if `callback_id` does not refer to a registered update callback.
    fn remove_on_update(&self, callback_id: Self::CallbackId);

    // Trait may have other non-user-facing members.
}
```

#### C#

In C#, an abstract class `TableWithPrimaryKey` which subclasses `Table` is defined with an `event` field `OnUpdate`.

TODO: C# definition.

#### TypeScript

TODO: TypeScript design.
