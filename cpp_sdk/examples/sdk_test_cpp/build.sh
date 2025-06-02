#!/bin/bash
set -e

# Get the paths to the SpacetimeDB SDK
SDK_INCLUDE_PATH="../../sdk/include"
SDK_SRC_PATH="../../sdk/src"

echo "Building sdk_test.cpp to WASM..."

# First, compile all SDK source files
SDK_OBJECTS=""
for src_file in ${SDK_SRC_PATH}/*.cpp ${SDK_SRC_PATH}/abi/*.cpp ${SDK_SRC_PATH}/bsatn/*.cpp ${SDK_SRC_PATH}/sdk/*.cpp; do
    if [ -f "$src_file" ]; then
        obj_file="/tmp/$(basename ${src_file%.cpp}).o"
        emcc -std=c++20 -I${SDK_INCLUDE_PATH} -c "$src_file" -o "$obj_file"
        SDK_OBJECTS="$SDK_OBJECTS $obj_file"
    fi
done

# Now compile and link our module with the SDK
emcc -std=c++20 \
    -I${SDK_INCLUDE_PATH} \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -s EXPORTED_FUNCTIONS='["__call_reducer__","__describe_module__","_spacetimedb_sdk_init"]' \
    -O2 \
    -Wl,--no-entry \
    -o sdk_test.wasm \
    src/sdk_test.cpp \
    $SDK_OBJECTS

echo "Build complete: sdk_test.wasm"
echo ""
echo "To publish to SpacetimeDB:"
echo "  spacetime publish --bin-path sdk_test.wasm sdk-test"
echo ""
echo "To verify schema:"
echo "  spacetime describe sdk-test --json > actual-schema.json"
echo "  diff actual-schema.json ../../../tests/sdk-test-desc.json"