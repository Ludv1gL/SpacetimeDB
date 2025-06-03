#!/bin/bash

# Build the module_test example using emcc directly

echo "Building module_test..."

# Compile with proper flags to avoid WASI imports
emcc -std=c++20 \
    -I../../sdk/include \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o module_test.wasm \
    src/module_test.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated module_test.wasm"
    
    # Optional: Inspect the generated WASM module
    if command -v wasm-objdump &> /dev/null; then
        echo ""
        echo "Module exports:"
        wasm-objdump -x module_test.wasm | grep -A 20 "Export section"
    fi
else
    echo "Build failed!"
    exit 1
fi