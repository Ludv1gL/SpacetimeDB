#!/bin/bash
# Build script for comprehensive integration test

set -e

echo "Building comprehensive integration test..."

# Find emcc
EMCC=$(which emcc || echo "/home/ludvi/emsdk/upstream/emscripten/emcc")

if [ ! -x "$EMCC" ]; then
    echo "Error: emcc not found. Please ensure Emscripten is installed and in PATH"
    exit 1
fi

# Get SDK path
SDK_PATH="../../sdk/include"

# Build the module
echo "Compiling comprehensive_integration_test.cpp..."
$EMCC -std=c++20 \
    comprehensive_integration_test.cpp \
    -I$SDK_PATH \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o comprehensive_integration_test.wasm

if [ -f comprehensive_integration_test.wasm ]; then
    echo "✓ Build successful: comprehensive_integration_test.wasm"
    echo "  Size: $(du -h comprehensive_integration_test.wasm | cut -f1)"
    echo ""
    echo "To test the module:"
    echo "  1. spacetime publish comprehensive_test --bin-path comprehensive_integration_test.wasm"
    echo "  2. spacetime call comprehensive_test run_all_tests"
    echo "  3. spacetime logs comprehensive_test -f"
else
    echo "✗ Build failed"
    exit 1
fi