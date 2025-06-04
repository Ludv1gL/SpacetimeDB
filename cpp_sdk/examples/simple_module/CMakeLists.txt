cmake_minimum_required(VERSION 3.15)
project(simple_module CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the SDK
add_subdirectory(../../sdk sdk_build)

# Create the module
add_executable(simple_module simple_module.cpp)

# Link against the SDK
target_link_libraries(simple_module PRIVATE spacetimedb_cpp_sdk)

# Set compile options for WASM
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set_target_properties(simple_module PROPERTIES
        SUFFIX ".wasm"
        LINK_FLAGS "-s STANDALONE_WASM=1 -s FILESYSTEM=0 -s DISABLE_EXCEPTION_CATCHING=1 -O2 -Wl,--no-entry"
    )
endif()