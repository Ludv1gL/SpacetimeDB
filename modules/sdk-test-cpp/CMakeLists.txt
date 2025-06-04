cmake_minimum_required(VERSION 3.16)
project(sdk-test-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the SDK include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../cpp_sdk/sdk/include)

# Create the WASM module
add_executable(sdk-test-cpp
    src/lib.cpp
)

# Set output name
set_target_properties(sdk-test-cpp PROPERTIES OUTPUT_NAME "sdk_test_cpp")

# WASM-specific flags
if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
    target_compile_options(sdk-test-cpp PRIVATE
        -fno-exceptions
        -fno-rtti
    )
    
    target_link_options(sdk-test-cpp PRIVATE
        -Wl,--export-dynamic
        -Wl,--no-entry
        -Wl,--allow-undefined
    )
endif()