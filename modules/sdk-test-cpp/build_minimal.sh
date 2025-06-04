#!/bin/bash

# Build the minimal SDK test module

echo "Building minimal SDK test..."

# SDK path
SDK_PATH="../../cpp_sdk/sdk"

# Create build directory
mkdir -p build

# Compile with emcc including all SDK sources
emcc -std=c++20 \
    -I${SDK_PATH}/include \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -Wl,--export-dynamic \
    -o build/minimal_sdk_test.wasm \
    src/minimal_sdk_test.cpp \
    ${SDK_PATH}/src/abi/module_exports.cpp \
    ${SDK_PATH}/src/internal/Module.cpp \
    ${SDK_PATH}/src/internal/FFI.cpp \
    ${SDK_PATH}/src/spacetimedb_sdk_table_registry.cpp \
    ${SDK_PATH}/src/spacetimedb_sdk_types.cpp \
    ${SDK_PATH}/src/spacetimedb_sdk_types_impl.cpp \
    ${SDK_PATH}/src/database.cpp \
    ${SDK_PATH}/src/reducer_context.cpp \
    ${SDK_PATH}/src/logging.cpp \
    ${SDK_PATH}/src/bsatn/reader.cpp \
    ${SDK_PATH}/src/bsatn/writer.cpp \
    ${SDK_PATH}/src/wasi_shims.cpp

if [ $? -eq 0 ]; then
    echo "Build successful! Generated build/minimal_sdk_test.wasm"
    ls -lh build/minimal_sdk_test.wasm
    
    # Check exports
    if command -v wasm-objdump &> /dev/null; then
        echo ""
        echo "Module exports:"
        wasm-objdump -x build/minimal_sdk_test.wasm | grep -A 20 "Export\["
    fi
else
    echo "Build failed!"
    exit 1
fi