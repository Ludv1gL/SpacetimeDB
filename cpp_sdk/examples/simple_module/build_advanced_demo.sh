#!/bin/bash

# Build advanced features demo
echo "Building advanced features demo..."

# Compile with emscripten
emcc -std=c++20 \
    -I../../sdk/include \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o advanced_features_demo.wasm \
    advanced_features_demo.cpp

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "To publish: spacetime publish --bin-path advanced_features_demo.wasm advanced-demo"
else
    echo "Build failed!"
    exit 1
fi