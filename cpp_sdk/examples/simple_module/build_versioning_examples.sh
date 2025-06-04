#!/bin/bash

# Build script for versioning examples

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SDK_DIR="$SCRIPT_DIR/../.."

echo "Building SpacetimeDB C++ SDK Versioning Examples"
echo "================================================"

# Ensure we have emcc
if ! command -v emcc &> /dev/null; then
    echo "Error: emcc (Emscripten) not found. Please install Emscripten."
    exit 1
fi

# Common flags for WASM compilation
COMMON_FLAGS="-std=c++20 -I$SDK_DIR/sdk/include -s STANDALONE_WASM=1 -s FILESYSTEM=0 -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry"

# Build versioning test (native, not WASM)
echo ""
echo "1. Building versioning test (native)..."
g++ -std=c++17 -I$SDK_DIR/sdk/include -o versioning_test versioning_test.cpp $SDK_DIR/sdk/src/versioning.cpp
echo "   ✓ Built versioning_test"

# Build versioned module v1
echo ""
echo "2. Building versioned module v1.0.0..."
emcc $COMMON_FLAGS -o versioned_module_v1.wasm versioned_module_v1.cpp
echo "   ✓ Built versioned_module_v1.wasm"

# Build versioned module v2
echo ""
echo "3. Building versioned module v2.0.0..."
emcc $COMMON_FLAGS -o versioned_module_v2.wasm versioned_module_v2.cpp
echo "   ✓ Built versioned_module_v2.wasm"

# Build version compatibility example
echo ""
echo "4. Building version compatibility example..."
emcc $COMMON_FLAGS -o version_compatibility.wasm version_compatibility_example.cpp
echo "   ✓ Built version_compatibility.wasm"

echo ""
echo "Build complete!"
echo ""
echo "To test versioning functionality:"
echo "  ./versioning_test"
echo ""
echo "To publish versioned modules:"
echo "  spacetime publish --bin-path versioned_module_v1.wasm my-module-v1"
echo "  spacetime publish --bin-path versioned_module_v2.wasm my-module-v2"
echo ""
echo "To test version compatibility:"
echo "  spacetime publish --bin-path version_compatibility.wasm version-compat-test"