#!/bin/bash

# Build the super minimal module

echo "Building super minimal module..."

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
    -o build/super_minimal.wasm \
    src/super_minimal.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated build/super_minimal.wasm"
    ls -lh build/super_minimal.wasm
else
    echo "Build failed!"
    exit 1
fi