#!/bin/bash

# Build script for constraint validation examples

set -e

echo "Building constraint validation examples..."

# Set paths
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SDK_DIR="$SCRIPT_DIR/../../sdk"
INCLUDE_DIR="$SDK_DIR/include"

# Ensure we have emcc
if ! command -v emcc &> /dev/null; then
    echo "Error: emcc (Emscripten) not found. Please install Emscripten."
    exit 1
fi

# Common flags
COMMON_FLAGS="-std=c++20 -I$INCLUDE_DIR -Os"
WASM_FLAGS="-s STANDALONE_WASM=1 -s FILESYSTEM=0 -s DISABLE_EXCEPTION_CATCHING=1 -s ALLOW_MEMORY_GROWTH=1 -Wl,--no-entry"

# Build constraint validation example
echo "Building constraint_validation_example.wasm..."
emcc $COMMON_FLAGS \
    constraint_validation_example.cpp \
    $SDK_DIR/src/constraint_validation.cpp \
    $WASM_FLAGS \
    -o constraint_validation_example.wasm

# Build foreign key validation example
echo "Building foreign_key_validation.wasm..."
emcc $COMMON_FLAGS \
    foreign_key_validation.cpp \
    $SDK_DIR/src/constraint_validation.cpp \
    $WASM_FLAGS \
    -o foreign_key_validation.wasm

echo "Build complete!"
echo ""
echo "To test the modules:"
echo "1. Publish: spacetime publish constraint_validation_example --bin-path constraint_validation_example.wasm"
echo "2. Run tests: spacetime call constraint_validation_example test_constraints"
echo "3. Show constraints: spacetime call constraint_validation_example show_constraints"
echo ""
echo "For foreign key example:"
echo "1. Publish: spacetime publish foreign_key_example --bin-path foreign_key_validation.wasm"
echo "2. Run tests: spacetime call foreign_key_example test_foreign_keys"
echo "3. Show relationships: spacetime call foreign_key_example show_relationships"