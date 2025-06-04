# Module and SDK API: Built-in Reducers

SpacetimeDB defines several built-in reducers which the host automatically invokes in response to certain events. Implementors are encouraged to ensure these built-in reducers can only be invoked by the host at the appropriate time, i.e. that it is not possible for a client to invoke them via the WebSocket API or the `/database/call POST` HTTP route.

Built-in reducers largely follow the syntax and semantics described in [05: User-Defined Reducers](./05-user-defined-reducers.md), with a few exceptions.

## Names

Client codegen respects the names supplied in the function definition for built-in reducers.

For example, a module containing:

```rust
#[spacetimedb::reducer(client_connected)]
fn blurgh(ctx: &ReducerContext) { todo!() }
```

Defines a reducer named `blurgh` with the properties of a client-connected reducer, and codegen results in a `Blurgh` `Reducer` variant and `on_blurgh` callback.

This may require `ModuleDef` to track additional metadata about reducers, possibly in the form of a `builtin: Option<BuiltinReducer>` field in `ReducerDef`, where `BuiltinReducer` is `enum { ClientConnected | ClientDisconnected | Init | ... }`.

## Duplicate definitions

It is an error for a module to designate multiple functions to implement the same built-in reducer.

For example, the following module should error at compile-time or publish-time:

```rust
#[spacetimedb::reducer(init)]
fn init_a(ctx: &ReducerContext) { todo!() }
#[spacetimedb::reducer(init)]
fn init_b(ctx: &ReducerContext) { todo!() }
```

## Error returns

It is currently not meaningful for a built-in reducer to return an error, with the questionable exception of `init`. Internally, we must handle the case where a built-in reducer traps or otherwise fails, but implementors are encouraged to cause an error at macro-expand time if a built-in reducer is declared as returning or throwing an error. In Rust, built-in reducers must have declared return type unit (including the no-declared-return-type syntactic sugar).

Future work may specify useful behavior for error returns in particular built-in reducers. In particular, we consider the possibility of an error return or trap in `client_connected` resulting in the client being forcibly disconnected. When such behavior is specified, the built-in reducer in question will be permitted to declare an error return.

TODO: Describe `migrate`, which can reasonably return an error.

## Client codegen

Because clients are not intended to invoke built-in reducers directly, `RemoteReducers` methods for invoking them are not emitted. `RemoteReducers` methods for registering callbacks on them are still emitted, with the exception of `init`. This is because no client can ever be connected to a module in time to observe its `init` reducer running.

The codegen for built-in does not occur if the module does not define those built-in reducers.

## Arguments

As of specification time, built-in reducers may not accept any arguments other than the mandatory `&ReducerContext`. Future work may specify interfaces for providing arguments to particular built-in reducers. In particular, we consider the possibility of `init` accepting arguments supplied by the `spacetime publish` CLI command. When such behavior is specified, the built-in reducer in question will be permitted to declare arguments.

## Current built-in reducers

### `init`

`#[spacetimedb::reducer(init)]` annotates a function or method which runs when the module is initially published (i.e. with `-c` or without an existing database). `init`'s execution is atomic with the creation of the module/database, so a trap or exception in `init` causes the publish to be aborted. This currently almost certainly represents a bug, but future work adding arguments to `init` may make failing in `init` a meaningful intended behavior.

The sender identity passed to the `init` reducer is the identity which published, and therefore owns, the module.

As described above, client codegen is not necessary for `init` because it must strictly run before any client connects.

TODO: identity and address updates for Tyler's proposal.

### `client_connected`

`#[spacetimedb::reducer(client_connected)]` annotates a function or method which runs when a client connects to the module. Currently this is invoked at the start of a WebSocket connection and prior to a `/database/call POST` HTTP call.

The sender identity and address passed to the `client_connected` reducer are those of the client which is newly connected.

TODO: identity and address updates for Tyler's proposal.

### `client_disconnected`

`#[spacetimedb::reducer(client_disconnected)]` annotates a function or method which runs when a client ends its connection to the module. Currently this is invoked after a WebSocket connection closes and after a `/database/call POST` HTTP call.

The sender identity and address passed to the `client_disconnected` reducer are those of the client which has just disconnected.

TODO: identity and address updates for Tyler's proposal.

### `migrate`

TODO: This is partially designed but not yet implemented.
