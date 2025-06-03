# Module and SDK API: Credential Management

TODO: After finalizing Tyler's Identity Proposal, this will significantly change. `Address` will be removed, possibly replaced with a session or connection ID for clients. Bring-your-own-auth will make persistent storage of tokens not meaningful.

## Accessing the current actor's `Identity` and `Address`

Each SpacetimeDB actor (module or client) has an `Identity` and an `Address`, which together form a unique identifier for the actor. It is often useful to access these. E.g. a module may gate certain reducer invocations so that only clients with the same `Identity` can invoke that reducer; a client may include its `Identity` and/or `Address` in its subscription queries to filter for rows related to its connection.

`DbContext` and its implementors have methods `identity` and `address` which return the current actor's `Identity` and `Address`, respectively, in both modules and clients.

Where meaningful, these may be accompanied by `try_identity` and/or `try_address` methods which return `Option` at the implementor's discretion. If `try_` methods are provided, documentation on both methods should reflect the circumstances in which `try_` will return `None` and what the non-`try` variant does in that case. In practice, in Rust clients, `ctx.identity()` may fail if the connection was constructed anonymously and the `IdentityToken` message has not yet been received from the server.

```rust
pub trait DbContext {
    // other members omitted

    fn identity(&self) -> Identity;
    fn address(&self) -> Address;
}
```

No method(s) are provided to access a client's access token. Clients which wish to store an access token should do so in the `DbConnection::on_connect` callback. This is because, unlike the `Identity` and `Address`, the token is not useful during the lifetime of a connection, only when constructing one.

## In clients: persistent credential storage

Previously, the SDKs provided APIs for persistent storage of access tokens. The proposers believe that our shift in focus to pluggable and bring-your-own auth for end users negates the usefulness of such APIs, as we will no longer expect the client or end user to have a long-lived token which should be stored for re-use. Instead, clients will likely acquire a short-lived or session token on client startup by authenticating with some external service, possibly an OpenID Connect provider.

Future work may add interfaces to the SDK libraries for automating or abstracting over common auth flows with known OpenID Connect providers.
