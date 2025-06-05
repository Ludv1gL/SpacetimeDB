#!/bin/bash
# Build script for SpacetimeDB C++ modules using generic CMake

# Check if source file is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <source_file> [no-lib]"
    echo ""
    echo "Examples:"
    echo "  $0 src/lib_simple_table_test.cpp        # Build with library"
    echo "  $0 src/lib_simple_table_test.cpp no-lib # Build without library"
    echo ""
    echo "Available modules in src/:"
    ls src/*.cpp 2>/dev/null | sed 's/^/  /'
    exit 1
fi

SOURCE_FILE=$1
LINK_LIBRARY="ON"

# Check if no-lib flag is provided
if [ "$2" = "no-lib" ]; then
    LINK_LIBRARY="OFF"
fi

# Extract output name from source file
OUTPUT_NAME=$(basename "$SOURCE_FILE" .cpp)

echo "Building module from: $SOURCE_FILE"
echo "Link library: $LINK_LIBRARY"
echo "Output: build/${OUTPUT_NAME}.wasm"

# Configure with CMake
emcmake cmake -B build \
    -DMODULE_SOURCE="$SOURCE_FILE" \
    -DLINK_LIBRARY=$LINK_LIBRARY \
    -DOUTPUT_NAME="$OUTPUT_NAME"

# Build
cmake --build build

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful! Output: build/${OUTPUT_NAME}.wasm"
    echo ""
    echo "To publish:"
    echo "  spacetime publish <name> -b build/${OUTPUT_NAME}.wasm -c"
else
    echo "Build failed!"
    exit 1
fi