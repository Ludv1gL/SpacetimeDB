#!/bin/bash

# Build the sdk-test-cpp module

echo "Building sdk-test-cpp..."

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "Error: emcc (Emscripten) not found. Please install Emscripten."
    exit 1
fi

# Create build directory
mkdir -p build

# Compile with proper flags
emcc -std=c++20 \
    -I../../cpp_sdk/sdk/include \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -Wl,--export-dynamic \
    -o build/sdk_test_cpp.wasm \
    src/lib.cpp \
    ../../cpp_sdk/sdk/src/bsatn/writer.cpp \
    ../../cpp_sdk/sdk/src/bsatn/reader.cpp \
    ../../cpp_sdk/sdk/src/internal/Module.cpp \
    ../../cpp_sdk/sdk/src/internal/FFI.cpp \
    ../../cpp_sdk/sdk/src/abi/module_exports.cpp \
    ../../cpp_sdk/sdk/src/reducer_context.cpp \
    ../../cpp_sdk/sdk/src/spacetimedb_sdk_types.cpp \
    ../../cpp_sdk/sdk/src/spacetimedb_sdk_table_registry.cpp \
    ../../cpp_sdk/sdk/src/logging.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated build/sdk_test_cpp.wasm"
    
    # Show module size
    ls -lh build/sdk_test_cpp.wasm
    
    # Optional: Inspect the generated WASM module
    if command -v wasm-objdump &> /dev/null; then
        echo ""
        echo "Module exports (first 20):"
        wasm-objdump -x build/sdk_test_cpp.wasm | grep -A 20 "Export section" | head -20
    fi
else
    echo "Build failed!"
    exit 1
fi