cmake_minimum_required(VERSION 3.20)
project(spacetimedb-module)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the SpacetimeDB root directory by looking for a marker file
# This works regardless of where this CMakeLists.txt is placed
function(find_spacetimedb_root result)
    set(current_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    while(NOT EXISTS "${current_dir}/Cargo.toml" OR NOT EXISTS "${current_dir}/bindings-cpp")
        get_filename_component(parent_dir "${current_dir}" DIRECTORY)
        if("${parent_dir}" STREQUAL "${current_dir}")
            message(FATAL_ERROR "Could not find SpacetimeDB root directory")
        endif()
        set(current_dir "${parent_dir}")
    endwhile()
    set(${result} "${current_dir}" PARENT_SCOPE)
endfunction()

find_spacetimedb_root(SPACETIMEDB_ROOT)
message(STATUS "Found SpacetimeDB root: ${SPACETIMEDB_ROOT}")

# Include the Module Library using its own CMakeLists.txt
set(MODULE_LIBRARY_DIR "${SPACETIMEDB_ROOT}/bindings-cpp/library")
add_subdirectory(${MODULE_LIBRARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/library)

# Required parameter: MODULE_SOURCE - the source file to build
if(NOT DEFINED MODULE_SOURCE)
    message(FATAL_ERROR "MODULE_SOURCE must be defined. Usage: -DMODULE_SOURCE=path/to/source.cpp")
endif()

# Optional parameter: LINK_LIBRARY - whether to link the SpacetimeDB module library
if(NOT DEFINED LINK_LIBRARY)
    set(LINK_LIBRARY ON)
endif()

# Optional parameter: OUTPUT_NAME - name for the output WASM file (without extension)
if(NOT DEFINED OUTPUT_NAME)
    # Extract filename without extension from MODULE_SOURCE
    get_filename_component(OUTPUT_NAME ${MODULE_SOURCE} NAME_WE)
endif()

# Create the module executable
add_executable(module ${MODULE_SOURCE})

# Configure linking and includes based on LINK_LIBRARY
if(LINK_LIBRARY)
    target_link_libraries(module spacetimedb_cpp_library)
    message(STATUS "Building module WITH SpacetimeDB library")
else()
    # Still need access to headers for modules that include them
    target_include_directories(module PRIVATE ${MODULE_LIBRARY_DIR}/include)
    message(STATUS "Building module WITHOUT SpacetimeDB library (provides own exports)")
endif()

# Set output name
set_target_properties(module PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}")

# If using Emscripten, add appropriate flags
if(EMSCRIPTEN)
    # Base flags for all modules
    set(WASM_FLAGS "-s STANDALONE_WASM=1 -s EXPORTED_FUNCTIONS=['_malloc','_free'] -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s DISABLE_EXCEPTION_CATCHING=1 -s DISABLE_EXCEPTION_THROWING=1 -s WASM=1 --no-entry -s FILESYSTEM=0 -s INITIAL_MEMORY=16MB -s ALLOW_MEMORY_GROWTH=0 -s SUPPORT_LONGJMP=0 -s SUPPORT_ERRNO=0")
    
    # Add optimization for release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(WASM_FLAGS "${WASM_FLAGS} -O2")
    endif()
    
    set_target_properties(module PROPERTIES
        SUFFIX ".wasm"
        COMPILE_FLAGS ""
        LINK_FLAGS "${WASM_FLAGS}"
    )
endif()

# Print build configuration
message(STATUS "")
message(STATUS "SpacetimeDB Module Build Configuration:")
message(STATUS "  Source: ${MODULE_SOURCE}")
message(STATUS "  Output: ${OUTPUT_NAME}.wasm")
message(STATUS "  Link Library: ${LINK_LIBRARY}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "Usage examples:")
message(STATUS "  cmake -B build -DMODULE_SOURCE=src/my_module.cpp")
message(STATUS "  cmake -B build -DMODULE_SOURCE=src/my_module.cpp -DLINK_LIBRARY=OFF")
message(STATUS "  cmake -B build -DMODULE_SOURCE=src/my_module.cpp -DOUTPUT_NAME=custom_name")