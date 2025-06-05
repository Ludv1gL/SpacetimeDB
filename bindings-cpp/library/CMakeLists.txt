# SpacetimeDB C++ Module Library CMake Configuration

cmake_minimum_required(VERSION 3.15)
project(SpacetimeDBCppModuleLibrary CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Module Library source files
set(LIBRARY_SOURCES
    src/abi/module_exports.cpp
    src/bsatn/reader.cpp
    src/bsatn/writer.cpp
    src/credentials.cpp
    src/database.cpp
    src/library/logging.cpp
    src/spacetimedb_library_table_registry.cpp
    src/spacetimedb_library_types.cpp
    src/wasi_shims.cpp
    src/internal/Module.cpp
    src/internal/FFI.cpp
    src/transaction.cpp
    src/versioning.cpp
    src/timestamp.cpp
    src/time_duration.cpp
    src/constraint_validation.cpp
    src/field_registration.cpp
    src/library/exceptions.cpp
    src/library/index_management.cpp
)

# Create the Module Library
add_library(spacetimedb_cpp_library STATIC ${LIBRARY_SOURCES})

# Set include directories
target_include_directories(spacetimedb_cpp_library
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/include>
        $<INSTALL_INTERFACE:include>
)

# Create an alias target for better namespacing
add_library(spacetimedb::spacetimedb_cpp_library ALIAS spacetimedb_cpp_library)

# Set compile options if building for WASM
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_options(spacetimedb_cpp_library PRIVATE -O2 -Wall -Wextra)
endif()

# Export compile commands for better IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)