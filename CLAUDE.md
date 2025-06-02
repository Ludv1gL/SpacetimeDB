# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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

SpacetimeDB is a unique database system where application logic runs inside the database as "modules". This eliminates the need for separate application servers.

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

### Development Tips

- When modifying table storage, check `crates/table/` for the low-level implementation
- Client connection logic is in `crates/core/src/client/`
- Module execution environment is in `crates/core/src/host/`
- SQL parsing and execution is in `crates/core/src/sql/`
- WASM module loading uses wasmtime (see `module_host.rs`)
- C++ modules require Emscripten and must match the module name in both CMakeLists.txt and Cargo.toml

### Testing Strategy

- Unit tests: Standard Rust tests in each crate
- Integration tests: Python smoketests in `/smoketests/` test full server functionality
- Module tests: Example modules in `/modules/` serve as both examples and tests
- SQL compatibility: `sqltest` crate runs standard SQL test suites