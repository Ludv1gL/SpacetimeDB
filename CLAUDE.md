# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SpacetimeDB is a revolutionary database that merges database and application server functionality into one system. Instead of the traditional architecture (database → app server → clients), SpacetimeDB lets you write your entire backend as "modules" that run **inside** the database. Clients connect directly to the database via WebSocket for real-time updates.

### What Makes SpacetimeDB Unique

- **Module-Based Architecture**: Application logic is compiled to WebAssembly (WASM) and runs inside the database
- **Memory-First Design**: All active data is held in memory for microsecond latency with Write-Ahead-Log (WAL) persistence
- **Real-time Synchronization**: Built-in subscription system automatically syncs changes to connected clients
- **Direct Client Connections**: Clients connect directly to the database via WebSocket, no intermediate servers needed
- **Tagline**: "Multiplayer at the speed of light" - optimized for real-time applications like games, chat, and collaboration tools

## Development Environment

### Rust Toolchain

The development environment has full Rust capabilities for cross-reference development and analysis:

```bash
# Rust toolchain versions
rustc --version  # Rust 1.84.0 (latest stable)
cargo --version  # Cargo 1.84.0

# Build and analyze Rust crates
cargo build --all           # Build all crates in workspace
cargo check --package <name>  # Type-check specific crate
cargo test --package <name>   # Run tests for specific crate
cargo doc --package <name>    # Generate documentation
cargo tree                   # Show dependency tree

# Cross-reference analysis for C++ SDK development
cargo expand --package spacetimedb  # Show macro expansions
cargo run --example <name>          # Run examples for reference
```

### WebAssembly Tools

```bash
# Check Emscripten compiler (for C++ modules)
emcc --version  # Should show version 4.0.9 or later

# Check WASM object dump tool
wasm-objdump --version  # Should show version 1.0.34 or later
```

### Local SpacetimeDB Server

A SpacetimeDB server is running locally and can be accessed using the `spacetime` CLI:

```bash
# Check server status
spacetime server status

# List databases
spacetime list

# Connect to a database
spacetime sql <database-name>
```

## SpacetimeDB Development Commands

### Building
```bash
# Build all crates
cargo build --all

# Build release binaries (CLI, standalone server, and updater)
cargo build --locked --release -p spacetimedb-standalone -p spacetimedb-update -p spacetimedb-cli

# Build specific crate
cargo build -p <crate-name>

# Build WASM modules (run from module directory)
cargo build --target wasm32-unknown-unknown

# Build C++ modules (requires Emscripten)
# Direct compilation with emcc - IMPORTANT: Use these exact flags to avoid WASI imports
emcc -std=c++20 -s STANDALONE_WASM=1 -s FILESYSTEM=0 -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry -o <module_name>.wasm <source.cpp>

# Alternative: Include SDK headers if needed
emcc -std=c++20 <source.cpp> -I<path-to-sdk>/cpp_sdk/sdk/include -s STANDALONE_WASM=1 -s FILESYSTEM=0 -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry -o <module_name>.wasm

# Using CMake (recommended)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<spacetimedb>/cpp_sdk/toolchains/wasm_toolchain.cmake
cmake --build build

# Using the build script (from example directory)
../../build_and_publish_example.sh
```

### Testing
```bash
# Run all tests (includes clippy, cargo test, smoketests, and C# tests)
./tools/run-all-tests.sh

# Run Rust tests only
cargo test --all

# Run specific test
cargo test --package <crate-name> <test-name>

# Run smoketests (Python integration tests)
python3 -m smoketests

# Run specific smoketest
python3 -m smoketests.tests.<test_name>

# Run C# tests
dotnet test crates/bindings-csharp

# Run SQL tests
cargo test -p sqltest

# C++ SDK Integration Testing
# Test C++ module schema output against expected schema
cd cpp_sdk/examples/sdk_test_cpp
emcc -std=c++20 -s STANDALONE_WASM=1 -s FILESYSTEM=0 -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry -o sdk_test.wasm src/sdk_test.cpp
spacetime publish --bin-path sdk_test.wasm sdk-test-validation
spacetime describe sdk-test-validation --json > actual-output.json
diff actual-output.json ../../tests/sdk-test-desc.json  # Should match exactly

# The file cpp_sdk/tests/sdk-test-desc.json contains the expected schema output 
# for a fully implemented sdk_test.cpp with all tables, reducers, and types
```

