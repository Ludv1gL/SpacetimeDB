cmake_minimum_required(VERSION 3.13)
project(SpacetimeDBCppCounterModule LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Emscripten Specific Setup ---
# This example assumes Emscripten is used to compile to Wasm.
# The Emscripten toolchain file should be set via CMAKE_TOOLCHAIN_FILE, e.g.,
# cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/emscripten/cmake/Modules/Platform/Emscripten.cmake ...

if(NOT CMAKE_CROSSCOMPILING_EMULATOR)
    # Try to find Emscripten. This is just a basic check.
    # A robust project would use find_package(Emscripten) if available or ensure toolchain is set.
    if(NOT DEFINED ENV{EMSDK} AND NOT EMSCRIPTEN_TOOLCHAIN_PATH)
        message(WARNING "Emscripten SDK environment (EMSDK) or toolchain path not detected. "
                        "Please ensure Emscripten is configured for Wasm compilation.")
    endif()
endif()

# --- SDK Source Files ---
# In a real SDK setup, these would be compiled into a static library that the module links against.
# For this example, we list the source files directly.
# Adjust these paths based on your SDK's directory structure.
# Assuming this example is in examples/cpp_counter_module and SDK is in sdk/cpp:
set(SDK_SRC_DIR "../../../sdk/cpp/src") # Adjust if your SDK path is different
set(SDK_INCLUDE_DIR "../../../sdk/cpp/include") # Adjust for headers

# List of SDK implementation files needed by the module
set(SDK_SOURCE_FILES
    "${SDK_SRC_DIR}/bsatn_reader.cpp"
    "${SDK_SRC_DIR}/bsatn_writer.cpp"
    "${SDK_SRC_DIR}/module_def_builder.cpp"       # For get_serialized_module_definition_bytes
    "${SDK_SRC_DIR}/spacetime_module_abi.cpp"     # For get_spacetimedb_module_def_data/size
    "${SDK_SRC_DIR}/spacetime_reducer_bridge.cpp" # For _spacetimedb_dispatch_reducer
    "${SDK_SRC_DIR}/spacetime_sdk_runtime.cpp"    # For SpacetimeDB::log_info etc.
)

# --- Module Definition ---
add_executable(counter_module_wasm counter_module.cpp ${SDK_SOURCE_FILES})

# --- Include Directories ---
# Add directory containing SDK headers (spacetime_macros.h, spacetime_schema.h, etc.)
# and BSATN headers (bsatn_reader.h, bsatn_writer.h, uint128_placeholder.h)
target_include_directories(counter_module_wasm PUBLIC
    "${SDK_INCLUDE_DIR}"
    # If uint128_placeholder.h is in a different common location:
    # "../../../sdk/cpp/common_include"
)

# --- Wasm Compilation and Linking Options ---
set_target_properties(counter_module_wasm PROPERTIES SUFFIX ".wasm")

# For Emscripten:
# The SPACETIMEDB_WASM_EXPORT macro (using EMSCRIPTEN_KEEPALIVE) in the SDK headers
# (spacetime_module_abi.h) should handle exporting the necessary ABI functions:
# - get_spacetimedb_module_def_data
# - get_spacetimedb_module_def_size
# - _spacetimedb_dispatch_reducer
#
# If EMSCRIPTEN_KEEPALIVE is not used or for extra robustness, you can use EXPORTED_FUNCTIONS.
# Note the leading underscores for C functions when listed this way.
# Emscripten also has EXPORT_ES6 for different module types.
# This example relies on EMSCRIPTEN_KEEPALIVE.
#
# Example of EXPORTED_FUNCTIONS (if not using KEEPALIVE directly in C++ code):
# target_link_options(counter_module_wasm PRIVATE
#    "-sEXPORTED_FUNCTIONS=['_get_spacetimedb_module_def_data','_get_spacetimedb_module_def_size','__spacetimedb_dispatch_reducer']"
# )
#
# Other common Emscripten flags:
# -sWASM=1 (usually default)
# -sALLOW_MEMORY_GROWTH=1 (if your module needs it)
# -sSTANDALONE_WASM (if not needing JS runtime integration beyond imports/exports)
# -O2 (for optimization)

target_link_options(counter_module_wasm PRIVATE
    # Add any necessary Emscripten flags here, e.g., for optimization or memory growth.
    # "-sALLOW_MEMORY_GROWTH=1"
    # "-O2"
)

message(STATUS "SpacetimeDB C++ Counter Module (for Wasm)")
message(STATUS "  SDK Sources: ${SDK_SRC_DIR}")
message(STATUS "  SDK Includes: ${SDK_INCLUDE_DIR}")
message(STATUS "  Target: counter_module.wasm")
message(STATUS "  To build, ensure Emscripten toolchain is active and run your build command (e.g., make or ninja).")
message(STATUS "  Example CMake configure: cmake -DCMAKE_TOOLCHAIN_FILE=<path_to_emscripten>/cmake/Modules/Platform/Emscripten.cmake .")

# Note: This CMakeLists.txt assumes the SDK files (headers and sources) are present
# at the specified relative paths. In a real distribution, the SDK would typically be
# installed to system paths or included as a subdirectory/FetchContent.
# The source files listed in SDK_SOURCE_FILES are those created in previous steps.
# Ensure these paths are correct for your project structure.
# For example, if this example is part of the same repo as the SDK, paths might be like:
# set(SDK_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../src")
# set(SDK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
# This example assumes a structure like:
# root/
#   sdk/
#     cpp/
#       include/ (*.h files from previous steps)
#       src/     (*.cpp files from previous steps)
#   examples/
#     cpp_counter_module/
#       CMakeLists.txt (this file)
#       counter_module.cpp
