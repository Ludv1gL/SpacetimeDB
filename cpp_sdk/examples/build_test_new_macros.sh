#!/bin/bash

# Build script for test_new_macros.cpp
# This validates the new consolidated macro system

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_DIR="$SCRIPT_DIR/../sdk"
OUTPUT_DIR="$SCRIPT_DIR/build"

# Create build directory
mkdir -p "$OUTPUT_DIR"

echo "Building test_new_macros module with new macro system..."

# Compile with wasi-sdk
/opt/wasi-sdk/bin/clang++ \
    -std=c++20 \
    -fno-exceptions \
    -fno-rtti \
    -nostdlib \
    -ffreestanding \
    -mexec-model=reactor \
    -Wl,--error-limit=0 \
    -Wl,--no-entry \
    -Wl,--export-dynamic \
    -Wl,--export=__describe_module__ \
    -Wl,--export=__call_reducer__ \
    -I"$SDK_DIR/include" \
    -DSPACETIMEDB_MODULE \
    -O2 \
    -o "$OUTPUT_DIR/test_new_macros.wasm" \
    "$SCRIPT_DIR/test_new_macros.cpp"

echo "Build complete: $OUTPUT_DIR/test_new_macros.wasm"

# Check if module was built successfully
if [ -f "$OUTPUT_DIR/test_new_macros.wasm" ]; then
    echo "✅ Module built successfully!"
    ls -lh "$OUTPUT_DIR/test_new_macros.wasm"
    
    # Optional: Try to publish if spacetime CLI is available
    if command -v spacetime &> /dev/null; then
        echo ""
        echo "To publish the module, run:"
        echo "  spacetime publish $OUTPUT_DIR/test_new_macros.wasm --name test_new_macros"
    fi
else
    echo "❌ Build failed!"
    exit 1
fi