### Code Quality
```bash
# Run clippy (strict mode with warnings as errors)
cargo clippy --all --tests --benches -- -D warnings
# or use the script
./tools/clippy.sh

# Format code
cargo fmt --all

# Check code without building
cargo check --all --tests --benches
```

### Development Workflow
```bash
# Start local SpacetimeDB server
spacetime start

# Create new module (from module directory)
spacetime init <module-name>

# Publish module to local server
spacetime publish <module-name>

# View logs
spacetime logs <module-name> -f

# Execute SQL queries
spacetime sql <module-name> "SELECT * FROM table_name"
```

## High-Level Architecture

### Core Components

1. **Module System**: Applications are compiled to WebAssembly and run inside the database
   - Modules define tables (data schema) and reducers (business logic)
   - Support for Rust, C#, and C++ modules

2. **Key Crates Structure**:
   - `standalone`: The main server binary that hosts SpacetimeDB instances
   - `core`: Database engine, module host, subscription system, and client connections
   - `cli`: Command-line tool for managing SpacetimeDB
   - `client-api`: HTTP/WebSocket API server for client connections
   - `sats`: SpacetimeDB Algebraic Type System - the type system for data
   - `table`: Low-level table storage implementation
   - `vm`: Virtual machine for query execution
   - `commitlog`: Write-ahead log for durability
   - `bindings`/`bindings-macro`: Rust module SDK
   - `bindings-csharp`: C# module SDK
   - `cpp_sdk`: C++ module SDK (compiled to WASM via Emscripten)

3. **Data Flow**:
   - Clients connect via WebSocket to the client API
   - Reducers (stored procedures) handle all mutations
   - Tables store state in memory with WAL persistence
   - Subscriptions automatically sync changes to connected clients

4. **Module Execution**:
   - Modules run in a WASM sandbox via wasmtime
   - Each module instance has isolated state
   - Reducers execute transactionally
   - Direct table access for queries

### C++ SDK Architecture

The C++ SDK (`/cpp_sdk/`) enables writing SpacetimeDB modules in C++:

- **SDK Headers**: `cpp_sdk/sdk/include/spacetimedb/` - Core SDK functionality
- **Toolchain**: `cpp_sdk/toolchains/wasm_toolchain.cmake` - Emscripten configuration
- **Examples**: `cpp_sdk/examples/` - Reference implementations

**C++ SDK Header Structure** (Fixed as of 2025-01):
- **spacetimedb.h**: Core functionality with conditional ReducerContext definition
- **spacetimedb_easy.h**: Clean syntax overlay providing ModuleDatabase with type-safe table methods
- Uses `SPACETIMEDB_CUSTOM_REDUCER_CONTEXT` flag to avoid redefinition conflicts

**Current Working C++ SDK Syntax**:
```cpp
#include <spacetimedb/spacetimedb.h>

struct OneU8 { uint8_t n; };

// Field registration for complex types with strings
struct Person {
    uint32_t id;
    std::string name;
    uint8_t age;
};

SPACETIMEDB_REGISTER_FIELDS(Person,
    SPACETIMEDB_FIELD(Person, id, uint32_t);
    SPACETIMEDB_FIELD(Person, name, std::string);
    SPACETIMEDB_FIELD(Person, age, uint8_t);
)

SPACETIMEDB_TABLE(OneU8, one_u8, true)  // Simple table
SPACETIMEDB_TABLE(Person, person, true) // Complex table with field registration

SPACETIMEDB_REDUCER(insert_one_u8, spacetimedb::ReducerContext ctx, uint8_t n) {
    OneU8 row{n};
    ctx.db.table<OneU8>("one_u8").insert(row);  // Type-safe syntax
}

SPACETIMEDB_REDUCER(insert_person, spacetimedb::ReducerContext ctx, std::string name, uint8_t age) {
    Person person{0, name, age};  // id auto-generated
    ctx.db.table<Person>("person").insert(person);
}
```

