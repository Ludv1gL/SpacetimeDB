cmake_minimum_required(VERSION 3.20)
project(sdk-test-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to the C++ Module Library
set(MODULE_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../bindings-cpp/library")

# Include the Module Library headers
include_directories(${MODULE_LIBRARY_DIR}/include)

# Add the main module source file
# Use lib_minimal.cpp for now as it compiles without WASI dependencies
# Switch to lib.cpp once WASI issues are resolved
add_executable(sdk-test-cpp src/lib_minimal.cpp)

# Link the Module Library
file(GLOB_RECURSE MODULE_LIBRARY_SOURCES
    "${MODULE_LIBRARY_DIR}/src/*.cpp"
)

# Create a static library from Module Library sources
add_library(spacetimedb_module_library STATIC ${MODULE_LIBRARY_SOURCES})
target_include_directories(spacetimedb_module_library PUBLIC ${MODULE_LIBRARY_DIR}/include)
target_compile_definitions(spacetimedb_module_library PUBLIC SPACETIMEDB_MODULE_LIBRARY)

# Link the module library to our module
target_link_libraries(sdk-test-cpp spacetimedb_module_library)

# Set output name to match expected WASM module name
set_target_properties(sdk-test-cpp PROPERTIES OUTPUT_NAME "sdk_test_cpp")

# If using Emscripten, add appropriate flags
if(EMSCRIPTEN)
    set_target_properties(sdk-test-cpp PROPERTIES
        SUFFIX ".wasm"
        COMPILE_FLAGS ""
        LINK_FLAGS "-s STANDALONE_WASM=1 -s EXPORT_ALL=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s DISABLE_EXCEPTION_CATCHING=1 -s MALLOC=emmalloc -s WASM=1 --no-entry -s FILESYSTEM=0 -s INITIAL_MEMORY=16MB -s ALLOW_MEMORY_GROWTH=0"
    )
endif()