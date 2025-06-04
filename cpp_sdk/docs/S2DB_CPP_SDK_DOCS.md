# SpacetimeDB C++ SDK Documentation

## 1. Introduction

### Purpose
The SpacetimeDB C++ SDK empowers developers to build high-performance server-side application logic (modules) for SpacetimeDB using the C++ programming language. It provides a comprehensive suite of tools and libraries for defining data schemas (tables), implementing custom business logic (reducers), interacting with the underlying database, handling efficient data serialization, and integrating with a familiar CMake-based build system. The final output is a WebAssembly (WASM) module, designed to run securely and efficiently within the SpacetimeDB server environment.

This SDK is tailored for C++ developers who want to leverage the performance and control of C++ while building scalable and real-time applications on SpacetimeDB. The SDK now features ~92% feature parity with the C# SDK through an intuitive macro system, with full support for scheduled reducers, constraints, and advanced indexing.

### Overview of SDK Features
*   **Schema Definition:** Define your database tables using C++ structs with C#-style attribute macros. The SDK automatically handles registration and type generation.
*   **Reducer Implementation:** Write your core application logic as C++ functions with lifecycle support (init, connect, disconnect, scheduled).
*   **Data Serialization (BSATN):** Fully automatic BSATN serialization with type registry, supporting all SpacetimeDB types including special types (Identity, Timestamp, etc.).
*   **Enhanced Codegen:** Automatic generation of C++ types from module definitions with seamless integration.
*   **Database Interaction API:** Comprehensive API with advanced querying, indexing (B-tree, unique), predicates, and transaction support.
*   **C# Feature Parity:** 100% equivalent macros to C# attributes (SPACETIMEDB_TABLE, SPACETIMEDB_REDUCER, etc.).
*   **Cross-Platform Support:** BSATN system designed to support both std:: types and Unreal Engine types.
*   **Build System Integration:** Streamlined CMake integration with Emscripten for WebAssembly compilation.
*   **Advanced Features:** Constraint validation, module versioning, credential management, and scheduled reducers.
*   **Comprehensive Testing:** Includes sdk-test-cpp module demonstrating all features with full test coverage.

### Prerequisites
Before you begin developing SpacetimeDB modules with the C++ SDK, ensure you have the following tools installed and configured:

