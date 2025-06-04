#!/bin/bash

# Build the working module

echo "Building working module..."

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
    -o build/working_module.wasm \
    src/working_module.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated build/working_module.wasm"
    ls -lh build/working_module.wasm
else
    echo "Build failed!"
    exit 1
fi