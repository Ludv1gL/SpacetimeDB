#!/bin/bash

# Test build script for scheduled reducer example

echo "Building scheduled reducer example..."

# Set paths
EXAMPLE_DIR=$(dirname "$0")
SDK_DIR="$EXAMPLE_DIR/../../sdk"
SDK_INCLUDE="$SDK_DIR/include"

# Compile with emcc
emcc -std=c++20 \
    -I"$SDK_INCLUDE" \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o scheduled_example.wasm \
    scheduled_example.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated scheduled_example.wasm"
    ls -la scheduled_example.wasm
else
    echo "Build failed!"
    exit 1
fi