#ifndef SPACETIME_TEST_COMMON_H
#define SPACETIME_TEST_COMMON_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iomanip> // For print_bytes

// Basic Assertion Macro
#define ASSERT_CONDITION(condition, message) \
    if (!(condition)) { \
        std::cerr << "Assertion Failed: (" #condition ") - Message: " << (message) \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("Assertion failed: " + std::string(message)); \
    }

#define ASSERT_TRUE(condition, message) ASSERT_CONDITION(condition, message)
#define ASSERT_FALSE(condition, message) ASSERT_CONDITION(!(condition), message)
#define ASSERT_EQ(val1, val2, message) ASSERT_CONDITION((val1) == (val2), message)
#define ASSERT_NE(val1, val2, message) ASSERT_CONDITION((val1) != (val2), message)
#define ASSERT_NULL(ptr, message) ASSERT_CONDITION((ptr) == nullptr, message)
#define ASSERT_NOT_NULL(ptr, message) ASSERT_CONDITION((ptr) != nullptr, message)


// Helper to print byte vectors for debugging
inline void print_bytes_test(const std::vector<std::byte>& bytes, const std::string& prefix = "") {
    if (!prefix.empty()) {
        std::cout << prefix;
    }
    std::cout << std::hex << std::setfill('0');
    for (std::byte b : bytes) {
        std::cout << std::setw(2) << static_cast<uint32_t>(b) << " ";
    }
    std::cout << std::dec << " (Size: " << bytes.size() << ")" << std::endl;
}


// Stubs for SpacetimeDB Host ABI functions
// These allow testing SDK runtime functions without a live host.
extern "C" {

static std::vector<std::string> g_host_log_messages; // Global for tests to inspect logs

inline void spacetimedb_host_log_message(const char* message_ptr, uint32_t message_len, uint8_t level) {
    std::string level_str = "UNKNOWN_LVL_" + std::to_string(level);
    switch(static_cast<SpacetimeDB::LogLevel>(level)) { // Assuming SpacetimeDB::LogLevel is accessible or we map ints
        case SpacetimeDB::LogLevel::Error: level_str = "ERROR"; break;
        case SpacetimeDB::LogLevel::Warn:  level_str = "WARN";  break;
        case SpacetimeDB::LogLevel::Info:  level_str = "INFO";  break;
        case SpacetimeDB::LogLevel::Debug: level_str = "DEBUG"; break;
        case SpacetimeDB::LogLevel::Trace: level_str = "TRACE"; break;
    }
    std::string message(message_ptr, message_len);
    std::string full_log = "[HOST STUB LOG (" + level_str + ")] " + message;
    std::cout << full_log << std::endl;
    g_host_log_messages.push_back(full_log);
}

// Simulate simple table operations by logging them.
// A more advanced mock could use in-memory maps to simulate table state.
static std::vector<std::string> g_host_table_ops_log;

inline int32_t spacetimedb_host_table_insert(const char* table_name_ptr, uint32_t table_name_len,
                                          const unsigned char* row_data_ptr, uint32_t row_data_len) {
    std::string table_name(table_name_ptr, table_name_len);
    std::string log_entry = "INSERT Table: " + table_name + ", DataLen: " + std::to_string(row_data_len);
    std::cout << "[HOST STUB] " << log_entry << std::endl;
    g_host_table_ops_log.push_back(log_entry);
    // print_bytes_test(std::vector<std::byte>(reinterpret_cast<const std::byte*>(row_data_ptr), reinterpret_cast<const std::byte*>(row_data_ptr + row_data_len)), "[HOST STUB] Insert Row Data: ");
    return 0; // Simulate success
}

inline int32_t spacetimedb_host_table_delete_by_pk(const char* table_name_ptr, uint32_t table_name_len,
                                                const unsigned char* pk_data_ptr, uint32_t pk_data_len) {
    std::string table_name(table_name_ptr, table_name_len);
    std::string log_entry = "DELETE_BY_PK Table: " + table_name + ", PKLen: " + std::to_string(pk_data_len);
    std::cout << "[HOST STUB] " << log_entry << std::endl;
    g_host_table_ops_log.push_back(log_entry);
    // print_bytes_test(std::vector<std::byte>(reinterpret_cast<const std::byte*>(pk_data_ptr), reinterpret_cast<const std::byte*>(pk_data_ptr + pk_data_len)), "[HOST STUB] Delete PK Data: ");
    return 0; // Simulate success (row found and deleted)
}

// Forward declaration for the reducer dispatcher, defined in spacetime_reducer_bridge.cpp
// This is needed if tests in sdk_unit_tests.cpp want to call it directly.
// It should also be SPACETIMEDB_WASM_EXPORT if that macro is defined here.
#ifndef SPACETIMEDB_WASM_EXPORT
#define SPACETIMEDB_WASM_EXPORT
#endif
SPACETIMEDB_WASM_EXPORT void _spacetimedb_dispatch_reducer(const char* reducer_name_ptr,
                                   uint32_t reducer_name_len,
                                   const unsigned char* args_data_ptr,
                                   uint32_t args_data_len);

} // extern "C"

#endif // SPACETIME_TEST_COMMON_H
