#!/bin/bash

# Test script for C++ SDK test module

set -e

echo "=== Building C++ SDK test module ==="

# Compile the module
emcc -std=c++20 \
    src/sdk_test.cpp \
    -s STANDALONE_WASM=1 \
    -s FILESYSTEM=0 \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -O2 \
    -Wl,--no-entry \
    -o sdk_test.wasm

echo "✓ Module compiled successfully"

# Publish the module
echo "=== Publishing module to SpacetimeDB ==="
spacetime publish --bin-path sdk_test.wasm cpp-sdk-test

# Get the module description
echo "=== Module description ==="
spacetime describe cpp-sdk-test --json > cpp_sdk_test_description.json
echo "✓ Module description saved to cpp_sdk_test_description.json"

# Run some basic tests
echo "=== Running basic tests ==="

# Test no-op reducer
echo "Testing no_op_succeeds reducer..."
if spacetime call cpp-sdk-test no_op_succeeds 2>&1 | grep -q "Error"; then
    echo "✗ no_op_succeeds failed"
else
    echo "✓ no_op_succeeds succeeded"
fi

# Test insert operations
echo "Testing insert_one_u8 reducer..."
if spacetime call cpp-sdk-test insert_one_u8 2>&1 | grep -q "Error"; then
    echo "✗ insert_one_u8 failed"
else
    echo "✓ insert_one_u8 succeeded"
fi

# Test identity insertion
echo "Testing insert_caller_one_identity reducer..."
if spacetime call cpp-sdk-test insert_caller_one_identity 2>&1 | grep -q "Error"; then
    echo "✗ insert_caller_one_identity failed"
else
    echo "✓ insert_caller_one_identity succeeded"
fi

# Test timestamp insertion
echo "Testing insert_call_timestamp reducer..."
if spacetime call cpp-sdk-test insert_call_timestamp 2>&1 | grep -q "Error"; then
    echo "✗ insert_call_timestamp failed"
else
    echo "✓ insert_call_timestamp succeeded"
fi

# Check logs
echo "=== Module logs ==="
spacetime logs cpp-sdk-test -n 20

echo "=== Test complete ==="