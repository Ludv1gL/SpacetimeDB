#include <iostream>
#include <stdexcept> // For std::exception

// Forward declaration for the main test execution function
// This function will be defined in sdk_unit_tests.cpp
void run_all_unit_tests();

// Minimal stubs for SpacetimeDb::ModuleSchema and related types if not fully included elsewhere.
// These are needed if any SDK headers included by sdk_unit_tests.cpp indirectly expect them
// via spacetime_macros.h, and if sdk_unit_tests.cpp doesn't provide them itself.
// Ideally, test_common.h or sdk_unit_tests.cpp would handle all necessary stubs or includes.
#ifndef SPACETIME_SCHEMA_H
#define SPACETIME_SCHEMA_H
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cstdint>
#include <functional> // For std::function (used in ReducerDefinition)

// Forward declare bsatn::Reader for ReducerDefinition invoker
namespace bsatn { class Reader; }

namespace SpacetimeDb {
    enum class CoreType : uint8_t { Bool=0, U8=1, U16=2, U32=3, U64=4, U128=5, I8=7, I16=8, I32=9, I64=10, I128=11, F32=12, F64=13, String=14, Bytes=15, UserDefined=16 };
    struct TypeIdentifier { CoreType core_type; std::string user_defined_name; };
    struct FieldDefinition { std::string name; TypeIdentifier type; bool is_optional = false;};
    struct EnumVariantDefinition { std::string name; };
    struct ReducerParameterDefinition { std::string name; TypeIdentifier type; };

    struct TypeDefinition {};
    struct TableDefinition {};
    // struct ReducerDefinition {}; // Full definition needed if schema is populated here
     struct ReducerDefinition {
        std::string spacetime_name;
        std::string cpp_function_name;
        std::vector<ReducerParameterDefinition> parameters;
        std::function<void(bsatn::Reader&)> invoker;
    };


    class ModuleSchema {
    public:
        std::map<std::string, TypeDefinition> types;
        std::map<std::string, TableDefinition> tables;
        std::map<std::string, ReducerDefinition> reducers;

        void register_struct_type(const std::string&, const std::string&, const std::vector<FieldDefinition>&) {}
        void register_enum_type(const std::string&, const std::string&, const std::vector<EnumVariantDefinition>&) {}
        void register_table(const std::string& , const std::string& ) {}
        void set_primary_key(const std::string& , const std::string& ) {}
        void register_reducer(const std::string& name, const std::string& , const std::vector<ReducerParameterDefinition>&, std::function<void(bsatn::Reader&)> invoker_fn) {
            // Basic stub for reducer registration for test purposes if macros are used by test_types.h
             ReducerDefinition def; def.spacetime_name = name; def.invoker = invoker_fn; reducers[name] = def;
        }
        static ModuleSchema& instance() {
            static ModuleSchema schema;
            return schema;
        }
    private:
        ModuleSchema() = default;
    };
} // namespace SpacetimeDb
#endif // SPACETIME_SCHEMA_H


int main() {
    try {
        run_bsatn_tests();
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception." << std::endl;
        return 1;
    }
    return 0;
}

// Conceptual ABI test (actual ABI functions are in spacetime_module_abi.cpp)
// This part of main is for basic check that symbols link and return something.
// It's not a Wasm runtime test.
// NOTE: The actual content of module_data will depend on types registered by spacetime_test_types.h
// via the macros, as g_serialized_module_def_buffer in spacetime_module_abi.cpp
// is populated by get_serialized_module_definition_bytes(), which uses ModuleSchema::instance().
int main_abi_test_placeholder() {
    std::cout << "\nTesting Module ABI functions (conceptual check)..." << std::endl;
    const unsigned char* module_data = get_spacetimedb_module_def_data();
    int module_size = get_spacetimedb_module_def_size();

    std::cout << "Module Def Size: " << module_size << " bytes." << std::endl;
    if (module_size > 0 && module_data != nullptr) {
        std::cout << "Module Def Data (first few bytes): ";
        int N = std::min(module_size, 16); // Print up to 16 bytes
        std::cout << std::hex << std::setfill('0');
        for (int i = 0; i < N; ++i) {
            std::cout << std::setw(2) << static_cast<unsigned int>(module_data[i]) << " ";
        }
        std::cout << std::dec << std::endl;
        std::cout << "Module ABI functions seem callable." << std::endl;
    } else if (module_size == 0) {
        std::cout << "Module Def is empty (0 bytes)." << std::endl;
        std::cout << "This is expected if test types in spacetime_test_types.h don't register "
                  << "any schema elements OR if the linking of spacetime_module_abi.cpp "
                  << "is not correctly using the same ModuleSchema instance." << std::endl;
    } else {
        std::cerr << "Error: Module data state is inconsistent (size=" << module_size << ")." << std::endl;
        return 1;
    }
    return 0;
}

// Modify main to call both tests
int main() {
    int test_result = 0;
    try {
        run_bsatn_tests();
        // Call the ABI test placeholder function after bsatn tests
        // to ensure ModuleSchema might have been populated by macros in spacetime_test_types.h
        if (main_abi_test_placeholder() != 0) {
            test_result = 1; // Indicate ABI test conceptual check had an issue
        }
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        test_result = 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception." << std::endl;
        test_result = 1;
    }

    if (test_result == 0) {
        std::cout << "\nAll tests and conceptual checks finished." << std::endl;
    } else {
        std::cout << "\nSome tests or conceptual checks failed." << std::endl;
    }
    return test_result;
}

