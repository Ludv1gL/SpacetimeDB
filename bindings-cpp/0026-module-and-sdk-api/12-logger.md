# Module and SDK API: Logger

C# and Rust module SDKs currently have very different APIs for logging. Rust uses an external package that provides a lot of convenient functions like `log::info!`, while C# only exposes a single method `Runtime.Log`, which is essentially 1-1 match with ABI, requiring users to manually specify filenames and line numbers of where that log originated from. This section aims to bring C#'s SDK closer in line with what rust has.

## Logger class

C# doesn't have a "standard" way of logging things. There are `System.Console` and `System.Diagnostics.Debugger` classes, but those, while possible to redirect, have different purposes. Instead of trying to reuse an existing logger type, the best way forward is to define our own API similar to [what Unity did with their Debug class](https://docs.unity3d.com/ScriptReference/Debug.html). There's not much a logger needs to do - at its core it's just a wrapper around ABI that retreives filename and line number. Having different methods for different log levels is also fairly standard in C# and would make logger more convenient to use.

Logger class would look something like this:

```cs
namespace SpacetimeDB;

public static class Log
{
    public static void Debug(string message); //LogLevel.Debug
    public static void Trace(string message); //LogLevel.Trace
    public static void Info(string message); //LogLevel.Info
    public static void Warn(string message); //LogLevel.Warn
    public static void Error(string message); //LogLevel.Error
    public static void Exception(string message); //LogLevel.Panic
    public static void Exception(Excepotion e); //LogLevel.Panic
}
```

Logging methods simply retreive caller source and pass it, along with log level, to `FFI._console_log`. Exception override that accepts an `Exception` object simply appends exception text and call stack to optional user message.

### Source code info
One of current C# WASM limitations is absence of debug symbols. This means that stacktraces (ex. from exceptions) do not have file names and line numbers. To get around this, `Runtime.Log` method currently has parameters with attribures like `[CallerFilePath]`. These attributes are replaced with string literals at `C# --> IL` compile time, and are thusly available in WASM. As such, `Runtime.Log` method currently looks like this
```cs
public static void Log(
    string text,
    [CallerMemberName] string target = "",
    [CallerFilePath] string filename = "",
    [CallerLineNumber] uint lineNumber = 0
) {...}
```
These parameters are currently a necessity and we can't get rid of them without potentially introducing a lot of issues. Therefore, module SDK will need to have these parameters on all publically exposed methods. We may be able to get rid of them later on, but for now we should make it clear that users are not expected to provide these parameters. To that end, I suggest our exposed methods have the following definition
```cs
/// <summary>
/// Write an error message to module log
/// </summary>
/// <param name="message">Message to log</param>
/// <param name="RESERVED_target"><b>!!! DO NOT USE !!!</b> Value for this parameter will be automatically generated at compile time. Providing this parameter could lead to undefined behavior</param>
/// <param name="RESERVED_filename"><b>!!! DO NOT USE !!!</b> Value for this parameter will be automatically generated at compile time. Providing this parameter could lead to undefined behavior</param>
/// <param name="RESERVED_lineNumber"><b>!!! DO NOT USE !!!</b> Value for this parameter will be automatically generated at compile time. Providing this parameter could lead to undefined behavior</param>
public static void Error(
    string message,
    [CallerMemberName] string RESERVED_target = "Do not pass - value will be automatically generated during compilation",
    [CallerFilePath] string RESERVED_filename = "Do not pass - value will be automatically generated during compilation",
    [CallerLineNumber] uint RESERVED_lineNumber = 0
) {...}
```

## Client SDK vs Module SDK logger
Client and module SDKs can have the same public API, but due to their nature will have different implementations. Client SDK methods will NOT have extra parameters that are required in WASM

## Alternatives considered

### Call site issues
In addition to issues associated with each of the alternatives below, all of them suffer from the same problem: lack of call site info in WASM. The only way to get that currently is to add specific parameters to the topmost method that gets called by user code, and since we can't change APIs of existing methods, using any of them would mean we'd lose access to log context.

### Redirect `System.Console` to module log
`System.Console` is the default way to write things in console applications, however it doesn't implement different levels of logging. `Console.WriteLine` is used for info, errors and everything in between, so it's not a good fit for our use case.

### Redirect `System.Diagnostics.Debugger` to module log
This one DOES have a parameter for log level, however it's not a very user-friendly class, and it's mostly used when interacting with external debugger.

### `ILoggerFactory`
This is a standard way of logging things in ASP.Net. It is extremely flexible with support of asynchronous logging to multiple targets. It is, however, not present in most non-ASP.Net projects and requires extra steps to set up. It is also instance-based, which means we'd have to inject a logger instance into each reducer (or ReducerContext), making it more cumbersome to use.

### Third-party logging packages
There are plenty of third-party logging solutions for C# out there (ex. NLog), however most of them are built on top of `ILoggerFactory`. The ones that don't, would still require us to somehow expose them to end user (you can't expose a namespace from external package in C# similar to how `spacetimedb::log` is set up in rust). That means that we'd either have to create a static user-facing class (essentially what I proposed above, but with extra steps), OR we'd have to include third-party package source code with module SDK, which creates even more problems (licensing, updates, etc).