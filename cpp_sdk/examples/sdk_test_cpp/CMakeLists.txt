cmake_minimum_required(VERSION 3.16)
project(SdkTestCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Assuming SpacetimeDB SDK provides an include directory and a library target
# These names are illustrative and might need adjustment based on actual SDK structure
# find_package(SpacetimeDB REQUIRED) # If SDK installs as a CMake package

# If using headers directly from the monorepo:
# Adjust relative path as necessary
include_directories(../../../sdk/include)

add_executable(sdk_test_cpp src/sdk_test.cpp src/sdk_test.h)
add_executable(minimal_sdk_test src/minimal_sdk_test.cpp)

# Link against SpacetimeDB module library (illustrative name)
# target_link_libraries(sdk_test_cpp PRIVATE SpacetimeDB::Module)

# Add other necessary flags for WASM compilation if applicable at this stage
# e.g., set(CMAKE_SYSTEM_NAME Emscripten) if building for Emscripten separately
