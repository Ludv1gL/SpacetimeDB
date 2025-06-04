#!/bin/bash
set -e

# Build the builtin reducers example
echo "Building builtin reducers example..."

# Get the SDK include path
SDK_PATH="../../sdk/include"

# Compile the test example (simpler, no strings)
emcc -std=c++20 \
    test_builtin_reducers.cpp \
    -I$SDK_PATH \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o test_builtin_reducers.wasm

echo "Built test_builtin_reducers.wasm"

# Try to compile the full example (with strings - may have issues)
echo "Building full builtin reducers example..."
emcc -std=c++20 \
    builtin_reducers_example.cpp \
    -I$SDK_PATH \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o builtin_reducers_example.wasm || echo "Warning: Full example compilation failed (likely due to string encoding issues)"

echo "Build complete!"

# Test publishing if spacetime CLI is available
if command -v spacetime &> /dev/null; then
    echo "Testing module publication..."
    
    # Try the simple test first
    echo "Publishing test_builtin_reducers..."
    spacetime publish --bin-path test_builtin_reducers.wasm test-builtin-reducers || echo "Publication failed"
    
    # Describe the module to see if built-in reducers are registered
    echo "Describing module..."
    spacetime describe test-builtin-reducers || echo "Describe failed"
fi