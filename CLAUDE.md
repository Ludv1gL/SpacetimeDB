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
# Direct compilation with emcc
emcc -std=c++20 <source.cpp> -I<path-to-sdk>/cpp_sdk/sdk/include -s STANDALONE_WASM=1 -Wl,--no-entry -o <module_name>.wasm -ferror-limit=500

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

Key C++ SDK components:
- `SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS`: Define table row types
- `SPACETIMEDB_TABLE`: Register tables with the module
- `SPACETIMEDB_REDUCER`: Export functions as reducers
- `Database` class: Global database access
- `Table<T>` template: Type-safe table operations

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