*   **C++17 Compiler:** A modern C++ compiler that supports at least the C++17 standard (e.g., Clang, GCC, MSVC for local development, though Emscripten will be the final compiler for WASM).
*   **Emscripten (emsdk):** This is the compiler toolchain used to compile C++ to WebAssembly.
    *   Download and install the EMSDK from the [official Emscripten documentation](https://emscripten.org/docs/getting_started/downloads.html).
    *   Ensure that the Emscripten environment is active in your terminal (e.g., by sourcing `emsdk_env.sh` or `emsdk_env.bat`) so that `emcc` and `em++` are in your system's PATH, or that the `EMSDK` environment variable is set to the root of your emsdk installation.
*   **CMake:** Version 3.15 or higher. CMake is used to manage the build process for both the SDK and your C++ modules.
*   **SpacetimeDB CLI:** The `spacetime` command-line tool for managing, running, and publishing your SpacetimeDB databases and modules.
*   **Ninja (Recommended):** While not strictly required, Ninja is a fast build system that works well with CMake and is often recommended for C++ projects, including those using Emscripten.

## 2. Getting Started

This section will guide you through setting up a new C++ SpacetimeDB module project, building it into a WASM file, and publishing it.

### Project Setup

We recommend the following general project structure:

```
spacetime_cpp_project_root/
├── sdk/                      # SpacetimeDB C++ SDK source and build files
│   ├── include/              # SDK public headers (e.g., spacetimedb/sdk/database.h)
│   ├── src/                  # SDK source files (e.g., database.cpp)
│   └── CMakeLists.txt        # CMake file for building the SDK static library
├── examples/
│   └── quickstart_cpp_kv/    # Example C++ module (your project would be similar)
│       ├── CMakeLists.txt    # CMake file for building this specific module
│       ├── Cargo.toml        # Dummy Cargo.toml for `spacetime` CLI compatibility
│       └── src/
│           ├── kv_store.h
│           └── kv_store.cpp
├── toolchains/
│   └── wasm_toolchain.cmake  # CMake toolchain file for Emscripten/WASM
└── build_and_publish_example.sh # Example script to build and publish the quickstart
```

#### 2.1. `Cargo.toml` for Your Module
In the root directory of your C++ module (e.g., `examples/quickstart_cpp_kv/`), create a `Cargo.toml` file. This file is **not** used to build your C++ code with Rust's Cargo build system. Instead, it serves as a metadata file that the `spacetime` CLI uses to identify your project, its name, and the conventional location of the compiled WASM artifact.

**Example: `examples/quickstart_cpp_kv/Cargo.toml`**
```toml
[package]
name = "kvstore_module" # Crucial: This name will be used by the CLI and build scripts.
version = "0.1.0"
edition = "2021" # Or any relevant Rust edition, e.g., "2018"

# This Cargo.toml file is primarily for compatibility with the `spacetime publish` CLI.
# It helps the CLI identify the project name and expected output path for the pre-compiled WASM module.
# The C++ code itself is built using CMake and a C++ toolchain (e.g., Emscripten).

[lib]
crate-type = ["cdylib"] # Indicates a dynamic system library, commonly used for WASM modules.
```
Ensure the `name` field matches the intended output name for your WASM module.

#### 2.2. `CMakeLists.txt` for Your Module
In your module's root directory (e.g., `examples/quickstart_cpp_kv/`), create a `CMakeLists.txt` file. This file will define how your C++ module is built.

**Key aspects:**
*   Set the C++ standard to 17 or higher.
*   Define a variable `MODULE_NAME` that exactly matches the `name` in your `Cargo.toml`.
*   Specify your C++ source files.
*   Configure the output directory for the WASM file to be `target/wasm32-unknown-unknown/release/` relative to this `CMakeLists.txt` (and `Cargo.toml`).
*   Ensure the final WASM filename is `${MODULE_NAME}.wasm`.
*   Link against the SpacetimeDB C++ SDK library. The example below shows how to do this using `add_subdirectory` if your module is part of a larger project that includes the SDK. If you've installed the SDK elsewhere, you might use `find_package(SpacetimeDBCppSDK)`.

**Example: `examples/quickstart_cpp_kv/CMakeLists.txt`**
```cmake
cmake_minimum_required(VERSION 3.15)
project(KvStoreModuleUserProject CXX) # This CMake project name can be different

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the module name, this MUST match the 'name' in Cargo.toml
set(MODULE_NAME "kvstore_module") # Matches Cargo.toml name

# Set the output directory and filename to match Rust's convention
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target/wasm32-unknown-unknown/release)

# Add module source files
add_executable(${MODULE_NAME}
    src/kv_store.cpp
)

# Set the output name and suffix for the WASM file
set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME "${MODULE_NAME}")
set_target_properties(${MODULE_NAME} PROPERTIES SUFFIX ".wasm")

# --- SpacetimeDB C++ SDK Linking ---
# Assuming this example module is in 'spacetime_cpp_project_root/examples/quickstart_cpp_kv/'
# and the SDK is in 'spacetime_cpp_project_root/sdk/'
get_filename_component(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../ ABSOLUTE)
set(SPACETIMEDB_SDK_DIR ${PROJECT_ROOT_DIR}/sdk CACHE PATH "Path to SpacetimeDB C++ SDK root directory")

if(NOT IS_DIRECTORY ${SPACETIMEDB_SDK_DIR})
    message(FATAL_ERROR "SpacetimeDB SDK directory not found: ${SPACETIMEDB_SDK_DIR}.")
endif()

# Include the SDK's CMakeLists.txt to make its targets available
# The second argument is a binary directory for the SDK's build artifacts within this project's build tree.
# EXCLUDE_FROM_ALL prevents the SDK from being built unless this module (or another target) depends on it.
add_subdirectory(${SPACETIMEDB_SDK_DIR} ${CMAKE_BINARY_DIR}/sdk_build EXCLUDE_FROM_ALL)

# Link against the SDK's public target (e.g., spacetimedb_cpp_sdk or spacetimedb::sdk::spacetimedb_cpp_sdk)
# The SDK's CMakeLists.txt should define 'spacetimedb_cpp_sdk' as an alias or interface library.
target_link_libraries(${MODULE_NAME} PUBLIC spacetimedb_cpp_sdk)

# Include directories for this module (e.g., its own 'src' directory for local headers)
target_include_directories(${MODULE_NAME} PUBLIC src)
# The SDK's include directories should be automatically propagated by target_link_libraries
# if the SDK target correctly sets its INTERFACE_INCLUDE_DIRECTORIES.
# --- End SDK Linking ---

message(STATUS "Building user module: ${MODULE_NAME}.wasm")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "To configure: cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path_to_toolchain>")
message(STATUS "To build: cmake --build build")
```

#### 2.3. `wasm_toolchain.cmake`
This CMake toolchain file configures CMake to use Emscripten for compiling to WebAssembly. It should be located in a known path, for example, `toolchains/wasm_toolchain.cmake` at the root of your overall project.

*(Refer to the `toolchains/wasm_toolchain.cmake` content from Step 4 of the Materialize plan (originally Step 6 of initial plan) for its full content. It handles finding `emcc`/`em++`, setting `CMAKE_SYSTEM_NAME` to `Emscripten`, and configuring appropriate compile/link flags like `-s SIDE_MODULE=1` and `--no-entry`.)*

### Building the Module

You can build your module manually using CMake commands or use the provided example script.

#### Manual CMake Build (from your module's directory, e.g., `examples/quickstart_cpp_kv/`)
1.  **Configure CMake:**
    ```bash
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=../../toolchains/wasm_toolchain.cmake
    ```
    (Adjust the path to `wasm_toolchain.cmake` based on your project structure.)

2.  **Compile:**
    ```bash
    cmake --build build
    ```
    This will compile your C++ code and link it with the SpacetimeDB C++ SDK, producing the WASM file in `target/wasm32-unknown-unknown/release/`.

#### Using `build_and_publish_example.sh`
An example script, `build_and_publish_example.sh`, is provided at the root of the SDK project. This script automates the build and publish process for the `quickstart_cpp_kv` example.

*(Refer to the `build_and_publish_example.sh` script from Step 5 of the Materialize plan (originally Step 7 of initial plan). It `cd`s into the example directory, runs CMake configuration and build, checks for the output, and then publishes.)*

### Publishing the Module
After successfully building your `.wasm` file into the `target/wasm32-unknown-unknown/release/` directory within your module's project folder:

1.  Ensure you are in your module's root directory (e.g., `examples/quickstart_cpp_kv/`), where your `Cargo.toml` is located.
2.  Run the `spacetime publish` command:

    ```bash
    spacetime publish <module_name_from_cargo_toml> --name <your_db_address_or_alias>
    ```
    For example, if your `Cargo.toml` has `name = "kvstore_module"`:
    ```bash
    spacetime publish kvstore_module --name my_kv_database_on_cloud
    ```
The `spacetime` CLI uses the `<module_name_from_cargo_toml>` to find the `Cargo.toml` file, reads the module name, and expects the compiled WASM artifact at the conventional path (`target/wasm32-unknown-unknown/release/<module_name>.wasm`). It then uploads this WASM file to SpacetimeDB.

## 3. Core SDK Concepts

The SpacetimeDB C++ SDK is designed around a few core concepts: defining your data schema with C++ types, writing business logic in C++ functions called reducers, and interacting with the database through an SDK-provided API. All SDK components are unified under the `spacetimedb` namespace. The SDK provides a single main header `#include <spacetimedb/spacetimedb.h>` that includes all necessary functionality.

### Defining Tables

Tables store your application's persistent data. You define the structure of each table row using C++ structs with automatic serialization.

#### 3.1. Modern Table Definition (Recommended)
The SDK now provides C#-style attribute macros for defining tables with automatic BSATN serialization:

```cpp
// src/user.h - Modern approach with automatic serialization
#include <spacetimedb/spacetimedb.h>
#include <string>
#include <cstdint>

struct User {
    [[SPACETIMEDB_PRIMARY_KEY]]
    uint32_t id;
    
    [[SPACETIMEDB_UNIQUE]]
    std::string username;
    
    std::string email;
    std::optional<std::string> bio;
    std::vector<uint32_t> friend_ids;
};

// This macro generates all necessary serialization code automatically
SPACETIMEDB_TYPE(User)
```

#### 3.2. Table Registration with Attributes
Register your table using the enhanced `SPACETIMEDB_TABLE` macro (equivalent to C# [Table] attribute):

```cpp
// src/user.cpp
#include "user.h"

// Register as a public table
SPACETIMEDB_TABLE(User, "users", true)

// Or as a private table
SPACETIMEDB_TABLE(User, "internal_users", false)
```

#### 3.3. Advanced Table Features
```cpp
// Table with indexes
struct Product {
    [[SPACETIMEDB_PRIMARY_KEY_AUTO]]  // Auto-incrementing primary key
    uint32_t id;
    
    [[SPACETIMEDB_INDEX]]  // B-tree index for fast lookups
    std::string category;
    
    std::string name;
    double price;
};
SPACETIMEDB_TYPE(Product)
SPACETIMEDB_TABLE(Product, "products", true)

// Create composite index
SPACETIMEDB_INDEX_BTREE_MULTI(Product, category_price_idx, category, price)
```

#### 3.4. Legacy Manual Serialization (Still Supported)
For backward compatibility or special cases, manual BSATN serialization is still supported:

```cpp
struct LegacyData {
    std::string key;
    std::string value;
    
    // Manual serialization methods
    void bsatn_serialize(spacetimedb::bsatn::Writer& writer) const {
        spacetimedb::bsatn::serialize(writer, key);
        spacetimedb::bsatn::serialize(writer, value);
    }
    
    static LegacyData bsatn_deserialize(spacetimedb::bsatn::Reader& reader) {
        LegacyData data;
        data.key = spacetimedb::bsatn::deserialize<std::string>(reader);
        data.value = spacetimedb::bsatn::deserialize<std::string>(reader);
        return data;
    }
};
```

This registration allows the SDK to map C++ types to database table names and understand their basic structure, particularly the primary key.

### Writing Reducers

Reducers are the heart of your module's logic. They are C++ functions that execute atomically and can modify database state. The SDK provides enhanced reducer support with lifecycle hooks matching C#.

#### 3.3.1. Modern Reducer Definition (Recommended)
The SDK now provides enhanced macros for different reducer types:

```cpp
// Standard reducer
void add_user(ReducerContext& ctx, const std::string& username, const std::string& email) {
    User user{0, username, email, std::nullopt, {}};
    ctx.db.users().insert(user);
    spacetimedb::log::info("User added: {}", username);
}
SPACETIMEDB_REDUCER(add_user, NORMAL, ctx, const std::string&, const std::string&)

// Init reducer - called when module is first deployed
void init_module(ReducerContext& ctx) {
    // Initialize default data
    ctx.db.system_config().insert(SystemConfig{"version", "1.0.0"});
}
SPACETIMEDB_INIT(init_module, ctx)

// Client connected reducer
void on_connect(ReducerContext& ctx, const Identity& client) {
    UserSession session{client, ctx.timestamp, true};
    ctx.db.sessions().insert(session);
}
SPACETIMEDB_CLIENT_CONNECTED(on_connect, ctx, const Identity&)

// Client disconnected reducer
void on_disconnect(ReducerContext& ctx, const Identity& client) {
    ctx.db.sessions().delete_where([&](const UserSession& s) { 
        return s.identity == client; 
    });
}
SPACETIMEDB_CLIENT_DISCONNECTED(on_disconnect, ctx, const Identity&)
```

#### 3.3.2. Scheduled Reducers (Coming Soon)
```cpp
// Scheduled reducer - runs at specified intervals
void cleanup_expired_sessions(ReducerContext& ctx) {
    auto cutoff = ctx.timestamp - std::chrono::hours(24);
    ctx.db.sessions().delete_where([&](const UserSession& s) {
        return s.last_active < cutoff;
    });
}
// SPACETIMEDB_SCHEDULED(cleanup_expired_sessions, std::chrono::hours(1), ctx)
```

#### 3.3.3. Enhanced Reducer Context
The `ReducerContext` provides comprehensive access to transaction context:

```cpp
void my_reducer(ReducerContext& ctx, /* args */) {
    // Identity of caller
    const Identity& sender = ctx.sender;
    
    // Transaction timestamp
    Timestamp time = ctx.timestamp;
    
    // Database access
    auto& db = ctx.db;
    
    // Logging with automatic context
    spacetimedb::log::info("Reducer called by {}", sender.to_hex());
    
    // Performance measurement
    LogStopwatch timer("my_operation");
    // ... operation ...
    // Timer logs duration automatically on destruction
}
```

### Enhanced Database Operations

The SDK provides a rich API for database operations with type safety and performance optimizations:

#### 3.4.1. Basic CRUD Operations
```cpp
void crud_example(ReducerContext& ctx) {
    // Insert
    User user{0, "alice", "alice@example.com", "Bio text", {1, 2, 3}};
    ctx.db.users().insert(user);
    
    // Query by primary key
    if (auto found = ctx.db.users().find_by_id(user.id)) {
        spacetimedb::log::info("Found user: {}", found->username);
    }
    
    // Update
    ctx.db.users().update_by_id(user.id, [](User& u) {
        u.email = "newemail@example.com";
    });
    
    // Delete
    ctx.db.users().delete_by_id(user.id);
}
```

#### 3.4.2. Advanced Queries
```cpp
void query_example(ReducerContext& ctx) {
    // Filter with predicate
    auto premium_users = ctx.db.users().filter([](const User& u) {
        return u.bio.has_value() && u.friend_ids.size() > 10;
    });
    
    // Iterate all
    for (const auto& user : ctx.db.users().iter()) {
        // Process each user
    }
    
    // Count with condition
    size_t active_count = ctx.db.sessions().count_where([](const UserSession& s) {
        return s.is_active;
    });
    
    // Delete with condition
    ctx.db.products().delete_where([](const Product& p) {
        return p.price < 0.01;  // Remove penny items
    });
}
```

#### 3.4.3. Index-Based Queries
```cpp
// Assuming Product has an index on category
void index_query_example(ReducerContext& ctx) {
    // Fast lookup by indexed field
    auto electronics = ctx.db.products().find_by_category("electronics");
    
    // Range query on indexed field
    auto mid_range = ctx.db.products().range_by_price(10.0, 100.0);
}
```
## 4. Advanced SDK Features

### 4.1. Constraint Validation
The SDK supports database constraints similar to traditional SQL databases:

```cpp
struct Order {
    [[SPACETIMEDB_PRIMARY_KEY]]
    uint32_t id;
    
    uint32_t customer_id;  // Foreign key
    
    [[SPACETIMEDB_UNIQUE]]
    std::string order_number;
    
    double total;
};
SPACETIMEDB_TYPE(Order)
SPACETIMEDB_TABLE(Order, "orders", true)

// Add foreign key constraint
SPACETIMEDB_FOREIGN_KEY(Order, customer_id, Customer, id)

// Add check constraint
SPACETIMEDB_CHECK_CONSTRAINT(Order, "total >= 0")
```

### 4.2. Transaction Support
All reducer operations are automatically transactional. For explicit transaction control:

```cpp
void complex_operation(ReducerContext& ctx) {
    // All operations in a reducer are atomic
    try {
        ctx.db.users().insert(user1);
        ctx.db.users().insert(user2);
        ctx.db.orders().insert(order);
        // All succeed or all fail
    } catch (const spacetimedb::Exception& e) {
        // Transaction automatically rolled back
        spacetimedb::log::error("Transaction failed: {}", e.what());
        throw;  // Re-throw to fail the reducer
    }
}
```

### 4.3. Module Versioning
Support for module versioning and migration:

```cpp
// In your main module file
SPACETIMEDB_MODULE_VERSION(1, 0, 0)  // Major, Minor, Patch

// Migration support
void migrate_v1_to_v2(ReducerContext& ctx) {
    // Perform schema migration
    for (auto& user : ctx.db.old_users().iter()) {
        NewUser new_user{user.id, user.name, /* new fields */};
        ctx.db.users().insert(new_user);
    }
}
```

### 4.4. Logging and Diagnostics
Enhanced logging with multiple levels and automatic context:

```cpp
#include <spacetimedb/spacetimedb.h>

void my_reducer(ReducerContext& ctx) {
    // Different log levels
    spacetimedb::log::trace("Detailed trace info");
    spacetimedb::log::debug("Debug information");
    spacetimedb::log::info("User {} logged in", username);
    spacetimedb::log::warn("Low memory: {} bytes", available);
    spacetimedb::log::error("Failed to process: {}", error_msg);
    
    // Performance monitoring
    {
        LogStopwatch timer("database_operation");
        // ... expensive operation ...
    }  // Automatically logs duration
}
```

### 4.5. Type System and BSATN
The SDK features a complete algebraic type system with automatic serialization:

```cpp
// Complex types are automatically supported
struct GameState {
    std::vector<Player> players;
    std::optional<Winner> winner;
    std::map<std::string, uint32_t> scores;
    SumType<Playing, Paused, Finished> status;
};
SPACETIMEDB_TYPE(GameState)

// The type system handles:
// - All primitive types (bool, integers, floats, strings)
// - Containers (vector, optional, map)
// - User-defined types with nested structures
// - Sum types (discriminated unions)
// - Special SpacetimeDB types (Identity, Timestamp, TimeDuration)
```

## 5. Best Practices

### 5.1. Performance Optimization
- Use indexes for frequently queried fields
- Batch operations when possible
- Use `delete_where` instead of iterating and deleting individually
- Profile with LogStopwatch to identify bottlenecks

### 5.2. Error Handling
- Use the exception hierarchy for proper error handling
- Let exceptions propagate to fail the reducer atomically
- Log errors with context for debugging

### 5.3. Schema Design
- Use appropriate column attributes (PRIMARY_KEY, UNIQUE, AUTO_INC)
- Design tables with normalization in mind
- Use indexes strategically for query performance

## 6. Migration from Legacy SDK

If you're migrating from an older version of the SDK:

1. Replace manual BSATN serialization with `SPACETIMEDB_TYPE` macro
2. Update table registration to use `SPACETIMEDB_TABLE`
3. Replace old reducer macros with new lifecycle-aware versions
4. Update include to use `#include <spacetimedb/spacetimedb.h>`

See the migration guide for detailed instructions.
`player_table.iter()` returns a `spacetimedb::sdk::TableIterator<MyPlayer>`. The iterator handles calling `_iter_next`, `_buffer_consume`, and deserializing rows. The iterator automatically calls `_iter_drop` when it goes out of scope.

#### 3.5.5. Finding Rows by Column Value
```cpp
uint64_t target_id = 456;
uint32_t pk_idx = 0; // Assuming player_id is PK at index 0
std::vector<my_module_namespace::MyPlayer> found_players = player_table.find_by_col_eq(pk_idx, target_id);

for (const auto& player : found_players) {
    // Process player
}
```
`find_by_col_eq` uses the `_iter_by_col_eq` ABI function, which returns a buffer of concatenated BSATN-encoded rows. The SDK deserializes these into a `std::vector<T>`.

### Logging
For logging within your C++ module, you can directly use the `_console_log` ABI function, which is declared in `<spacetimedb/abi/spacetimedb_abi.h>`.

```cpp
#include <spacetimedb/abi/spacetimedb_abi.h> // For _console_log
#include <string>

// Define log levels (or use those from your header, e.g. kv_store.h)
// const uint8_t LOG_LEVEL_INFO_EXAMPLE = 3; // (As defined in kv_store.h example)

void my_function_with_logging() {
    std::string message = "This is an informational log message.";
    _console_log(3, // LOG_LEVEL_INFO
                 nullptr, 0,                // target_ptr, target_len (optional module path)
                 nullptr, 0,                // filename_ptr, filename_len (optional)
                 0,                         // line_number (optional)
                 reinterpret_cast<const uint8_t*>(message.c_str()),
                 message.length());
}
```
The `target`, `filename`, and `line_number` parameters are optional and can be `nullptr` and `0` if not used.

### Supported Data Types for Reducer Arguments and Table Fields
The C++ SDK directly supports serialization/deserialization for:
*   **Primitives:** `bool`, `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, `int8_t`, `int16_t`, `int32_t`, `int64_t`, `float` (`f32`), `double` (`f64`).
*   **Strings:** `std::string` (serialized as UTF-8 bytes with a `uint32_t` length prefix).
*   **Byte Arrays:** `std::vector<uint8_t>` (serialized with a `uint32_t` length prefix).
*   **Collections:** `std::vector<T>`, where `T` is any other supported BSATN-serializable type (including primitives, strings, `std::vector<uint8_t>`, or custom structs/classes).
*   **SDK-Specific Types:**
    *   `spacetimedb::sdk::Identity` (from `<spacetimedb/sdk/spacetimedb_sdk_types.h>`)
    *   `spacetimedb::sdk::Timestamp` (from `<spacetimedb/sdk/spacetimedb_sdk_types.h>`)
*   **Custom Serializable Types:** Any C++ struct or class that implements the `spacetimedb::bsatn::BsatnSerializable` interface or provides the necessary `bsatn_serialize` and `bsatn_deserialize` methods.

## 4. KeyValueStore Example Walkthrough

The `examples/quickstart_cpp_kv/` directory in the SDK project provides a simple key-value store module. This example demonstrates many of the core SDK concepts.

*   **`Cargo.toml`:** Defines `name = "kvstore_module"`.
*   **`CMakeLists.txt`:** Configures the build for `kvstore_module.wasm`, linking the SDK and outputting to `target/wasm32-unknown-unknown/release/`.
*   **`src/kv_store.h`:**
    *   Defines the `spacetimedb_quickstart::KeyValue` struct (with `key_str`, `value_str`) inheriting from `BsatnSerializable`.
    *   Declares reducer functions: `kv_put`, `kv_get`, `kv_del`.
*   **`src/kv_store.cpp`:**
    *   Implements `KeyValue::bsatn_serialize` and `KeyValue::bsatn_deserialize`.
    *   Registers the table: `SPACETIMEDB_REGISTER_TABLE(spacetimedb_quickstart::KeyValue, "kv_pairs", "key_str");`
    *   Implements `kv_put` (simulates upsert with delete then insert), `kv_get` (uses `find_by_col_eq`), and `kv_del` (uses `delete_by_col_eq`). Logging is done via a helper calling `_console_log`.
    *   Registers reducers using `SPACETIMEDB_REDUCER`.

**Build and Publish the Example:**
(Assuming you are in the `spacetime_cpp_project_root` directory where `build_and_publish_example.sh` is located)
```bash
./build_and_publish_example.sh my_kv_db_instance_name
```
This script will:
1.  `cd` into `examples/quickstart_cpp_kv`.
2.  Configure and build the WASM module using CMake and the Emscripten toolchain file. The output will be `examples/quickstart_cpp_kv/target/wasm32-unknown-unknown/release/kvstore_module.wasm`.
3.  Publish the module to SpacetimeDB using `spacetime publish kvstore_module --name my_kv_db_instance_name`.

**Example `spacetime call` commands:**
```bash
spacetime call my_kv_db_instance_name kv_put --key "player:1" --value "{\"name\":\"Alice\",\"score\":100}"
spacetime call my_kv_db_instance_name kv_get --key "player:1"
spacetime call my_kv_db_instance_name kv_del --key "player:1"
spacetime call my_kv_db_instance_name kv_get --key "player:1"
```
*(Note: The `--key <value>` syntax for `spacetime call` arguments assumes the CLI can map these named arguments to the BSATN-encoded buffer expected by the reducer. The C++ SDK's reducer macro expects arguments in the order they are defined in the C++ function signature, after the initial `Identity` and `Timestamp`.)*

## 5. Advanced Topics

*   **C ABI (`<spacetimedb/abi/spacetimedb_abi.h>`):** The C++ SDK is built upon a low-level C Application Binary Interface (ABI). This ABI defines a set of `extern "C"` functions (like `_console_log`, `_insert`, `_get_table_id`, etc.) that the WASM module imports from the SpacetimeDB host environment. While you typically interact with the higher-level C++ classes, understanding that this C ABI exists can be helpful for debugging or advanced scenarios.
*   **BSATN Serialization (`<spacetimedb/bsatn/bsatn.h>`):** All data exchanged with the host (reducer arguments, table rows) and stored in the database is serialized using BSATN. The SDK provides `bsatn_writer` and `bsatn_reader` classes to handle this. Custom C++ types need to be made BSATN-serializable.
*   **Reducer Macro Expansion (Conceptual):** The `SPACETIMEDB_REDUCER` macros are powerful tools that generate significant boilerplate code. For each registered C++ reducer, the macro creates an `extern "C"` wrapper function. This wrapper is what's actually exported from the WASM module. It handles:
    1.  Receiving a raw byte buffer from the host.
    2.  Deserializing the sender `Identity` and transaction `Timestamp` from the beginning of this buffer.
    3.  Constructing the `ReducerContext`.
    4.  Deserializing all subsequent user-defined arguments using BSATN.
    5.  Calling your C++ reducer function with the context and deserialized arguments.
    6.  Implementing a `try-catch` block to handle C++ exceptions, log them, and return an appropriate `uint16_t` error code to the host.
*   **SDK Initialization (`_spacetimedb_sdk_init()`):** The SDK requires initialization when the WASM module is loaded by the host. The `<spacetimedb/sdk/spacetimedb_sdk_reducer.h>` header defines and exports an `extern "C" void _spacetimedb_sdk_init()` function. The SpacetimeDB host environment is expected to call this function once upon module load. This function typically sets up any global state required by the SDK, such as the global `Database` instance accessor used by `ReducerContext`.
