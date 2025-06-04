# Module and SDK API: Module Timing Spans

It is often useful for module developers to determine the runtime of a span of code in order to determine where optimization efforts should be spent. However, a full interface for reading a system clock from modules is undesirable because it introduces nondeterminism into module execution, which breaks many desirable future behaviors, e.g. testing via log replay or retrying reducers on MVCC conflicts.

As such, we provide an interface to modules where they can construct a "console timer," which writes a duration to the module's logs. The duration measured by the timer is not accessible to the module's computation, and is only usable by module developers for diagnostic purposes.

The module bindings library defines a type `LogStopwatch`, exported from a `log_stopwatch` module, file or namespace within the module bindings crate or library. It is named for its similarity to C#'s [`System.Diagnostics.Stopwatch`](https://learn.microsoft.com/en-us/dotnet/api/system.diagnostics.stopwatch?view=net-8.0), except that `LogStopwatch` logs rather than returning time measurements.

`LogStopwatch` has a constructor or associated function `start` which accepts a string identifier, and a method `end` which writes the elapsed time since construction to the module logs in an unspecified format which includes the string identifier. In languages with destructors, dropping or destroying a `LogStopwatch` behaves the same as calling `end`.

It is valid to have two or more `LogStopwatch` instances running concurrently with the same string identifier. These must refer to distinct timers, and must track their start times and log their elapsed times separately.

Unlike most interfaces exposed to modules by the host, console timers are not tied to a `DbContext` or `ReducerContext`. They are available in any module code, even exported functions which do not run transactionally (e.g. prepared query or view constructors).

`LogStopwatch` instances are invalidated after the WASM function which constructed them ends. Calling `end` or the destructor on a `LogStopwatch` created by a past WASM entry and stashed across invocations invokes unspecified behavior, and may trap WASM execution, end an unrelated `LogStopwatch`, or print any unhelpful message to the module logs. Ending an invalid `LogStopwatch` in this way should not return or throw a handle-able error.

## Rust interface

```rust
/// ### In `bindings/src/lib.rs`:

pub mod log_stopwatch;

/// ### In `bindings/src/console_timer.rs`:

pub struct LogStopwatch {
    // private fields
}

impl LogStopwatch {
    pub fn start(name: &str) -> Self;

    pub fn end(self);
}

impl Drop for LogStopwatch {
    fn drop(&mut self) {
        // Same behavior as calling `self.end()`
    }
}
```

## C# interface

```cs
public class LogStopwatch : IDisposable
{
    /// <summary>
    /// Starts a new timed span
    /// </summary>
    /// <param name="name">Human-readable name that will be printed in module logs</param>
    public LogStopwatch(string name) { }

    /// <summary>
    /// Print elapsed time and stop the stopwatch
    /// </summary>
    /// <exception cref="ObjectDisposedException">Stopwatch was already stopped</exception>
    public void End() { }

    public void Dispose() { } //Calls Stop if it hasn't been previously called
}
```
