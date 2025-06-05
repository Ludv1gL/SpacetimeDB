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

## Project Structure

### Workspace Organization
- **Rust Monorepo**: 35+ crates organized in a Cargo workspace
- **Main Directories**:
  - `crates/` - Core SpacetimeDB functionality
  - `modules/` - Example modules in Rust, C++, and C#
  - `bindings-cpp/` - C++ SDK and module library
  - `crates/bindings-csharp/` - C# SDK
  - `smoketests/` - Python integration tests
  - `tools/` - Development scripts and utilities

### Key Components
- `crates/cli` - SpacetimeDB command-line interface
- `crates/standalone` - Standalone database server
- `crates/core` - Core database engine
- `crates/codegen` - Code generation for language bindings
- `crates/sdk` - Rust SDK for client applications

## Development Commands Reference

### SpacetimeDB CLI Commands
```bash
# Database Management
spacetime start              # Start local database server
spacetime server             # Server management commands

# Module Development
spacetime init <lang>        # Initialize new module project (rust/csharp/cpp)
spacetime build              # Build current module
spacetime publish <path>     # Publish module to database
spacetime publish <path> -b <wasm> --name <db-name>  # Publish with specific WASM file

# Module Interaction
spacetime describe <database> --json    # Get module schema as JSON
spacetime call <database> <reducer> <args>  # Call a reducer
spacetime sql <database> <query>        # Execute SQL query
spacetime logs <database> -f            # Follow module logs
spacetime subscribe <database> <table>  # Subscribe to table changes

# Identity and Access
spacetime login              # Login to SpacetimeDB cloud
spacetime identity           # Show current identity
```

### C++ Module Development

#### Building with Emscripten (Direct)
```bash
# Basic compilation
emcc src/lib.cpp -o module.wasm \
  -s STANDALONE_WASM=1 \
  -s EXPORTED_FUNCTIONS=['_malloc','_free'] \
  -s ERROR_ON_UNDEFINED_SYMBOLS=0 \
  -s DISABLE_EXCEPTION_CATCHING=1 \
  -s DISABLE_EXCEPTION_THROWING=1 \
  -s WASM=1 \
  --no-entry \
  -s FILESYSTEM=0 \
  -s INITIAL_MEMORY=16MB \
  -s ALLOW_MEMORY_GROWTH=1 \
  -I../../bindings-cpp/library/include \
  -std=c++20

# With module library (includes WASI shims)
emcc src/lib.cpp ../../bindings-cpp/library/src/**/*.cpp -o module.wasm \
  [same flags as above]
```

#### Building with CMake
```bash
# Configure with Emscripten
emcmake cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=../../bindings-cpp/toolchains/wasm_toolchain.cmake

# Build
cmake --build build

# The WASM file will be in build/module_name.wasm
```

### Rust Module Development
```bash
# Build for WASM target
cargo build --target wasm32-unknown-unknown --release

# Run tests
cargo test

# Check with clippy
cargo clippy --all-targets --all-features
```

### C# Module Development
```bash
# Build module
dotnet build

# Run tests
dotnet test

# Publish (builds to wasm automatically)
spacetime publish
```

### Testing Commands
```bash
# Run all Rust tests
cargo test --all

# Run specific test suite
cargo test -p <crate-name>

# Run smoke tests (integration tests)
cd smoketests && python -m smoketests

# Run C++ module tests
cd bindings-cpp/examples/simple_module
./test_all_modules.sh

# Run with specific features
cargo test --features unstable
```

### Code Generation
```bash
# Regenerate C++ autogen files
cargo run --example regen-cpp-moduledef

# Regenerate C# autogen files  
cargo run --example regen-csharp-moduledef
```

### Development Scripts
```bash
# Run all tests
./tools/run-all-tests.sh

# Run clippy on all crates
./tools/clippy.sh

# Update test snapshots
./tools/update-test-snapshots.sh

# Performance profiling
./tools/perf.sh
```

## Development Memories and Guidelines

### Code Generation
- Files in `bindings-cpp/library/include/spacetimedb/internal/autogen/` are generated from `/crates/codegen/examples/regen-cpp-moduledef.rs`
- Any file ending with `.g.h`, `.g.cpp`, or `.g.cs` is autogenerated - DO NOT EDIT DIRECTLY
- After modifying Rust types, regenerate bindings with the cargo commands above

### Module Publishing
- Always use `spacetime publish` for deploying modules, not direct cargo/crate commands
- For C++ modules, build the WASM file first, then publish with `-b` flag
- The local server must be running (`spacetime start`) before publishing

### C++ Module Development
- WASI shims are provided in `bindings-cpp/library/src/abi/wasi_shims.cpp`
- Link against `spacetimedb_module_library` to get WASI support for C++ stdlib
- Use CMake with the provided toolchain file for consistent builds
- Module entry point should include table definitions and reducer implementations

### Testing Best Practices
- Always run tests before committing: `cargo test --all`
- For C++ changes, test module compilation with example modules
- Use smoke tests for end-to-end testing of new features
- Check generated code compiles by building example modules

### Common Workflows

#### Adding a New C++ Module
1. Create module directory with CMakeLists.txt
2. Write module code with SPACETIMEDB_TABLE and SPACETIMEDB_REDUCER macros
3. Build with: `emcmake cmake -B build && cmake --build build`
4. Start server: `spacetime start`
5. Publish: `spacetime publish . -b build/module.wasm --name my-module`
6. Test: `spacetime describe my-module --json`

#### Debugging Module Issues
1. Check compilation errors in build output
2. Use `wasm-objdump -x module.wasm` to inspect imports/exports
3. Check logs: `spacetime logs <database> -f`
4. Verify ABI compatibility with `spacetime describe`

#### Updating Language Bindings
1. Modify Rust types in appropriate crates
2. Run code generation: `cargo run --example regen-cpp-moduledef`
3. Test that example modules still compile
4. Update documentation if ABI changes

### Important File Locations
- C++ ABI definitions: `bindings-cpp/library/include/spacetimedb/abi/spacetimedb_abi.h`
- WASI shims: `bindings-cpp/library/src/abi/wasi_shims.cpp`
- Module examples: `bindings-cpp/examples/`, `modules/`
- Autogen templates: `crates/codegen/src/cpp_enhanced.rs`
- CLI implementation: `crates/cli/src/`

### Environment Setup
- Emscripten SDK required for C++ modules
- Rust toolchain with wasm32-unknown-unknown target
- .NET 8.0+ for C# modules
- Python 3.8+ for running smoke tests

### Performance Considerations
- All module state is kept in memory
- Use indexes for frequently queried columns
- Batch operations when possible
- Profile with `tools/perf.sh` for bottlenecks

[Rest of the existing content remains the same...]