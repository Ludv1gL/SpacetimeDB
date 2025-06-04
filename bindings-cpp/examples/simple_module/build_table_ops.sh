#!/bin/bash

# Build test_table_ops module

echo "Building test_table_ops module..."

# Compile with emcc
emcc -std=c++20 \
    test_table_ops.cpp \
    -I../../sdk/include \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o test_table_ops.wasm

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "You can now publish with: spacetime publish --bin-path test_table_ops.wasm test-table-ops"
else
    echo "Build failed!"
    exit 1
fi