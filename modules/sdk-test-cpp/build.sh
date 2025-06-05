#!/bin/bash

# Build script for sdk-test-cpp module

set -e

# Clean previous build
rm -rf build

# Create build directory
mkdir -p build
cd build

# Configure with emscripten
emcmake cmake ..

# Build
emmake make

echo "Build complete! WASM module is at: build/sdk_test_cpp.wasm"