#!/bin/bash
# Test compilation of all C++ SDK example modules

set -e

echo "========================================"
echo "C++ SDK Module Compilation Test Suite"
echo "========================================"

# Find emcc
EMCC=$(which emcc || echo "/home/ludvi/emsdk/upstream/emscripten/emcc")

if [ ! -x "$EMCC" ]; then
    echo "Error: emcc not found. Please ensure Emscripten is installed and in PATH"
    exit 1
fi

# Track results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
FAILED_MODULES=()

# Function to test module compilation
test_module() {
    local module_name=$1
    local source_file=$2
    local description=$3
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    echo ""
    echo "Testing: $description"
    echo "  Source: $source_file"
    
    if [ ! -f "$source_file" ]; then
        echo "  ✗ SKIPPED: Source file not found"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        FAILED_MODULES+=("$module_name (file not found)")
        return
    fi
    
    # Try to compile
    if $EMCC -std=c++20 \
        "$source_file" \
        -I../../sdk/include \
        -s STANDALONE_WASM=1 \
        -s FILESYSTEM=0 \
        -s DISABLE_EXCEPTION_CATCHING=1 \
        -O2 \
        -Wl,--no-entry \
        -o "test_${module_name}.wasm" 2>test_error.log; then
        
        echo "  ✓ PASSED: Compilation successful"
        echo "    Output: test_${module_name}.wasm ($(du -h test_${module_name}.wasm | cut -f1))"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        rm -f test_error.log
    else
        echo "  ✗ FAILED: Compilation error"
        echo "    Error log saved to: test_error_${module_name}.log"
        mv test_error.log "test_error_${module_name}.log"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        FAILED_MODULES+=("$module_name")
    fi
}

# Test all modules
echo ""
echo "Testing module compilations..."
echo "=============================="

# Basic modules
test_module "minimal" "minimal_module.cpp" "Minimal module with basic table"
test_module "simple" "simple_module.cpp" "Simple module with reducers"
test_module "simple_no_strings" "simple_no_strings.cpp" "Module without string types"
test_module "simple_table" "simple_table.cpp" "Basic table operations"
test_module "working" "working_module.cpp" "Working module example"
test_module "working_simple" "working_simple_module.cpp" "Working simple module"
test_module "improved" "improved_module.cpp" "Improved module with features"

# Feature-specific modules
test_module "builtin_reducers" "builtin_reducers_example.cpp" "Built-in reducer implementations"
test_module "context_demo" "context_demo.cpp" "Enhanced reducer context features"
test_module "credentials" "credential_example.cpp" "Credential management"
test_module "constraints" "constraint_validation_example.cpp" "Constraint validation"
test_module "foreign_keys" "foreign_key_validation.cpp" "Foreign key constraints"
test_module "transactions" "transaction_example.cpp" "Transaction support"
test_module "scheduled" "scheduled_example.cpp" "Scheduled reducers"
test_module "versioning" "versioning_test.cpp" "Table versioning"
test_module "versioned_v1" "versioned_module_v1.cpp" "Versioned module v1"
test_module "versioned_v2" "versioned_module_v2.cpp" "Versioned module v2"
test_module "version_compat" "version_compatibility_example.cpp" "Version compatibility"
test_module "advanced_features" "advanced_features_demo.cpp" "Advanced SDK features"

# Test modules
test_module "test_encoding" "test_encoding.cpp" "BSATN encoding tests"
test_module "test_table_ops" "test_table_ops.cpp" "Table operation tests"
test_module "test_iter_ops" "test_iter_ops.cpp" "Iterator operation tests"
test_module "test_builtin" "test_builtin_reducers.cpp" "Built-in reducer tests"
test_module "test_credentials" "test_credentials.cpp" "Credential tests"
test_module "test_schedule" "test_schedule_types.cpp" "Schedule type tests"
test_module "test_transactions" "test_transactions.cpp" "Transaction tests"

# Comprehensive test
test_module "comprehensive" "comprehensive_integration_test.cpp" "Comprehensive integration test"

# Summary
echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo "Total tests:  $TOTAL_TESTS"
echo "Passed:       $PASSED_TESTS"
echo "Failed:       $FAILED_TESTS"
echo ""

if [ $FAILED_TESTS -gt 0 ]; then
    echo "Failed modules:"
    for module in "${FAILED_MODULES[@]}"; do
        echo "  - $module"
    done
    echo ""
    echo "Check test_error_*.log files for details"
else
    echo "All modules compiled successfully!"
fi

echo ""
echo "Cleaning up test WASM files..."
rm -f test_*.wasm

echo "Done!"
exit $FAILED_TESTS