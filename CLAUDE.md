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

#### Primary Build Method: CMake with Central Build System
The primary way to build C++ modules is using the centralized CMakeLists.txt located at `modules/sdk-test-cpp/CMakeLists.txt`. This system automatically detects the SpacetimeDB root directory and handles all library dependencies.

```bash
# Navigate to the module directory
cd modules/sdk-test-cpp

# Configure and build a specific module
emcmake cmake -B build -DMODULE_NAME=lib_simple_table_test
cmake --build build

# The WASM file will be in build/lib_simple_table_test.wasm

# Available modules:
# - lib                      # Basic module
# - lib_with_macros          # Module using SPACETIMEDB macros
# - lib_empty_with_library   # Empty module with library linked
# - lib_abi_only             # Minimal module with ABI only
# - lib_simple_table_test    # Working example with table and reducer
# - lib_minimal              # Minimal module implementation
# - And more...

# To build for release (with optimizations):
emcmake cmake -B build -DMODULE_NAME=lib_simple_table_test -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The CMake system automatically:
- Finds the SpacetimeDB root directory regardless of where it's placed
- Builds the module library once as a static library
- Determines whether to link the module library based on module type
- Uses consistent WASM compilation flags
- Disables memory growth to ensure compatibility

#### Alternative: Direct Emscripten Compilation
For quick tests or custom builds, you can compile directly:
```bash
# Basic compilation without library
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
  -s ALLOW_MEMORY_GROWTH=0 \  # Important: Must be 0
  -I../../bindings-cpp/library/include \
  -std=c++20
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

#### Building and Testing C++ Modules
1. Navigate to the C++ test module directory:
   ```bash
   cd modules/sdk-test-cpp
   ```

2. Choose a module to build from the available examples:
   - `lib_simple_table_test` - Working example with table and reducer
   - `lib_abi_only` - Minimal module using only ABI header
   - `lib_with_macros` - Module attempting to use SPACETIMEDB macros
   - See CMakeLists.txt for full list

3. Build the module:
   ```bash
   emcmake cmake -B build -DMODULE_NAME=lib_simple_table_test
   cmake --build build
   ```

4. Start the SpacetimeDB server:
   ```bash
   spacetime start
   ```

5. Publish the module:
   ```bash
   spacetime publish . -b build/lib_simple_table_test.wasm --name test-module
   ```

6. Verify the module:
   ```bash
   spacetime describe test-module --json
   spacetime logs test-module -f  # In another terminal
   ```

7. Test the module:
   ```bash
   # Call a reducer
   spacetime call test-module add_person '{"name": "Alice", "age": 30}'
   
   # Query the table
   spacetime sql test-module "SELECT * FROM Person"
   ```

#### Adding a New C++ Module
1. Add your source file to `modules/sdk-test-cpp/src/`
2. Add an entry to CMakeLists.txt:
   ```cmake
   set(MODULE_SOURCES_my_module "src/my_module.cpp")
   ```
3. If your module provides its own exports, add it to MODULES_WITHOUT_LIBRARY
4. Build and test as above

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