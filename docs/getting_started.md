# Getting Started with the SpacetimeDB C++ Module SDK

This guide will walk you through the basics of creating a SpacetimeDB module using the C++ SDK. You'll learn how to define your schema, implement reducers, and build your module into a WebAssembly (Wasm) file.

## Prerequisites

*   A C++ compiler that supports C++17 (e.g., Clang, GCC).
*   Emscripten SDK: For compiling C++ to WebAssembly. Ensure `emcc` is in your PATH.
*   CMake: For building the example module (and potentially your own).
*   The SpacetimeDB C++ Module SDK files.

## 1. Project Setup

1.  **Include SDK Headers**:
    Ensure your project can find the SpacetimeDB C++ SDK header files. These include:
    *   `spacetime_macros.h`: For schema definition.
    *   `spacetime_schema.h`: Internals for schema registration.
    *   `spacetime_sdk_runtime.h`: For SDK runtime functions like logging and table operations.
    *   `bsatn_reader.h` & `bsatn_writer.h`: For BSATN (de)serialization.
    *   `spacetime_module_abi.h`: Declares functions your Wasm module must export.
    *   `spacetime_host_abi.h`: Declares functions your module imports from the SpacetimeDB host.

2.  **Link SDK Source Files (or Library)**:
    Your module will need to be compiled and linked with the SDK's C++ implementation files (e.g., `bsatn_reader.cpp`, `spacetime_sdk_runtime.cpp`, etc.). For an example, see the `CMakeLists.txt` in the `examples/cpp_counter_module`. In a typical setup, you might compile the SDK into a static library.

## 2. Define Your Schema

SpacetimeDB modules use macros to define their schema (data types, tables, reducers).

### Defining a Struct

Use `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS` to define a C++ struct and register it with SpacetimeDB. This macro also generates BSATN (de)serialization functions.

```cpp
// my_module.cpp
#include "spacetime_macros.h"
#include "bsatn_reader.h" // Required by generated deserialize
#include "bsatn_writer.h" // Required by generated serialize

// Define your C++ struct
struct Player {
    uint64_t player_id;
    std::string username;
    std::optional<uint32_t> score;
};

// Create an X-Macro listing fields for (de)serialization
// XX(CPP_TYPE_OR_ELEMENT_TYPE, FIELD_NAME, IS_OPTIONAL, IS_VECTOR)
#define PLAYER_FIELDS(XX) \
    XX(uint64_t, player_id, false, false) \
    XX(std::string, username, false, false) \
    XX(uint32_t, score, true, false) // For std::optional<uint32_t>, CPP_TYPE is uint32_t

// Register the struct and generate BSATN functions
SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(Player, "PlayerDb", PLAYER_FIELDS, {
    SPACETIMEDB_FIELD("player_id", SpacetimeDb::CoreType::U64),
    SPACETIMEDB_FIELD("username", SpacetimeDb::CoreType::String),
    SPACETIMEDB_FIELD_OPTIONAL("score", SpacetimeDb::CoreType::U32)
});
```
*   The C++ struct `Player` must be defined manually.
*   `PLAYER_FIELDS` is an X-Macro that you define. For each field, it calls the macro argument `XX` with the field's C++ type (or element type for optionals/vectors), name, and boolean flags for optional/vector.
*   The last argument to `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS` is an initializer list for schema registration, using helpers like `SPACETIMEDB_FIELD`.

### Defining an Enum

Use `SPACETIMEDB_TYPE_ENUM` for `enum class` types. The enum **must** have `uint8_t` as its underlying type.

```cpp
#include "spacetime_macros.h" // (if not already included)

enum class PlayerStatus : uint8_t {
    Offline = 0,
    Online = 1,
    InGame = 2
};

SPACETIMEDB_TYPE_ENUM(PlayerStatus, "PlayerStatusDb", {
    SPACETIMEDB_ENUM_VARIANT("Offline"),
    SPACETIMEDB_ENUM_VARIANT("Online"),
    SPACETIMEDB_ENUM_VARIANT("InGame")
});
```

### Defining a Table

Use `SPACETIMEDB_TABLE` and `SPACETIMEDB_PRIMARY_KEY`.

```cpp
// (Continuing my_module.cpp)

// Define a table for Player structs, named "Players" in SpacetimeDB
SPACETIMEDB_TABLE(Player, "Players");

// Set "player_id" as the primary key for the "Players" table
SPACETIMEDB_PRIMARY_KEY("Players", "player_id");
```

## 3. Implement Reducers

Reducers are functions that modify your application's state.