Key C++ SDK components:
- `SPACETIMEDB_TABLE(Type, table_name, is_public)`: Register tables with the module
- `SPACETIMEDB_REGISTER_FIELDS(Type, ...)`: Automatic field registration for complex types
- `SPACETIMEDB_FIELD(struct, field, type)`: Individual field registration
- `SPACETIMEDB_REDUCER(name, spacetimedb::ReducerContext ctx, ...)`: Export functions as reducers
- `ModuleDatabase` class: Type-safe database access via `ctx.db.table<T>(name)`
- `TableHandle<T>` template: Type-safe table operations

**C++ SDK Testing & Validation**:
- `cpp_sdk/tests/sdk-test-desc.json`: Expected schema output for complete sdk_test.cpp implementation
- After publishing a C++ module, use `spacetime describe <database> --json` to get actual schema
- For validation: `diff actual-output.json cpp_sdk/tests/sdk-test-desc.json` should match exactly
- This ensures all tables, reducers, and types are properly registered and match expected schema

**Critical Compilation Flags** (to avoid WASI import errors):
- `-s STANDALONE_WASM=1`: Creates standalone WASM without WASI dependencies
- `-s FILESYSTEM=0`: Disables filesystem support (prevents `fd_close` errors)  
- `-s DISABLE_EXCEPTION_CATCHING=1`: Removes exception handling that can pull in WASI
- `-Wl,--no-entry`: No main entry point (required for SpacetimeDB modules)

**Common C++ SDK Issues & Solutions**:
- **TableHandle constructor missing**: Fixed by adding `TableHandle(const std::string& name)` constructor
- **ReducerContext redefinition**: Use `SPACETIMEDB_CUSTOM_REDUCER_CONTEXT` flag and conditional compilation
- **Module description format errors**: Ensure `__describe_module__` generates correct RawModuleDef V9 format
- **BSATN Option encoding**: Fixed to use tag 0 for Some, tag 1 for None
- **Field registration macro**: Fixed namespace qualification in `SPACETIMEDB_FIELD` macro
- **Current Issue**: String field encoding causing "unknown tag 0x61" errors during module publishing

**C++ Module SDK Status (as of Jan 2025)**:
- ✅ **Working**: Basic table registration, primitive types, reducer registration, BSATN encoding, module exports
- ✅ **Working**: Simple modules without strings publish and run successfully
- ⚠️ **In Progress**: String field encoding issue blocking complex table usage
- ❌ **Missing**: Advanced query operations (count, iter, delete, index accessors)
- ❌ **Missing**: Error handling (try_insert, constraint violations)
- ❌ **Missing**: Built-in reducers (init, client_connected, client_disconnected)
- ❌ **Missing**: ReducerContext metadata (timestamp, sender, connection_id)
- ❌ **Missing**: Module utilities (RNG, LogStopwatch, credential management)

**Module SDK Completion**: ~32% (7/22 core features implemented)
**Client SDK**: Not started (0% - focused on Module SDK first)

### Key Design Patterns

1. **Everything is a Module**: All application logic lives in modules, not external services
2. **Reducers for Mutations**: All state changes go through reducers (like stored procedures)
3. **Automatic Client Sync**: Subscriptions handle real-time updates without manual coding
4. **Memory-First Storage**: All active data in memory with WAL for persistence

### Language Support

**Server-Side Module SDKs** (for writing modules):
- **Rust**: Native support with macros (`#[spacetimedb::table]`, `#[spacetimedb::reducer]`)
- **C#**: Full .NET support with attributes (`[SpacetimeDB.Table]`, `[SpacetimeDB.Reducer]`)
- **C++**: Header-only SDK, compiles via Emscripten (requires `emcc`)

**Client-Side SDKs** (for connecting to SpacetimeDB):
- **Rust**: Async WebSocket client with automatic caching
- **TypeScript/JavaScript**: Generated via codegen, WebSocket-based real-time subscriptions
- **C#**: Unity-compatible for game development