// Stubs for SpacetimeDB Host ABI functions (for linking and basic testing)
extern "C" {

void spacetimedb_host_log_message(const char* message_ptr, uint32_t message_len, uint8_t level) {
    std::string level_str = "UNKNOWN";
    switch(level) {
        case 0: level_str = "ERROR"; break;
        case 1: level_str = "WARN"; break;
        case 2: level_str = "INFO"; break;
        case 3: level_str = "DEBUG"; break;
        case 4: level_str = "TRACE"; break;
    }
    std::cout << "[HOST STUB LOG (" << level_str << ")] " << std::string(message_ptr, message_len) << std::endl;
}

int32_t spacetimedb_host_table_insert(const char* table_name_ptr, uint32_t table_name_len,
                                   const unsigned char* row_data_ptr, uint32_t row_data_len) {
    std::cout << "[HOST STUB TABLE_INSERT] Table: " << std::string(table_name_ptr, table_name_len)
              << ", DataLen: " << row_data_len << std::endl;
    // Simulate success
    return 0;
}

int32_t spacetimedb_host_table_delete_by_pk(const char* table_name_ptr, uint32_t table_name_len,
                                         const unsigned char* pk_data_ptr, uint32_t pk_data_len) {
    std::cout << "[HOST STUB TABLE_DELETE_BY_PK] Table: " << std::string(table_name_ptr, table_name_len)
              << ", PKLen: " << pk_data_len << std::endl;
    // Simulate row found and deleted
    return 0;
}

// Forward declaration from spacetime_reducer_bridge.cpp if not in a common header for main
// For testing purposes, ensure its signature is known.
// extern void _spacetimedb_dispatch_reducer(const char* name_ptr, uint32_t name_len, const unsigned char* args_ptr, uint32_t args_len);

} // extern "C"


// --- Test Reducer Dispatch ---
static int test_reducer_sum_result = 0; // Global to check side effect

void test_reducer_add_impl(int32_t a, int32_t b) {
    std::cout << "test_reducer_add_impl called with a=" << a << ", b=" << b << std::endl;
    test_reducer_sum_result = a + b;
}
// Register the reducer
// Need to make sure SpacetimeDb::CoreType and ReducerParameterDefinition are available or stubbed if this file doesn't include spacetime_schema.h fully
// The stub for ModuleSchema in main.cpp already has ReducerParameterDefinition.
SPACETIMEDB_REDUCER("add", test_reducer_add_impl,
    { SPACETIMEDB_REDUCER_PARAM("a", SpacetimeDb::CoreType::I32),
      SPACETIMEDB_REDUCER_PARAM("b", SpacetimeDb::CoreType::I32) },
    int32_t, int32_t // C++ types for invoker generation
);

void run_reducer_dispatch_tests() {
    std::cout << "\nRunning Reducer Dispatch Tests..." << std::endl;
    test_reducer_sum_result = 0; // Reset

    bsatn::Writer args_writer;
    // SpacetimeDB::bsatn::serialize(args_writer, static_cast<int32_t>(123)); // This serialize is not in SpacetimeDB::bsatn for primitives
    // SpacetimeDB::bsatn::serialize(args_writer, static_cast<int32_t>(456));
    // Need to use the global bsatn::serialize or writer methods directly.
    // The test_types.h added SpacetimeDB::bsatn::serialize for int32_t
    SpacetimeDB::bsatn::serialize(args_writer, static_cast<int32_t>(123));
    SpacetimeDB::bsatn::serialize(args_writer, static_cast<int32_t>(456));

    std::vector<std::byte> args_bytes = args_writer.take_buffer();

    std::string reducer_name = "add";
    _spacetimedb_dispatch_reducer(
        reducer_name.c_str(), static_cast<uint32_t>(reducer_name.length()),
        reinterpret_cast<const unsigned char*>(args_bytes.data()), static_cast<uint32_t>(args_bytes.size())
    );

    assert(test_reducer_sum_result == (123 + 456));
    std::cout << "Reducer 'add' dispatch and execution: SUCCESS (Result: " << test_reducer_sum_result << ")" << std::endl;

    // Test SDK runtime wrappers (which call host stubs)
    SpacetimeDB::log_info("This is an info log from SDK wrapper.");
    SpacetimeDB::log_error("This is an error log from SDK wrapper.");

    TestNestedStruct row_to_insert; // From spacetime_test_types.h
    row_to_insert.nested_id = 789;
    row_to_insert.description = "Row via SDK wrapper";
    bool insert_ok = SpacetimeDB::table_insert("TestNestedStructTable", row_to_insert);
    assert(insert_ok);
    std::cout << "SDK SpacetimeDB::table_insert call: SUCCESS" << std::endl;

    // Assuming PK for TestNestedStructTable is nested_id (uint32_t) for this test
    uint32_t pk_to_delete = 789;
    bool delete_ok = SpacetimeDB::table_delete_by_pk("TestNestedStructTable", pk_to_delete);
    assert(delete_ok);
    std::cout << "SDK SpacetimeDB::table_delete_by_pk call: SUCCESS" << std::endl;


    std::cout << "Reducer dispatch and SDK runtime wrapper tests finished." << std::endl;
}

// Modify main to call all tests
int main() {
    int test_result = 0;
    try {
        run_bsatn_tests(); // From spacetime_test_types.h

        // Ensure ModuleSchema is populated by macros in spacetime_test_types.h before ABI test
        // Then run ABI conceptual check
        if (main_abi_test_placeholder() != 0) {
            test_result = 1;
        }
        // Run reducer dispatch tests (which also registers a reducer)
        run_reducer_dispatch_tests();

    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        test_result = 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception." << std::endl;
        test_result = 1;
    }

    if (test_result == 0) {
        std::cout << "\nAll tests and conceptual checks finished successfully." << std::endl;
    } else {
        std::cout << "\nSome tests or conceptual checks failed." << std::endl;
    }
    return test_result;
}
