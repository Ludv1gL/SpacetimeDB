#!/bin/bash

# Build the basic test module

echo "Building basic test..."

# Create build directory
mkdir -p build

# Compile with emcc
emcc -std=c++20 \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -Wl,--export-dynamic \
    -o build/basic_test.wasm \
    src/basic_test.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated build/basic_test.wasm"
    ls -lh build/basic_test.wasm
    
    # Check exports
    if command -v wasm-objdump &> /dev/null; then
        echo ""
        echo "Module exports:"
        wasm-objdump -x build/basic_test.wasm | grep -A 20 "Export section"
    fi
else
    echo "Build failed!"
    exit 1
fi