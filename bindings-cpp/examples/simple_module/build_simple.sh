#!/bin/bash
# Build script for simple_module with all dependencies

LIBRARY_PATH="../../library"
INCLUDES="-I$LIBRARY_PATH/include"

# Compile all source files to object files
emcc -std=c++20 $INCLUDES -c $LIBRARY_PATH/src/bsatn/reader.cpp -o reader.o
emcc -std=c++20 $INCLUDES -c $LIBRARY_PATH/src/bsatn/writer.cpp -o writer.o
emcc -std=c++20 $INCLUDES -c $LIBRARY_PATH/src/internal/Module.cpp -o Module.o
emcc -std=c++20 $INCLUDES -c $LIBRARY_PATH/src/spacetimedb_library_types.cpp -o spacetimedb_library_types.o
emcc -std=c++20 $INCLUDES -c simple_module.cpp -o simple_module.o

# Link everything together
emcc -std=c++20 \
  reader.o writer.o Module.o spacetimedb_library_types.o simple_module.o \
  -s STANDALONE_WASM=1 \
  -s FILESYSTEM=0 \
  -s DISABLE_EXCEPTION_CATCHING=1 \
  -O2 \
  -Wl,--no-entry \
  -o simple_module.wasm

# Clean up object files
rm -f *.o