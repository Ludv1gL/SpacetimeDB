# Built-in Reducers in SpacetimeDB C++ SDK

This document describes the implementation of built-in lifecycle reducers in the SpacetimeDB C++ SDK.

## Overview

SpacetimeDB supports three built-in lifecycle reducers that are automatically called by the runtime:

1. **`init`** - Called when the module is first initialized
2. **`client_connected`** - Called when a client connects to the database
3. **`client_disconnected`** - Called when a client disconnects from the database

## Implementation Details

### Files Added/Modified

1. **`sdk/include/spacetimedb/builtin_reducers.h`** - New header providing:
   - `Identity` type for representing client identities
   - `Lifecycle` enum for reducer types
   - Macros for declaring built-in reducers
   - Helper functions for lifecycle detection

2. **`sdk/include/spacetimedb/spacetimedb.h`** - Modified to:
   - Include `builtin_reducers.h`
   - Add lifecycle field to `ModuleDef::Reducer`
   - Add specialized registration functions for each built-in reducer type
   - Update module description writing to include lifecycle information
   - Pass sender identity from `__call_reducer__` to lifecycle reducers

### Key Components

#### Identity Type

```cpp
class Identity {
    static constexpr size_t SIZE = 32;
    using ByteArray = std::array<uint8_t, SIZE>;
    
    // Constructors
    Identity();
    Identity(const ByteArray& bytes);
    Identity(uint64_t part0, uint64_t part1, uint64_t part2, uint64_t part3);
    
    // Methods
    const ByteArray& to_byte_array() const;
    std::string to_hex_string() const;
};
```

The Identity is a 32-byte value that uniquely identifies a client. It's constructed from the 4 uint64_t parts passed to `__call_reducer__`.

#### Lifecycle Enum

```cpp
enum class Lifecycle : uint8_t {
    Init = 0,
    OnConnect = 1,
    OnDisconnect = 2
};
```

This matches the Rust SDK's lifecycle enum values.

#### Registration Functions

Three specialized registration functions handle built-in reducers:

```cpp
void register_init_reducer(void (*func)(ReducerContext));
void register_client_connected_reducer(void (*func)(ReducerContext, Identity));
void register_client_disconnected_reducer(void (*func)(ReducerContext, Identity));
```

## Usage

### Declaring Built-in Reducers

Use the provided macros to declare built-in reducers:

```cpp
// Init reducer - called on module initialization
SPACETIMEDB_INIT(on_init) {
    // Your initialization code here
    LOG_INFO("Module initialized!");
}

// Client connected reducer
SPACETIMEDB_CLIENT_CONNECTED(on_connect) {
    LOG_INFO("Client connected: " + sender.to_hex_string());
    // Track the connection
}

// Client disconnected reducer
SPACETIMEDB_CLIENT_DISCONNECTED(on_disconnect) {
    LOG_INFO("Client disconnected: " + sender.to_hex_string());
    // Clean up after the client
}
```

### Example: Connection Tracking

See `examples/simple_module/builtin_reducers_example.cpp` for a complete example that:
- Tracks client connections and disconnections
- Logs module initialization
- Provides queries for connection status

### Example: Simple Test

See `examples/simple_module/test_builtin_reducers.cpp` for a minimal example that:
- Records all lifecycle events in a simple table
- Works without string types (avoiding current encoding issues)

## Technical Implementation

### Sender Identity Passing

The sender identity is passed from `__call_reducer__` to the reducer via thread-local storage:

```cpp
thread_local uint64_t g_sender_parts[4];
```

This allows the lifecycle reducer handlers to access the sender identity without modifying the standard reducer calling convention.

### Module Description

Built-in reducers are marked in the module description with their lifecycle type:

```cpp
// In module description writing
if (reducer.lifecycle.has_value()) {
    w.push_back(0);  // Option::Some
    w.push_back(static_cast<uint8_t>(reducer.lifecycle.value()));
} else {
    w.push_back(1);  // Option::None
}
```

## Current Limitations

1. **Timestamp Access**: The current `ReducerContext` doesn't expose timestamp information, so examples use placeholder values
2. **Connection ID**: Not currently exposed to reducers (available in `__call_reducer__` but not passed through)
3. **String Encoding**: The string encoding issue affects examples using string fields

## Future Enhancements

1. Add timestamp and connection_id to ReducerContext
2. Add helper methods for Identity parsing/formatting
3. Support for additional lifecycle events if added to SpacetimeDB
4. Better integration with error handling for lifecycle reducers