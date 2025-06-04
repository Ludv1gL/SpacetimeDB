# Module and SDK API: Module RNG

This file amends [0006: Module RNG](../0006-module-rng.md) to suit [01: `DbConnection`](./01-db-connection.md).

In modules, the `DbContext` trait has a method `rng` which returns a PRNG seeded by the reducer's timestamp. In languages whose build systems support conditional compilation via features, access to this method is controlled by a feature `rand`, which is enabled by default but may be explicitly disabled.

Internally, the PRNG type returned by the `rng` method should be implemented as a handle on or reference to a per-reducer or global PRNG state which is seeded either lazily on first access or eagerly on entering a reducer. Accesses to the global PRNG state should be synchronized internally so that multiple handles or references may coexist and make progress.

Using a global per-reducer PRNG rather than constructing a new one on each call to `ctx.rng()` is important so that two separate calls to `ctx.rng()` within a single reducer will not yield the same series of values.

## Rust

```rust
pub trait DbContext {
    // Other methods elided.

    #[cfg(feature = "rand")]
    fn rng(&self) -> impl rand::Rng;
}
```

## C#

```cs
public class DbContext
{
    /// <summary>
    /// Returns a random number generator that's seeded with current reducer's Timestamp. Multiple calls from the same reducer will return the same instance (ie. calling `DbContext.Rng.Next()` twice will (likely) return different values)
    /// </summary>
    public System.Random Rng { get; }

    //...
}
```