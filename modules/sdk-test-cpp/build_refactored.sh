#!/bin/bash

# Build the refactored test module using centralized ABI

echo "Building refactored test module..."

# Create build directory
mkdir -p build

# Compile with emcc
emcc -std=c++20 \
    -I../../cpp_sdk/sdk/include \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -Wl,--export-dynamic \
    -o build/refactored_test.wasm \
    src/refactored_test.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated build/refactored_test.wasm"
    ls -lh build/refactored_test.wasm
    
    # Check exports
    if command -v wasm-objdump &> /dev/null; then
        echo ""
        echo "Module exports:"
        wasm-objdump -x build/refactored_test.wasm | grep -E "(Export|func.*__)" | head -10
    fi
else
    echo "Build failed!"
    exit 1
fi