```cpp
// (Continuing my_module.cpp)
#include "spacetime_sdk_runtime.h" // For SpacetimeDB::log_info etc.
#include <string> // For std::string

// Reducer implementation
void create_player_reducer_impl(std::string username, uint32_t initial_score) {
    SpacetimeDB::log_info("Reducer 'CreatePlayer' called for user: " + username);

    Player new_player;
    new_player.player_id = 1; // In a real app, generate a unique ID (e.g., from a host function)
    new_player.username = username;
    new_player.score = initial_score;

    // Use SDK runtime functions to interact with tables
    bool inserted = SpacetimeDB::table_insert("Players", new_player);
    if (inserted) {
        SpacetimeDB::log_info("New player inserted successfully.");
    } else {
        SpacetimeDB::log_error("Failed to insert new player.");
    }
}

// Register the reducer
SPACETIMEDB_REDUCER("CreatePlayer", create_player_reducer_impl, {
    SPACETIMEDB_REDUCER_PARAM("username", SpacetimeDb::CoreType::String),
    SPACETIMEDB_REDUCER_PARAM("initial_score", SpacetimeDb::CoreType::U32)
}, std::string, uint32_t); // C++ types of parameters, in order
```
*   The reducer function (`create_player_reducer_impl`) contains your game logic.
*   `SPACETIMEDB_REDUCER` registers this function.
    *   The first argument is the SpacetimeDB name of the reducer.
    *   The second is the C++ function name.
    *   The third is an initializer list for schema registration of parameters.
    *   The variadic arguments at the end are the C++ types of the function's parameters, in order. This is crucial for generating the correct invoker.

## 4. Building Your Wasm Module

You'll need to compile all your C++ source files (your module code and the SDK `.cpp` files) into a single Wasm file. Here's a conceptual `CMakeLists.txt` snippet using Emscripten:

```cmake
cmake_minimum_required(VERSION 3.13)
project(MySpacetimeModule LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# --- Paths to SpacetimeDB C++ SDK ---
# Adjust these to your SDK location
set(SPACETIMEDB_SDK_INCLUDE_DIR "/path/to/sdk/cpp/include")
set(SPACETIMEDB_SDK_SRC_DIR "/path/to/sdk/cpp/src") # Assuming SDK sources are compiled directly

# --- SDK Source Files ---
# In a real setup, this would be a pre-built library.
file(GLOB SDK_SOURCES "${SPACETIMEDB_SDK_SRC_DIR}/*.cpp")

# --- Your Module Sources ---
add_executable(my_module_wasm
    my_module.cpp
    ${SDK_SOURCES}
)

target_include_directories(my_module_wasm PUBLIC "${SPACETIMEDB_SDK_INCLUDE_DIR}")
set_target_properties(my_module_wasm PROPERTIES SUFFIX ".wasm")

# --- Emscripten Linker Flags for Exports ---
# The SDK uses EMSCRIPTEN_KEEPALIVE on ABI functions, so explicit export
# via EXPORTED_FUNCTIONS might not be needed if EMSCRIPTEN_KEEPALIVE is active.
# However, being explicit can be safer.
# target_link_options(my_module_wasm PRIVATE
#    "-sEXPORTED_FUNCTIONS=['_get_spacetimedb_module_def_data','_get_spacetimedb_module_def_size','__spacetimedb_dispatch_reducer']"
# )
```
*   **Toolchain**: You must use the Emscripten toolchain file with CMake:
    `cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/emscripten/cmake/Modules/Platform/Emscripten.cmake .`
*   **ABI Exports**: The SpacetimeDB runtime needs certain functions exported from your Wasm module:
    *   `get_spacetimedb_module_def_data()`
    *   `get_spacetimedb_module_def_size()`
    *   `_spacetimedb_dispatch_reducer()`
    The SDK headers (`spacetime_module_abi.h` and the definition of `_spacetimedb_dispatch_reducer`) use `EMSCRIPTEN_KEEPALIVE` (when `__EMSCRIPTEN__` is defined) to ensure these are exported. If this method isn't used or you want to be explicit, use the `-sEXPORTED_FUNCTIONS` linker flag with Emscripten, making sure to include the leading underscores for C function names if necessary (e.g., `_get_...`, `__spacetimedb_dispatch_reducer`).

## 5. Next Steps

*   Consult `api_macros.md` and `api_runtime.md` for detailed API documentation.
*   Explore the example module in `examples/cpp_counter_module` for a practical demonstration.
*   Implement your game logic within reducers.
*   Use the SDK runtime functions to interact with SpacetimeDB (logging, table operations).

Welcome to building with SpacetimeDB and C++!
```
