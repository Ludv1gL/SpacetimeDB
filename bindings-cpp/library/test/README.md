# BSATN C++ Tests

This directory contains comprehensive tests for the C++ implementation of BSATN (Binary SpacetimeDB Algebraic Type Notation).

## Test Coverage

The test suite (`test_bsatn.cpp`) verifies:

- **Primitive Types**: All basic types (bool, integers, floats, strings)
- **Module Library Types**: SpacetimeDB types (Identity, Timestamp, ConnectionId)
- **Collections**: Vectors, nested collections, empty containers
- **Option Types**: Nullable values matching Rust/C# semantics
- **Structures**: Complex structs using `SPACETIMEDB_BSATN_STRUCT` macro
- **Sum Types**: Discriminated unions with multiple variants
- **Binary Format**: Exact encoding verification
- **Error Handling**: Buffer underruns, invalid tags
- **Random Testing**: Property-based testing with 100+ random cases

## Building and Running

### Using CMake (Recommended)
```bash
mkdir build && cd build
cmake ..
make
ctest              # Run tests
ctest -V           # Verbose output
./test_bsatn -v    # Direct execution with verbose
```

### Direct Compilation
```bash
clang++ -std=c++20 -I../include -o test_bsatn test_bsatn.cpp \
    ../src/bsatn/reader.cpp ../src/bsatn/writer.cpp \
    ../src/spacetimedb_library_types.cpp ../src/spacetimedb_library_types_impl.cpp

./test_bsatn       # Run tests
./test_bsatn -v    # Verbose output
```

## Test Structure

The test suite is designed for both development and CI/CD:

- **Concise Output**: Default mode shows only summary
- **Verbose Mode**: `-v` flag shows detailed test progress
- **Fast Execution**: Typically completes in < 1 second
- **Deterministic**: Fixed seed for random tests ensures reproducibility

## Adding New Tests

To add new tests:

1. Add a new `test_<name>()` function
2. Use `ASSERT_ROUNDTRIP(value)` to verify serialization
3. Add `TEST(name)` to the main function
4. Follow the existing pattern for consistency

Example:
```cpp
void test_my_feature() {
    MyType value{...};
    ASSERT_ROUNDTRIP(value);
}

// In main():
TEST(my_feature);
```

## Compatibility

These tests verify that the C++ BSATN implementation is compatible with:
- C# BSATN Runtime (bindings-csharp)
- Rust BSATN (crates/sats)
- SpacetimeDB binary format specification