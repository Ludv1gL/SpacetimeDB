# SpacetimeDB C++ SDK CMake Configuration

cmake_minimum_required(VERSION 3.15)
project(SpacetimeDBCppSDK CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SDK source files
set(SDK_SOURCES
    src/abi/module_exports.cpp
    src/abi/reducer_bridge.cpp
    src/bsatn/reader.cpp
    src/bsatn/writer.cpp
    src/database.cpp
    src/logging.cpp
    src/module_def_builder.cpp
    src/raw_module_def_v9.cpp
    src/reducer_context.cpp
    src/sdk/logging.cpp
    src/spacetime_module_abi.cpp
    src/spacetime_reducer_bridge.cpp
    src/spacetimedb_sdk_table_registry.cpp
    src/spacetimedb_sdk_types.cpp
    src/wasi_shims.cpp
)

# Create the SDK library
add_library(spacetimedb_cpp_sdk STATIC ${SDK_SOURCES})

# Set include directories
target_include_directories(spacetimedb_cpp_sdk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/include>
        $<INSTALL_INTERFACE:include>
)

# Create an alias target for better namespacing
add_library(spacetimedb::sdk::spacetimedb_cpp_sdk ALIAS spacetimedb_cpp_sdk)

# Set compile options if building for WASM
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_options(spacetimedb_cpp_sdk PRIVATE -O2 -Wall -Wextra)
endif()

# Export compile commands for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)