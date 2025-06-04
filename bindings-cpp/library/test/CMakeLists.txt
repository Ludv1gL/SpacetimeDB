cmake_minimum_required(VERSION 3.16)
project(spacetimedb_bsatn_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# BSATN library sources
set(BSATN_SOURCES
    ../src/bsatn/reader.cpp
    ../src/bsatn/writer.cpp
    ../src/spacetimedb_library_types.cpp
    ../src/spacetimedb_library_types_impl.cpp
)

# Main test executable
add_executable(test_bsatn
    test_bsatn.cpp
    ${BSATN_SOURCES}
)

# Refactored implementation test
add_executable(test_bsatn_refactored
    test_bsatn_refactored.cpp
    ${BSATN_SOURCES}
)

# Existing implementation test
add_executable(test_bsatn_existing
    test_bsatn_existing.cpp
    ${BSATN_SOURCES}
)

# Enable testing
enable_testing()

# Add test
add_test(NAME bsatn_tests COMMAND test_bsatn)
add_test(NAME bsatn_refactored_tests COMMAND test_bsatn_refactored)
add_test(NAME bsatn_existing_tests COMMAND test_bsatn_existing)

# Add verbose test variant
add_test(NAME bsatn_tests_verbose COMMAND test_bsatn -v)

# Set test properties
set_tests_properties(bsatn_tests PROPERTIES
    LABELS "unit"
    TIMEOUT 30
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(test_bsatn PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
    )
    target_compile_options(test_bsatn_refactored PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
    )
    target_compile_options(test_bsatn_existing PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
        -Wno-unused-variable
    )
endif()

# Summary
message(STATUS "BSATN Tests configured")
message(STATUS "  Build with: cmake --build .")
message(STATUS "  Run tests with: ctest")
message(STATUS "  Run verbose: ctest -V")