### Development Tips

- When modifying table storage, check `crates/table/` for the low-level implementation
- Client connection logic is in `crates/core/src/client/`
- Module execution environment is in `crates/core/src/host/`
- SQL parsing and execution is in `crates/core/src/sql/`
- WASM module loading uses wasmtime (see `module_host.rs`)
- C++ modules require Emscripten and must match the module name in both CMakeLists.txt and Cargo.toml
- Real-time subscription system is in `crates/core/src/subscription/`
- BSATN (Binary Spacetime Algebraic Type Notation) serialization is in `crates/sats/`

### Using Rust for C++ SDK Development

**Cross-Reference Analysis**: The development environment supports using Rust as a reference for C++ SDK implementation:

```bash
# Study Rust SDK implementations for C++ feature development
cargo check --package spacetimedb        # Rust Module SDK (bindings)
cargo check --package spacetimedb-cli    # CLI tools and codegen

# Analyze specific features for C++ implementation
cargo expand --package spacetimedb       # Show macro expansions for table/reducer registration
cargo doc --package spacetimedb --open   # Browse Rust SDK documentation
cargo test --package spacetimedb -- --nocapture  # See test patterns

# Extract type definitions and schemas
cargo run --bin spacetime -- describe <db> --json  # Compare schema outputs
cargo tree --package spacetimedb         # Understand dependencies and structure
```

**Key Rust Crates for C++ Reference**:
- `spacetimedb` (`crates/bindings/`) - Rust Module SDK with complete table/reducer system
- `spacetimedb-sats` (`crates/sats/`) - BSATN serialization and type system
- `spacetimedb-lib` (`crates/lib/`) - Core types and module definitions
- `spacetimedb-bindings-macro` (`crates/bindings-macro/`) - Macro implementations to study

**Cross-Language Validation**:
- Use Rust modules to generate reference schemas for testing C++ output
- Compare BSATN encoding between Rust and C++ implementations  
- Study error handling patterns in Rust for C++ exception design
- Extract complete feature lists from Rust SDK for C++ roadmap planning

### SpacetimeDB Module Description Format (BSATN)

**Important for C++ SDK debugging**: The `__describe_module__` function must generate correct RawModuleDef format:

- **RawModuleDef enum** (from `crates/lib/src/lib.rs`):
  ```rust
  pub enum RawModuleDef {
      V8BackCompat(RawModuleDefV8),  // tag = 0
      V9(RawModuleDefV9),            // tag = 1
  }
  ```

- **Correct V9 format** (tag = 1):
  ```cpp
  // C++ __describe_module__ implementation should:
  buf.push_back(1);  // RawModuleDef::V9 tag
  // Followed by RawModuleDefV9 structure:
  // - typespace: Typespace
  // - tables: Vec<RawTableDefV9>  
  // - reducers: Vec<RawReducerDefV9>
  // - types: Vec<RawTypeDefV9>
  // - misc_exports: Vec<RawMiscModuleExportV9>
  // - row_level_security: Vec<RawRowLevelSecurityDefV9>
  ```

- **Common error**: "unknown tag 0x2 for sum type RawModuleDef" means incorrect tag or format
- **Reference implementation**: Working Rust code in `crates/bindings/src/rt.rs` lines 416-419

### Testing Strategy

- Unit tests: Standard Rust tests in each crate
- Integration tests: Python smoketests in `/smoketests/` test full server functionality
- Module tests: Example modules in `/modules/` serve as both examples and tests
- SQL compatibility: `sqltest` crate runs standard SQL test suites

## Key Technologies & Dependencies

- **Runtime**: Rust-based core with `tokio` async runtime
- **WASM Execution**: `wasmtime` for sandboxed module execution
- **Networking**: `axum` for HTTP server, `tokio-tungstenite` for WebSocket
- **Serialization**: Custom BSATN (Binary Spacetime Algebraic Type Notation) format
- **Storage**: Custom memory-mapped table storage with B-tree indexes
- **Metrics**: Prometheus integration for monitoring
- **Security**: JWT-based authentication system