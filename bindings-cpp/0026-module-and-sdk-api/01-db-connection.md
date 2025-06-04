# Module and SDK API: the `DbConnection` Type

* Proposal: [0026/01](0026-module-and-sdk-api/01-db-connection.md)
* Authors: [Phoebe Goldman](https://github.com/gefjon), [Jeremie Pelletier](https://github.com/lcodes)
* Status: **Draft**
* Previous Proposals:
  * [Macro design (in review)](https://github.com/clockworklabs/SpacetimeDBPrivate/blob/centril/proposals/spacetimedb-macro/proposals/0009-spacetimedb-macro.md)
  * [Subscription manipulation (in review)](https://github.com/clockworklabs/SpacetimeDBPrivate/blob/phoebe/proposal-multiple-subscriptions/proposals/0000-subscription-manipulation.md)

## Motivation & Goals

As of writing, SpacetimeDB user code in both modules and SDK-ful clients treats the connection to the database as a global, performing implicit accesses to it using free functions and static methods. This has already been a limitation on clients, as it prevents a single SDK-ful process from maintaining multiple parallel connections to different modules. Future module features, such as inter-module communication and partitioning, may also be limited.

Note that the current SDK design where the connection is a global is part of the SDK library, i.e. it is a per-process global, not a per-module per-process global.

Also, we intend to add new non-transactional contexts in which module WASM code can run without access to the database, e.g. in functions which return prepared queries or views. Our current system allows us to prevent these accesses at runtime via a WASM trap (and we must still do that to be resilient against WASM blobs which make raw host calls without using the bindings library), but that's a poor user experience compared to a type-checkable compile-time diagnostic.

As such, this proposal seeks to move the database connection, at least at the user-facing source code level, to an explicit object which receives methods, rather than an implicit global affected by free functions. In SDKs, multiple such objects can coexist, referring to different remote databases. In modules, we can control what code has access to such an object; reducers will take it as an argument, granting them access to the database, but contexts which should not have access to the database will not take it as an argument.

A not strictly related concern which nonetheless is best addressed at the same time is that, as of writing, the SpacetimeDB bindings library and macros equate source-level types with table definitions. This is a sensible default, but we have already encountered cases where it would be beneficial to have two or more tables with the same type. E.g. it may be desirable to have separate tables `logged_in_user` and `logged_out_user` both with rows of type `User`. The current design imposes unnecessary boilerplate on this pattern, requiring separate types `LoggedInUser` and `LoggedOutUser` with conversions between them duplicated in the module and client code.

As such, this proposal seeks to change the association from types to tables to be one-to-many, where previously it was one-to-one.

## Constraints

- Modules and clients must use equivalent or nearly-equivalent code to access the database or client cache.
  - Ideally it should be possible to copy and paste a read-only query between the two contexts with no modifications. (The code will differ from the compiler's perspective due to different types. Advanced configuration/dependency hackery could allow it to be literally shared via a library upstream of both, but this proposal is not concerned with making that level of sharing easy.)
  - This requirement is motivated by ease of use both for new users learning to write both Spacetime modules and clients, and also to simplify cognitive overhead for existing users switching between the two contexts.
- In client code, it must be possible to use the SpacetimeDB SDK to maintain multiple connections to the same or different modules running on the same or different hosts.
  - I.e. no hidden global state.
- The SpacetimeDB SDK and module library should abstract away all concurrency and shared access concerns, i.e. a user should never need to explicitly interact with an `Arc` or `Mutex` to access the database.
- The proposed design must be extensible to enable inter-module communication, including:
  - Having one database connect to another module using equivalent or nearly-equivalent code as in a client.
  - Maintaining such a connection across reducer runs, likely by storing it in either a system- or user-defined table.
  - Subscribing to a subset of the remote module and accessing them using equivalent or nearly-equivalent code as in a client.
  - Registering callbacks to run in response to subscribed events in the remote module.
  - Requesting that the remote module run reducers, and observing their status.
  - In short, writing all the same code for interacting with a remote module as in a client using the SDK, even if the semantics may differ slightly due to transactionality and the implementation may be completely separate.
- The SpacetimeDB host does not know, and does not need to know, anything about table and reducer-argument types, except for their corresponding SATS structural types. Table and reducer-argument types are abstractions that are contained within the client/module language.

## Non-goals

- Backwards-compatibility.

## Proposed Solution

Define a trait, interface or class `DbContext` in both the module library and the SDK library, representing a handle on either a remote or local database. In modules, this has one associated type or generic parameter, `DbView`, which mediates access to the tables of a specific module's schemas. In clients, it additionally has a second associated type or generic parameter, `Reducers`, which mediates reducer callbacks and invocations.

`DbContext<DbView = Db>` has a property `db: Db`. In clients, `DbContext<DbView, Reducers = R>` additionally has a property `reducers: R`. In Rust, where traits cannot have properties, these are methods, but every implementor additionally has these public fields. Providing both trait methods and fields allows users to write code generic over the `DbContext`, while still using the more succinct field-access syntax in common cases.

In modules, `DbContext` is implemented for `ReducerContext` with `DbView = Local`, where `Local` is a type defined in the module bindings library.

In clients, `DbContext` is implemented for `DbConnection` and `EventContext`, both defined in the `module_bindings`. Both implementors have `DbView = RemoteTables` and `Reducers = RemoteReducers`, where `RemoteTables` and `RemoteReducers` are types defined in the `module_bindings`.

All accesses to tables in the database or client cache are performed via `&self` shared reference receiver methods on the `DbView` of a `DbContext`, rather than free functions or static methods.

In clients, all requests to invoke reducers are, and all management of reducer callbacks is, performed through `&self` shared reference receiver methods on the `Reducers` of a `DbContext`, rather than free functions or static methods.

On clients, all operations which manage a connection to a remote server are trait methods of `DbContext`. In modules, a small number of host calls are explicitly called out below as happening via free functions, but the majority are also trait methods on `DbContext`.

In modules, the `DbView` is implemented as an empty type, and its methods simply make host calls via the WASM module ABI. In clients, the `DbConnection` holds all state associated with the connection, including the WebSocket handle, client cache and callbacks, and the `EventContext` borrows or contains `Arc`s to this state.

Accesses to tables within a `DbView` are performed through a `TableHandle` intermediate. This is an autogenerated type specific to the table being accessed, which has methods corresponding to the previous `TableType` associated functions. `DbView` has a `&self` shared receiver method for each table defined in the module which returns a `&TableHandle`. These methods on `DbView` are provided through an extension trait in Rust or as an extension method in C#. This method, and the trait which provides it if any, shares the same name as the table, which notably is distinct from the source-level type name for the table's rows.

See [the Rust RFC book](https://rust-lang.github.io/rfcs/0445-extension-trait-conventions.html) and [Karol Kuczmarski's blog](http://xion.io/post/code/rust-extension-traits.html) for information on extension traits in Rust, and [Microsoft's documentation](https://learn.microsoft.com/en-us/dotnet/csharp/programming-guide/classes-and-structs/extension-methods) for information on extension methods in C#.

### A simple example module

TODO: Update example for new auth stuff following Tyler's Identity Proposal.

#### Rust

```rust
#[spacetimedb::table(
    name = user,
    visibility = public,
)]
pub struct User {
    #[auto_inc]
    #[primary_key]
    pub id: u64,

    #[unique]
    pub owner: Identity,

    pub name: String,
}

#[spacetimedb::reducer]
pub fn add_user(ctx: &ReducerContext, name: String) {
    ctx.db.user().insert(User {
        id: 0, // auto_inc placeholder
        owner: ctx.sender,
        name
    }).unwrap();
}

#[spacetimedb::reducer]
pub fn greet_all_users(ctx: &ReducerContext) {
    for user in ctx.db.user().iter() {
        log::info!("Hello, {}", user.name);
    }
}
```

#### C#

```csharp
namespace Project {
    using SpacetimeDB;

    [Table(Name = "User", Public = true)]
    public struct User {
        [AutoInc]
        [PrimaryKey]
        public ulong Id;

        [Unique]
        public Identity Owner;

        public string Name;
    }

    // C# doesn't allow top-level functions, so they're wrapped in a static class.
    public static class Module {
        [Reducer]
        public static void AddUser(ReducerContext ctx, string name) {
            ctx.Db.User.Insert(new() {
                Id = 0,
                Owner = ctx.Sender,
                Name = name
            });
        }

        [Reducer]
        public static void GreetAllUsers(ReducerContext ctx) {
            foreach (var user in ctx.Db.User.Iter()) {
                Console.WriteLine($"Hello, {user.Name}");
            }
        }
    }
}
```

## Detailed Design

### Notes on cross-language implementation

Code is presented here in Rust, as the author is most fluent in it compared to our other supported languages, C# and TypeScript. The intention is that C# and TypeScript implementations of this proposal will translate all public interfaces as literally and directly as possible, with the following exceptions:

- Names should be case-converted to the convention of the language.
- User-supplied names should be encouraged to match the convention of the language.
- Differences in how users import generated extension methods are permitted where necessary.
- Interfaces for registering and removing callbacks may be altered to suit the language's tooling. The user-facing names should be the same modulo case-conversion.
- Error types should be translated to a language-appropriate dynamic error type without exhaustive matching, so that we may define new variants without breaking compatibility.
- Specified traits may be implemented as superclasses or interfaces as appropriate.
- Associated types may be implemented as generic parameters in languages without support for associated types, functional dependencies or type families.
- In languages with subclassing or subtyping, non-user-facing superclasses or supertypes may be added at the implementor's discretion.

#### Callbacks in Rust

Callback-driven interfaces are generally not idiomatic Rust. As such, we are left with a dearth of examples for how to best expose callbacks in the Rust SDK. If we prioritized idiomatic Rust design, the answer would of course be to not structure our interface around callbacks at all. However, we are interested in presenting a consistent API in all of Rust, C# and TypeScript, and in C# and TypeScript callbacks are very much the obvious choice. For that reason, we develop the following convetions for callbacks in Rust, [based on a rough adaptation of the NodeJS `EventEmitter` API](https://nodejs.org/api/events.html):

- Each callback `foo` has a function or method `on_foo` which registers the callback. In JS, this would be a call to `.on` or `.addListener` (or similar) with a particular `eventName` argument, but Rust's strong typing prevents us from re-using the same method for distinct events where the callbacks will have different argument types.
- Each callback `foo` which can be meaningfully removed or canceled has its `on_foo` return an opaque `FooCallbackId`, implemented as an integer index into some structure which holds the `Box<dyn Fn>` object (or what have you). A corresponding function or method `remove_on_foo` takes this `FooCallbackId` as an argument and removes the callback. In JS, this would be a call to `.removeListener` or `.off` (or similar) with the same `eventName` and function object arguments, but in Rust we cannot generally retain the same function object until removing it, since `FnMut` and `Fn` closures are not `Clone`, `Copy`, `PartialEq`, `Eq` or `Hash`.
- Callbacks are typed at `FnOnce` if they will only run once, or `FnMut` if they are expected to run multiple times. Using `FnMut` over the more-restrictive `Fn` is a choice prioritizing ergonomics for users, potentially at the expense of performance, as the SDK will have to implicitly synchronize calls to the closures rather than placing the burned on the user to explicitly synchronize closed-over state.

#### Callbacks in TypeScript

TODO: Describe how we impl callbacks in TypeScript. Ingvar suggests we use [the standard `EventTarget` API](https://developer.mozilla.org/en-US/docs/Web/API/EventTarget). Probably we remove `on_`/`On` from the callback names, and otherwise use `camelCase` names, like `addEventListener("subscriptionApplied")`.

#### Method namespacing and importing

Different supported languages have significantly different semantics for what methods do or do not require importing a trait, interface or class to invoke. In particular, in C# and TypeScript, methods defined on an object are always in scope, even if they are defined via an interface which is not in scope, whereas in Rust, trait methods are only in scope if the trait which defines them is imported.

Previous iterations of this proposal sought to paper over these differences by defining intrinsic methods on all concrete types corresponding to the trait methods, thus emulating the C# and TypeScript behavior. The latest revision opts not to do this, as the method duplication causes tangibly worse behavior in IDEs, particularly w.r.t. jump-to-definition and documentation pop-ups. We instead specify that Rust code must import the appropriate trait, whereas C# and TypeScript need not import anything to gain access to its methods. Our expectation is that `rustc`'s relatively good compiler diagnostics will prevent this from being a problem.

This means that in Rust, each specified method must be exposed on exactly the proposed trait, whereas in C# methods may be defined on super-classes or -interfaces. The Rust implementation may define non-user-facing supertraits to aid implementation, but users must never be required to name or import those supertraits to access any of the methods or functionality described below.

#### Generic types and interfaces

Many languages do not support associated types. Some of these have other mechanisms for signaling to the compiler that a relationship between one type or constraint and another is one-to-one rather than one-to-many, while other languages have no way at all to define this.

Where relevant, we specify traits with associated types to make clear the one-to-one relationship. Proposed traits with associated types may be implemented in languages without associated types as:

- Any one of the dozen-or-so type system features approximately equivalent to associated types, incl. type families, functional dependencies or module types. (No language we support as of writing falls into this category.)
- Regular generic parameters. (C# and TypeScript.)

Because languages without associated types will, naturally, be less able to infer generic parameters, we specify autogenerated fully-applied aliases. For consistency, these aliases are defined even in languages with associated types (or equivalent).

### Note on namespace exports

Unless otherwise specified, all definitions are assumed to be exported from the root namespace of the library or module which defines them.

### The `DbContext` trait

#### In modules

The bindings crate or library defines a trait `DbContext`. This trait has at least the following members:

```rust
/// A handle on a database with a particular table schema.
pub trait DbContext {
    /// A view into the tables of a database.
    ///
    /// This type is specialized on the database's particular schema.
    ///
    /// Methods on the `DbView` type will allow querying tables defined by the module.
    type DbView;

    /// Get a view into the tables.
    ///
    /// This method is provided for times when a programmer wants to be generic over the `DbContext` type.
    /// Concrete-typed code is expected to read the `.db` field off the particular `DbContext` implementor.
    /// Currently, being this generic is only meaningful in clients,
    /// as modules have only a single implementor of `DbContext`.
    fn db(&self) -> &'ctx Self::DbView;
}
```

A fully-applied alias `LocalDbContext` is defined 

C# has no traits or associated types, but can use subclassing and generics to the same effect:

```csharp
namespace SpacetimeDB {
    public abstract class DbContext<DbView> where DbView : struct {
        public readonly DbView Db = new();
    }
}
```

Future work will extend this trait with additional members.

#### In SDKs

The SDK library defines a trait `DbContext` analogous to the one defined by the bindings library.

In addition to the `DbView` type and the `db` accessor, `DbContext` has the following members:

##### Rust

```rust
// ###### In the SDK library, the maximally-generic `DbContext`.
pub trait DbContext {
    // Above members elided.

    /// A handle on the reducers exposed by a particular database.
    ///
    /// This type is specialized on the database's particular schema.
    ///
    /// Methods on the `Reducers` type will allow invoking reducers defined by the module,
    /// and registering callbacks to run in response to those reducers.
    type Reducers;

    /// Get a handle on the reducers.
    ///
    /// This method is provided for times when a programmer wants to be generic over the `DbContext` type.
    /// Concrete-typed code is expected to read the `.reducers` field off the particular `DbContext` implementor.
    fn reducers(&self) -> &Self::Reducers;

    /// True if the connection is still alive; false if we have been disconnected.
    fn is_active(&self) -> bool;

    /// Close this connection.
    ///
    /// Returns an `Err` if this connection has already closed.
    fn disconnect(&self) -> Result<()>;

    /// See 07: Subscriptions.
    type SubscriptionBuilder;
    fn subscription_builder(&self) -> Self::SubscriptionBuilder;
}
```

##### C#

TODO: Make match [07: Subscriptions](./07-subscriptions.md).
TODO: Add non-generic alias.

```csharp
namespace SpacetimeDB {
    public abstract class DbContext<DbView, ReducerView> where DbView : struct where ReducerView : struct {
        public readonly DbView Db = new();
        public readonly Reducers Reducers = new();

        public bool Active => false;

        public event Action? OnSubscriptionApplied;

        public void Subscribe(IReadOnlyList<string> sql) { }

        public void Disconnect() { }
    }
}
```

### `DbView` (and `Reducers`) types

#### In modules: `Local`

The module bindings crate/library defines a type `Local`, to be used as the `DbView` type for `DbContext` implementations. `Local` is an empty type which does not implement `Copy`, `Clone` or `SpacetimeType`, which user code only interacts with by shared reference through a `ReducerContext` (see below).

Methods on `Local` and its `TableHandle`s (see below) make WASM host calls according to the SpacetimeDB WASM ABI.

#### In SDKs: `RemoteTables` and `RemoteReducers`

In clients, the `module_bindings` codegen for a particular module schema defines two types, `RemoteTables` and `RemoteReducers`, to be used as the `DbView` and `Reducers` types respectively for `DbContext` implementations. As with `Local`, `RemoteTables` and `RemoteReducers` do not implement `Copy`, `Clone` or `SpacetimeType`. Unlike `Local`, `RemoteTables` and `RemoteReducers` may contain runtime state, but this state is not considered part of the user-facing API.

### Implementors of `DbContext`

#### In modules: `ReducerContext`

The only implementor of `DbContext` in modules is `ReducerContext`. `ReducerContext` is defined by the bindings crate/library, and implements `DbContext<Item = Local>`.

Implementing `DbContext` for `ReducerContext` does not require any non-zero-sized state in `ReducerContext` or `Local`, as all `Local` methods simply make WASM host calls according to the SpacetimeDB WASM ABI.

`ReducerContext` is extended with a field `pub db: Local`, to allow concrete-typed code to access the database view through the more consise field-access syntax rather than the more verbose method-call syntax.

As with `Local`, `ReducerContext` does not implement `Copy`, `Clone` or `SpacetimeType`. The `ReducerContext` argument to reducers is now mandatory and taken by reference, i.e. adding the `#[spacetimedb::reducer]` attribute to a function without a first argument of type `&ReducerContext` is a compile-time error.

#### In clients: `DbConnection`

In clients, the `module_bindings` codegen for a particular module schema defines a type `DbConnection`, representing a connection to a remote database. `DbConnection` holds all the state that was previously stored in globals defined by the SDK. This includes, but is not limited to:
- The client cache.
- Any registered callbacks.
- The client's `Identity` and `Address`.
- The WebSocket connection handle.

As with `ReducerContext`, `DbConnection` has two user-visible fields, `pub db: RemoteTables` and `pub reducers: RemoteReducers`.

Additionally, `DbConnection` has one associated function or static method:

```rust
impl DbConnection
    pub fn builder() -> spacetimedb_sdk::DbConnectionBuilder<Self>;
```

```csharp
namespace SpacetimeDB {
    public abstract class DbConnection<T> where T : DbConnection<T> {
        public static DbConnectionBuilder<T> Builder() => new();
    }
}
```

##### `DbConnectionBuilder` interface

`DbConnection`s are constructed via a builder-pattern `DbConnectionBuilder<DbConnection>`. `DbConnectionBuilder` is defined in the SDK library and is generic over the particular module's `DbConnection` type (presumably bounded by some non-user-facing trait).

Note that no direct user-facing constructor is provided for `DbConnectionBuilder`. `DbConnectionBuilder`s are constructed by the `builder` static method or associated function on `DbConnection`.  The intended design is that users will rarely or never explicitly name a `DbConnectionBuilder` type.

It has the following methods:

###### Rust

```rust
pub struct DbConnectionBuilder<T> {
    // private fields
}

impl<DbConnection: SomeNonUserVisibleBound> DbConnectionBuilder<DbConnection> {
    /// Initiate a connection to a remote module using the parameters contained in `self`.
    ///
    /// Returns immediately; does not wait for the server to acknowledge the connection.
    /// Users may observe the status of the connection
    /// via [`Self::on_connect`] and [`Self::on_connect_error`] callbacks.
    ///
    /// Returns an `Err` if networking syscalls fail, as this represents a potentially recoverable condition.
    ///
    /// Panics if invalid parameters are supplied or required parameters are not supplied,
    /// as this represents an unrecoverable bug in the client code.
    pub fn build(self) -> Result<DbConnection>;

    /// Sets the URI and protocol of the remote SpacetimeDB host.
    pub fn with_uri(self, uri: impl Into<Uri>) -> Self;

    /// Sets the name or address of the remote module.
    ///
    /// Use of `impl Into<String>` as the argument type here is not an exact prescription,
    /// but it should at least be possible to supply either
    /// a string literal or a computed heap-allocated string
    /// in languages where these are distinct types.
    pub fn with_module_name(mut self, name_or_address: impl Into<String>) -> Self;

    /// Sets the client's credentials for the new connection.
    ///
    /// If not supplied, or supplied as `None`, the new connection will be anonymous.
    ///
    /// A later call to `with_credentials` will overwrite a previous call on the same builder.
    /// A call like `builder.with_credentials(Some(thing)).with_credentials(None).build()`
    /// will connect anonymously.
    pub fn with_credentials(self, credentials: Option<(Identity, String)>) -> Self;

    /// Register a callback to run when the connection is confirmed by the server
    /// and the client's identity and token are received.
    ///
    /// The `&str` argument to the callback is the token.
    /// It is passed borrowed here so that the `DbConnection` may retain ownership of the token.
    ///
    /// This must be registered prior to initiating the connection to prevent race conditions.
    ///
    /// To simplify SDK implementation, at most one such callback can be registered.
    /// Calling `on_connect` on the same `DbConnectionBuilder` multiple times panics or throws.
    ///
    /// Unlike callbacks registered via [`DbConnection`],
    /// no mechanism is provided to un-register the provided callback.
    /// This is a concession to ergonomics; there's no clean place to return a `CallbackId` from this method
    /// or from `build`.
    pub fn on_connect(self, callback: impl FnOnce(&DbConnection, Identity, &str)) -> Self;

    /// Register a callback to run when the connection fails or is rejected by the server,
    /// with an error message as an argument.
    ///
    /// This must be registered prior to initiating the connection to prevent race conditions.
    ///
    /// To simplify SDK implementation, at most one such callback can be registered.
    /// Calling `on_connect_error` on the same `DbConnectionBuilder` multiple times panics or throws.
    ///
    /// Unlike callbacks registered via [`DbConnection`],
    /// no mechanism is provided to un-register the provided callback.
    /// This is a concession to ergonomics; there's no clean place to return a `CallbackId` from this method
    /// or from `build`.
    pub fn on_connect_error(self, callback: impl FnOnce(&anyhow::Error)) -> Self;

    /// Register a callback to run when a [`DbConnection`] whose connection initially succeeded
    /// is disconnected, either after a [`DbConnection::disconnect`] call or an error.
    ///
    /// If the connection ended due to an error, passes said error to the callback.
    ///
    /// The `callback` will be installed on the `DbConnection` created by `build`
    /// before initiating the connection, so that there's no opportunity for the disconnect to happen
    /// before the callback is installed.
    ///
    /// Note that this does not trigger if `build` fails
    /// or in the cases where [`Self::on_connect_error`] would trigger.
    /// This callback only triggers if the connection closes after `build` returns successfully
    /// and [`Self::on_connect`] is invoked, i.e. after the `IdentityToken` is received.
    ///
    /// To simplify SDK implementation, at most one such callback can be registered.
    /// Calling `on_disconnect` on the same `DbConnectionBuilder` multiple times panics or throws.
    ///
    /// Unlike callbacks registered via [`DbConnection`],
    /// no mechanism is provided to un-register the provided callback.
    /// This is a concession to ergonomics; there's no clean place to return a `CallbackId` from this method
    /// or from `build`.
    pub fn on_disconnect(self, callback: impl FnOnce(&DbConnection, Option<&anyhow::Error>)) -> Self;
}
```

###### C#

Descriptions of the methods written in the Rust interface apply.

```csharp
namespace SpacetimeDB {
    public class DbConnectionBuilder<DbConnection> where DbConnection : DbConnectionBase<DbConnection> {
        public DbConnection Build();

        public DbConnectionBuilder<DbConnection> WithUri(Uri uri);

        public DbConnectionBuilder<DbConnection> WithModuleName(string nameOrAddress);

        public DbConnectionBuilder<DbConnection> WithCredentials(in (Identity, string)? creds);

        public DbConnectionBuilder<DbConnection> OnConnect(Action<Identity, string> cb);

        public DbConnectionBuilder<DbConnection> OnConnectError(Action<Exception> cb);

        public DbConnectionBuilder<DbConnection> OnDisconnect(Action<DbConnection, Exception?> cb);
    }
}
```

##### Example - construction

In the below example, assume that `maybe_get_creds` is of type `fn() -> Option<(Identity, String)>`, and retrieves an identity and token saved to persistent local storage from a previous connection if one exists, or returns `None` if no such credentials are saved. The full SDK API stability proposal will define an interface which fills this role, though it will vary slightly from the `maybe_get_creds` in the example below.

###### Rust

```rust
use module_bindings::DbConnection;
fn my_connect() -> Result<DbConnection> {
    let mut builder = DbConnection::builder()
        .with_module_name("my_module")
        .with_credentials(maybe_get_creds())
        .on_connect(|ident, token| println!("Connected with {ident:?}, {token:?}"))
        .on_connect_error(|err| eprintln!("Failed to connect: {err:#?}"))
        .on_disconnect(|_conn, err| {
            if let Some(err) = err {
                eprintln!("Disconnected erroneously: {err:#?}");
            } else {
                println!("Disconnected normally");
            }
        })
        .build()
}
```

###### C#

```csharp
static class MyProject {
    static (Identity, string)? MaybeGetCreds() => null;

    public static DbConnection MyConnect() => DbConnection.Builder()
        .WithModuleName("MyModule")
        .WithCredentials(MaybeGetCreds())
        .OnConnect((ident, token) => Console.Out.WriteLine($"Connected with {ident}, {token}"))
        .OnConnectError(ex => Console.Error.WriteLine($"Failed to connect: {ex}"))
        .OnDisconnect((conn, ex) => {
            if (ex != null) {
                Console.Error.WriteLine($"Disconnected erroneously: {ex}");
            }
            else {
                Console.Out.WriteLine("Disconnected normally");
            }
        })
        .Build();
}
```

#### In clients: `EventContext`

In clients, the `module_bindings` codegen additionally defines a type `EventContext`, analogous to the module's `ReducerContext`. `EventContext` is passed by reference as the argument to callbacks which run in response to module-side events, e.g. reducers and SQL queries, as well as subscription-related callbacks.

`EventContext` implements `DbContext` with `DbView = RemoteTables` and `Reducers = RemoteReducers`. It also has fields `pub db: RemoteTables` and `pub reducers: RemoteReducers`.

If the main connection continues processing events and updating the client cache in parallel with callback execution, as it does in the Rust SDK, the `RemoteTables` view in an `EventContext` should use internal magic to provide a consistent view of the eventful state for the duration of callback execution.

Other members of `EventContext` are left intentionally vague by this proposal, to be specified later in the API stability process. It is intended to contain the information currently provided as `Event` or `ReducerEvent`. This means at least, for events which were the result of reducers, the reducer's name, its arguments and its termination status.

### Table access methods

#### In modules

The `#[spacetimedb::table]` attribute is modified to take a required explicit table name argument. This attribute may be applied to the same `struct` definition multiple times with different name arguments. Table names must be unique within a module.

The `#[unique]`, `#[auto_inc]` and `#[primary_key]` column attributes are extended to accept an optional table name argument, supplied as `table = <name>`. When these attributes specify a table name, the attribute is applied to only that table; when no table name is supplied, the attribute is applied to all tables which share that type.

##### Example - table definition

###### Rust

```rust
#[spacetimedb::table(
    public, // `my_table_1` is public, i.e. visible to non-owner clients.
    name = my_table_1,
    index(btree(columns = [name])), // indexed only in `my_table_1`
)]
#[spacetimedb::table(
    private,  // `my_table_2` is private, i.e. not visible to non-owner clients.
    name = my_table_2,
)]
pub struct MyTable {
    pub name: String,

    #[auto_inc] // auto_inc in both `my_table_1` and `my_table_2`
    pub foo: u32,

    #[unique(table = my_table_2)] // unique in only `my_table_2`
    pub bar: u32,
}
```

###### C#

```csharp
namespace Project {
    [Table(Name = "MyTable1", Public = true)]
    [Table(Name = "MyTable2")]
    public struct MyTable {
        public string Name;

        [AutoInc]
        public uint foo;

        [Unique(Table = "MyTable2")]
        public uint bar;
    }
}
```

##### Expansion

Each `#[spacetimedb::table(name = foo)]` attribute generates an extension trait or method on `Local` named `foo` which returns `TableHandle<'_>`, where `TableHandle` is a generated type which represents a handle on a specific table in the database. Because the table name will be used as a method name, the convention for the name is to match the module language's convention for method names: in Rust, tables should be named in snake case, while in C# they should be named in Pascal case. In Rust, the extension trait is named `FooTableAccess`, (i.e. `table_name.to_case(Case::Pascal) + "TableAccess"`) so it can be imported.

The generated `TableHandle` type need not be convenient to name or import, as users are expected to access it transiently through the `foo` method. It cannot be constructed except by the `foo` extension method. Note that the `#[non_exhaustive]` attribute is insufficient here, as that attribute affects only downstream crates, but table handles will generally be used within the defining crate.

Rust or C# visibilities are inherited from appropriate items in the type definition. In the example expansion, items which are always public are marked `pub`, items which are never private are unmarked, and items which inherit their visibility are marked `#vis`. Note that all `TableHandle` generated methods must be at least `pub(super)`, and items where `#vis` is `pub(super)` must be rewritten as `pub(in super::super)`. Implementations of uninteresting methods are elided.

The above example expands into roughly:

###### Rust

```rust
#vis struct MyTable {
    #vis name: String,
    #vis foo: u32,
    #vis bar: u32,
}

#vis mod my_table_1_impl {
    pub struct TableHandle<'a> {
        _private_field: PhantomData<&'a Local>,
    }

    pub trait MyTable1TableAccess {
        // Lifetime specified unnecessarily for clarity.
        fn my_table_1<'a>(&'a self) -> TableHandle<'a>;
    }

    impl MyTable1TableAccess for Local {
        // Lifetime specified unnecessarily for clarity.
        fn my_table_1<'a>(&'a self) -> TableHandle<'a> {
            TableHandle {
                _private_field: (),
            }
        }
    }

    impl<'a> TableHandle<'a> {
        // Methods shown here are for demonstration only, and are not normative.
        // See 02: Physical Queries for the normative definitions of the methods that go here.
        #vis fn iter(&self) -> impl Iterator<Item = MyTable>;
        #vis fn insert(&self, row: MyTable);

        // other methods previously on `MyTable` or `TableType`...
    }
}

#vis use my_table_1_impl::MyTable1TableAccess;

#vis mod my_table_2_impl {
    pub struct TableHandle<'a> {
        _private_field: PhantomData<&'a Local>,
    }

    pub trait MyTable2TableAccess {
        // Lifetime specified unnecessarily for clarity.
        fn my_table_2<'a>(&'a self) -> TableHandle<'a>;
    }

    impl MyTable2TableAccess for Local {
        // Lifetime specified unnecessarily for clarity.
        fn my_table_2<'a>(&'a self) -> TableHandle<'a> {
            TableHandle {
                _private_field: PhantomData,
            }
        }
    }

    impl<'a> TableHandle<'a> {
        // Methods shown here are for demonstration only, and are not normative.
        // See 02: Physical Queries for the normative definitions of the methods that go here.

        #vis fn iter(&self) -> impl Iterator<Item = MyTable>;
        #vis fn insert(&self, row: MyTable);

        // other methods previously on `MyTable` or `TableType`...
    }
}

pub use my_table_2_impl::MyTable2TableAccess;
```

###### C#

```csharp
namespace Project {
    using SpacetimeDB;

    namespace TableHandles {
        public readonly struct MyTable1 {
            public readonly IEnumerable<Project.MyTable> Iter();

            public readonly void Insert(in Project.MyTable row);

            // ...
        }

        public readonly struct MyTable2 {
            public readonly IEnumerable<Project.MyTable> Iter();

            public readonly void Insert(in Project.MyTable row);

            // ...
        }
    }
}

namespace SpacetimeDB {
    public static class Local {
        public static readonly TableHandles.MyTable1 MyTable1 = new();

        public static readonly TableHandles.MyTable2 MyTable2 = new();

        // ...
    }
}
```

##### Notes on implementation

Note that the specific structure of the nested modules and the `TableHandle` structs is unimportant. The only constraints on implementation are:
- The method for retrieving a table handle from a database handle must have the same name as the table itself, as given by the `name` argument to the `spacetimedb::table` attribute.
- The listed methods on the table handle must exist.
- Within the same file, namespace or module (Rust `mod`) as the table definition, no imports are required to retrieve the table handle from the database handle, or to invoke methods on the table handle.
- In a file, namespace or module other than that of the table definition, importing the type name and the table name from the namespace which defines the table is sufficient to access the above methods.
- It is impossible, or at least difficult, to construct a table handle except using the autogenerated method on a database handle.
  - Similar to other resource-handle types, like `std::fs::File` in Rust, where users are discouraged from circumventing the normal path to acquire the resource.

Notably, an implementor may decide that a table handle should be a `Copy` type with a lifetime parameter bounding it to the database handle, rather than being a reference. This choice should not meaningfully affect user code, as users are not expected to ever refer to the type of a table handle by name.

Implementors are encouraged to generate doc comments describing the trait and its methods, and on the `TableHandle` type discouraging users from naming it.

#### In SDKs

##### Table codegen interface

This section is non-normative. The normative definition of the generated interface for registering row callbacks is in [04: Table and Type Definitions](./04-table-and-type-definitions.md#client-row-callbacks).

For each type with at least one `#[spacetimedb::table]` annotation in the module, the CLI generates a file containing a type definition. This is *just* a type definition, the way the CLI would previously generate for a `#[derive(SpacetimeType)] struct` definition in the module.

For each `#[spacetimedb::table(name = foo)]` annotation in the module (potentially multiple for a single type), the CLI generates a file containing a `TableHandle` struct with the same methods as the module-side macro emits (excluding those which mutate the DB, since the client cache is read-only), an extension trait `FooTableAccess` with a method `foo` which returns said `TableHandle`, and an implementation of `FooTableAccess` for `RemoteTables`. The `TableHandle` additionally has methods for managing row callbacks, which accept an `EventContext` (described above):

###### Rust

```rust
impl TableHandle {
    // Generated name of `CallbackId` types may vary. See 04: Table and Type Definitions.
    pub fn on_insert(&self, callback: impl FnMut(&super::EventContext, Row)) -> MyInsertCallbackId;

    pub fn on_delete(&self, callback: impl FnMut(&super::EventContext, Row)) -> MyDeleteCallbackId;

    pub fn on_update(&self, callback: impl FnMut(&super::EventContext, Row, Row)) -> MyUpdateCallbackId;

    pub fn remove_on_insert(&self, callback: FooInsertCallbackId);
    pub fn remove_on_delete(&self, callback: FooDeleteCallbackId);
    pub fn remove_on_update(&self, callback: FooUpdateCallbackId);
}
```

###### C#

```csharp
namespace Project {
    using SpacetimeDB;

    namespace TableHandles {
        public struct MyTable1 {
            public event Action<EventContext, Project.MyTable1> OnInsert;
            public event Action<EventContext, Project.MyTable1> OnDelete;
            public event Action<EventContext, Project.MyTable1, Project.MyTable1> OnUpdate;
        }
    }
}
```

##### Notes on implementation

The table name is automatically converted to the SDK language's convention for method names: snake case in Rust, pascal case in C#, and camel case in TypeScript.

##### Example - row callbacks

For example, to define a callback when a row of the `player` table is inserted, a user of the Rust SDK might write:

###### Rust

```rust
mod module_bindings;
use module_bindings::PlayerTableAccess;

fn register_player_on_insert(db: &module_bindings::RemoteTables) {
    db.player().on_insert(|ctx, player| {
        println!("Player {player:?} created during event {:?}", ctx.event);
    });
}
```

###### C#

```csharp
class MyProject {
    public void RegisterPlayerOnInsert(ModuleBindings.RemoteTables db) {
        db.Player().OnInsert += (ctx, player) =>
          Console.Out.WriteLine("Player {player} created during event {ctx.Event}");
    }
}
```

##### Reducer codegen interface

For each `#[spacetimedb::reducer] fn foo(Args...)` reducer defined in the module, the CLI generates a file or namespace whose name is the concatenation of the reducer name with `_reducer` (case-converted as appropriate), containing an extension trait or method `foo`. The extension trait is re-exported from the root of the `module_bindings` namespace.

The extension trait is implemented for `RemoteReducers`. The trait has methods for invoking the reducer and managing reducer callbacks:

###### Rust

```rust
#[allow(non_camel_case_types)]
pub trait foo {
    /// Request that the reducer be invoked.
    ///
    /// Returns an error if the connection is not active.
    fn foo(&self, Args...) -> Result<()>;

    /// Register a callback to run when this client is notified that the reducer ran.
    ///
    /// The callback accepts:
    /// - A `DbConnection` for accessing the connection.
    /// - The caller's `Identity`.
    /// - The caller's `Address`.
    /// - The status of the reducer, one of `Committed`, `Failed(String)` or `OutOfEnergy`.
    /// - The arguments to the reducer, as references.
    ///
    /// Multiple callbacks on the same reducer can coexist. Invocation order is undefined.
    //
    // Generated name of the `CallbackId` return type may vary.
    fn on_foo(&self, callback: impl FnMut(&super::EventContext, ArgRefs...)) -> FooCallbackId;

    /// Unregister an `on_foo` callback.
    ///
    /// May panic if `callback` is not a registered callback.
    fn remove_on_foo(&self, callback: FooCallbackId);
}
```

###### C#

```csharp
namespace Project {
    using SpacetimeDB;

    public class RemoteReducers {
        public void Foo(uint id, string name) {}

        public event Action<EventContext, uint, string> OnFoo;
    }
}
```

A note on C# implementations: there exist first-class events implementing the observer pattern which covers the
equivalent to `on_foo`/`remove_on_foo` pair in Rust. However, only extension /methods/ can be added to types in C#, and events are their own kind of member (fields and properties being the other 2, for a total of 4). As a solution, these are generated into concrete types, which are either subclassing an abstract type, or instantiating one using generics and themselves as the parameter. Or, both `class Foo : BaseFoo` and `class Foo; Bar<Foo>` are valid ways to extend sdk types with project-specific members and have these members be fields, properties or events. Another potential solution is partial classes, which is a mechanism to split a type definition into multiple parts joined together as the assembly is compiled, however it doesn't support splitting types across libraries this way, which prevents sdk types from sharing a partial implementation with project-specific types, user-defined or generated.

TL;DR: There's no traits in C#, extension classes are limited to methods, partials don't cross lib boundaries, and this leaves generics and subclassing for the extension of SDK types in project-specific libraries.

###### TypeScript

TODO: TypeScript interface.

##### Example - reducer callbacks

For example, to define a callback for a reducer `move(unit_name: String, x: i32, y: i32)`, SDK users might write:

###### Rust

```rust
mod module_bindings;
use module_bindings::on_foo;

fn register_on_foo(reducers: &module_bindings::RemoteReducers) {
    reducers.on_move(|ctx, unit_name, x, y| {
        println!("Notified of call `move({unit_name:?}, {x}, {y})`");
    });
}
```

###### C#

```csharp
class MyProject {
    public void RegisterOnFoo(RemoteReducers reducers) {
        reducers.OnMove += (ctx, unitName, x, y) =>
            Console.Out.WriteLine($"Notified of call `Move({unit_name}, {x}, {y})`");
    }
}
```

##### Notes on implementation

As above, implementors are encouraged to emit doc comments.

### Fully-applied `DbContext` alias, subtrait or sub-interface

As described in [Notes on cross-language implementation: Generic types and interfaces](#generic-types-and-interfaces), we define fully-applied aliases, subtraits, sub-interfaces or subclasses in both modules and clients to alleviate the pain of writing many generic parameters.

#### In modules

The module library defines a trait or interface `LocalDbContext` with no generic parameters or associated types, which inherits from or extents `DbContext<DbView = Local>`. `LocalDbContext` is implemented for `ReducerContext`.

##### Rust

```rust
pub trait LocalDbContext: DbContext<DbView = Local> {}
impl LocalDbContext for ReducerContext {}
```

##### C#

TODO: check this

```csharp
namespace SpacetimeDB {
    public abstract class LocalDbContext: DbContext<Local> {}

    public class ReducerContext: LocalDbContext {
        // implementations
    }
}
```

#### In clients

The `module_bindings` module-specific codegen defines a trait or interface `RemoteDbContext` with no generic parameters or associated types, which inherits from or extents `DbContext<DbView = RemoteTables, Reducers = RemoteReducers>`. Any other generic parameters or associated types on `DbContext` are also constraint to the module-specific generated concrete types. `RemoteDbContext` is implemented for `DbConnection` and `EventContext`.

##### Rust

```rust
pub trait RemoteDbContext: spacetimedb_sdk::DbContext<DbView = RemoteTables, Reducers = RemoteReducers> {}
impl RemoteDbContext for DbConnection {}
impl RemoteDbContext for EventContext {}
```

##### C#

TODO: check this

```csharp
namespace ModuleBindings {
    public abstract class RemoteDbContext: SpacetimeDB.DbContext<RemoteTables, RemoteReducers> {}

    public class DbConnection: RemoteDbContext {
        // implementations
    }

    public class EventContext: RemoteDbContext {
        // implementations
    }
}
```

##### TypeScript

TODO

### Module interfaces which do not require a `DbContext`

The following module bindings interfaces do not require a `DbContext`, i.e. remain free functions:

- `TimeSpan`-related methods, which are intended only for diagnostic use and do not read or write any interesting state.
- Logging via `println!`, `log::*` macros, and anything else that devolves to the `console_log` host call, for similar reasons.

## Implementation stages

- Rust module implementation.
- Rust SDK implementation.
- C# module implementation, with review to ensure consistency with the above.
- C# SDK implementation, with review to ensure consistency with the above.
- TypeScript SDK implementation, with review to ensure consistency with the above.
- Update quickstarts and reference documents.
- Update all example and test modules.
- Update BitCraft module.
- Update BitCraft client.

## Future Work

### SDK and Module API Stability

Soon!

### Language-specific idiomatic APIs in SDKs and modules

Future work may define more idiomatic language-specific APIs for various parts of the above interface where users experience friction. Largely the needs will reveal themselves over time as users complain, but we notably expect it to be desirable to offer an async alternative to `DbConnectionBuilder::build`. These interfaces will necessarily vary between languages more than the ones proposed here; future design work will determine what level of varience is acceptable.

### Inter-Module Communication

Future work will enable one module to connect, subscribe to, and request reducers run in other modules. How exactly the user-facing API for this looks, and how it interacts with `DbContext`s, remains undetermined.

### Partitioning

A rough sketch from a discussion with Tyler:

- Write one WASM program which orchestrates many database-modules.
- Mark tables as partitionable with a `#[partition_key]` column.
- Reducers may be declared to run within a partition.
- Partitioned reducers get a `PartitionContext` instead of a `ReducerContext` or something.
- Logically, partitioned tables are stored in and partitioned reducers run in a separate module, which must explicitly subscribe to and communicate with the non-partition parent module.
- Open questions:
  - How to clean up a parition when you're done with it? E.g. "this game has ended, so we don't need the room for it any more."
