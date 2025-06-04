#!/bin/bash

# Build the simple test module

echo "Building simple test..."

# Create build directory
mkdir -p build

# Compile with minimal dependencies
emcc -std=c++20 \
    -I../../cpp_sdk/sdk/include \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -Wl,--export-dynamic \
    -o build/simple_test.wasm \
    src/simple_test.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated build/simple_test.wasm"
    ls -lh build/simple_test.wasm
else
    echo "Build failed!"
    exit 1
fi