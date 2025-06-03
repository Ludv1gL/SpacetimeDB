cmake_minimum_required(VERSION 3.10)
project(module_test)

# Set the output to a .wasm file
set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

# Add the module executable
add_executable(module_test src/module_test.cpp)

# Include directories
target_include_directories(module_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/include
)

# Set compilation flags for WebAssembly
target_compile_options(module_test PRIVATE
    -std=c++20
    -fno-exceptions
    -fno-rtti
    -Wall
    -Wextra
)

# Set linker flags for WebAssembly
target_link_options(module_test PRIVATE
    -nostdlib
    -Wl,--no-entry
    -Wl,--export-dynamic
    -Wl,--allow-undefined
)