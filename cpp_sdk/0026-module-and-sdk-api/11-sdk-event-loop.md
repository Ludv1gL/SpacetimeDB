# Module and SDK API: SDK Event Loop

Against everyone's best wishes, supporting Unity and similar game engines imposes the requirements:

- Certain calls, notably anything that interacts with the GUI or with game objects, happen only on the main thread.
- The SDK receive time slices on the main thread only at explicit, predictable times.

## Low-level methods

To this effect, `DbConnection` has methods which control when it updates:

```rust
impl DbConnection {
    /// Process up to one message, running its callbacks and updating the client cache.
    ///
    /// If a message is pending, process it and return `true`.
    ///
    /// If no message is ready, return `false` immediately.
    ///
    /// If the connection is not active, return an error.
    ///
    /// The consequences are undefined if multiple threads call `step` concurrently on the same `DbConnection`,
    /// or if a callback causes a recursive invocation of `step` on a `DbConnection`.
    pub fn advance_one_message(&self) -> Result<bool>;

    /// Like [`Self::step`], but blocks until a message is available.
    pub fn advance_one_message_blocking(&self) -> Result<()>;

    /// Like [`Self::step`], but awaits until a message is available.
    pub async fn advance_one_message_async(&self) -> Result<()>;
}
```

Callbacks exclusively run during calls to `advance_one_message` (or `advance_one_message_blocking` or `_async`), on the thread which called `advance_one_message`. The client cache is only updated during calls to `advance_one_message`. If a call to `advance_one_message` processes an event, the client cache is updated for that event before any callbacks are invoked, and all callbacks return before the call to `advance_one_message` returns.

SDK implementations must provide the `advance_one_message` method and at least one of `advance_one_message_blocking` or `advance_one_message_async`, as appropriate. SDK implementations are not required to provide both `advance_one_message_blocking` and `advance_one_message_async`.

Implementors are encouraged to document `advance_one_message` and its variants as being low-level primitives which most users will not encounter, and to direct them to the below higher-level operators.

## Common execution patterns

SDK implementations are encouraged to provide as many of the following operators as are meaningful. Sample implementations are provided for some of the listed methods, but these implementations are not normative; only the interface and semantics are normative.

### Clear the queue every frame

For contexts where it is desirable or necessary to run callbacks on the main thread, we provide a method `frame_tick` which calls `advance_one_message` in a loop until it returns `false`, thereby processing all pending messages. Users may set up their game loop to call this method once per frame.

```rust
impl DbConnection {
    pub fn frame_tick(&self) -> Result<()> {
        while self.advance_one_message()? {}
    }
}
```

### Object which calls `frame_tick`

In game engines which provide an interface for game objects with update methods that are automatically invoked per-frame, implementations are encouraged to provide a class which holds a `DbConnection` and calls `frame_tick` in the update method.

### Run in a background thread

For contexts where shared-memory preemptive multithreading (i.e. OS threading) is available, we provide a method `run_threaded` which calls `advance_one_message_blocking` in a loop continuously. This loop should recognize normal disconnects and terminate the thread normally, and otherwise panic if `advance_one_message_blocking` returns an error.

```rust
impl DbConnection {
    pub fn run_threaded(&self) -> std::thread::JoinHandle<()> {
        let this = Arc::clone(&self.imp);
        std::thread::spawn(move || {
           loop {
               match this.advance_one_message_blocking() {
                   Ok(()) => (),
                   Err(e) if error_is_normal_disconnect(&e) => return,
                   Err(e) => panic!(e),
               }
           }
        });
    }
}
```

### Run in an async loop

For contexts where an intelligent userspace `async` executor is available, we provide a method `run_async` which calls `advance_one_message_async` in a loop continuously. As above, this loop should recognize normal disconnects and exit the loop, and otherwise return any error from `advance_one_message_async`.

```rust
impl DbConnection {
    pub async fn run_async(&self) -> Result<()> {
        let this = Arc::clone(&self.imp); // So the future will be `Send`.
        loop {
            match this.advance_one_message_async().await {
                Ok(()) => (),
                Err(e) if error_is_normal_disconnect(&e) => return Ok(()),
                Err(e) => return Err(e),
            }
        }
    }
}
```

## Alternatives considered

### Be an event source rather than a callback manager

We could have a method `DbConnection::poll() -> Option<Event>`, where `Event` is a parsed-and-prettified version of the WebSocket `ServerMessage`, including row deltas, possibly alongside a materialized view of the client cache. Users would then inspect the contents of the `Event`, rather than registering callbacks in the `DbConnection` which run during `advance_one_message` calls. This would be equivalent to the current design internally, but users would "route" along the `Event` themselves via a `match` or `switch`, rather than us routing for them and invoking their callback. We prefer the callback version which removes routing boilerplate in the common case.

## Future work

### Single-threaded configuration

Expose conditional compilation or feature flags to disable any and all use of background threads to allow compilation for targets which do not support multithreading.
