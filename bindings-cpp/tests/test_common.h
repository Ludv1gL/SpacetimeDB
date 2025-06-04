#ifndef SPACETIME_TEST_COMMON_H
#define SPACETIME_TEST_COMMON_H

#include <iostream>
#include <stdexcept> // For std::runtime_error
#include <string>
#include <vector>
#include <iomanip> // For print_bytes_test, if added here
#include <cstddef> // For std::byte

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


// Helper to print byte vectors for debugging (optional, can be in specific test files)
inline void print_bytes_test_common(const std::vector<std::byte>& bytes, const std::string& prefix = "") {
    if (!prefix.empty()) {
        std::cout << prefix;
    }
    std::cout << std::hex << std::setfill('0');
    for (std::byte b : bytes) {
        std::cout << std::setw(2) << static_cast<uint32_t>(b) << " ";
    }
    std::cout << std::dec << " (Size: " << bytes.size() << ")" << std::endl;
}

// Forward declare SpacetimeDB::LogLevel for host stubs, if not fully included
// This avoids pulling in full spacetime_sdk_runtime.h if only stubs are needed.
// It's better to include the actual definition to ensure type compatibility.
#include "spacetimedb/sdk/logging.h" // For SpacetimeDB::LogLevel (C++ enum wrapper)
#include "spacetimedb/abi/common_defs.h" // For ABI types like ::LogLevel, ::Status, ::BytesSink, ::BytesSource
#include "spacetimedb/abi/spacetimedb_abi.h" // For actual ABI function signatures to ensure stubs match

#include <map> // For mock sinks/sources
#include <algorithm> // For std::min

// --- Globals for test inspection ---
static std::vector<std::string> g_host_log_messages;
static std::vector<std::string> g_host_table_ops_log;

// Mock storage for BytesSinks and BytesSources
static uint16_t g_next_sink_source_id = 1; // Start from 1, 0 could be invalid handle
static std::map<uint16_t, std::vector<std::byte>> g_mock_sinks_data;
static std::map<uint16_t, std::vector<std::byte>> g_mock_sources_data;
static std::map<uint16_t, size_t> g_mock_sources_read_offset;


extern "C" {

// --- Logging ---
// Matches: void _log_message_abi(LogLevel level, const uint8_t* message_ptr, uint32_t message_len);
void _log_message_abi(::LogLevel level_abi, const uint8_t* message_ptr, uint32_t message_len) {
    SpacetimeDB::LogLevel level = static_cast<SpacetimeDB::LogLevel>(level_abi.inner); // Using C++ enum from sdk/logging.h
    std::string level_str = "UNKNOWN_LVL_" + std::to_string(level_abi.inner);
    switch(level) {
        case SpacetimeDB::LogLevel::Error: level_str = "ERROR"; break;
        case SpacetimeDB::LogLevel::Warn:  level_str = "WARN";  break;
        case SpacetimeDB::LogLevel::Info:  level_str = "INFO";  break;
        case SpacetimeDB::LogLevel::Debug: level_str = "DEBUG"; break;
        case SpacetimeDB::LogLevel::Trace: level_str = "TRACE"; break;
    }
    std::string message(reinterpret_cast<const char*>(message_ptr), message_len);
    std::string full_log = "[HOST STUB _log_message_abi (" + level_str + ")] " + message;
    std::cout << full_log << std::endl;
    g_host_log_messages.push_back(full_log);
}

// --- Table Operations ---
// Matches: Status table_insert(const uint8_t* table_name_ptr, uint32_t table_name_len,
//                             const uint8_t* row_data_ptr, uint32_t row_data_len);
Status table_insert(const uint8_t* table_name_ptr, uint32_t table_name_len,
                    const uint8_t* row_data_ptr, uint32_t row_data_len) {
    std::string table_name(reinterpret_cast<const char*>(table_name_ptr), table_name_len);
    std::string log_entry = "table_insert Table: " + table_name + ", DataLen: " + std::to_string(row_data_len);
    std::cout << "[HOST STUB] " << log_entry << std::endl;
    g_host_table_ops_log.push_back(log_entry);
    return {0}; // OK Status
}

// Matches: Status table_delete_by_pk(const uint8_t* table_name_ptr, uint32_t table_name_len,
//                                   const uint8_t* pk_data_ptr, uint32_t pk_data_len);
Status table_delete_by_pk(const uint8_t* table_name_ptr, uint32_t table_name_len,
                          const uint8_t* pk_data_ptr, uint32_t pk_data_len) {
    std::string table_name(reinterpret_cast<const char*>(table_name_ptr), table_name_len);
    std::string log_entry = "table_delete_by_pk Table: " + table_name + ", PKLen: " + std::to_string(pk_data_len);
    std::cout << "[HOST STUB] " << log_entry << std::endl;
    g_host_table_ops_log.push_back(log_entry);
    return {0}; // OK Status
}

// Minimal stub for _get_table_id if needed by any C++ wrappers under test
uint16_t _get_table_id(const uint8_t *name_ptr, size_t name_len, uint32_t *out_table_id_ptr) {
    std::string name(reinterpret_cast<const char*>(name_ptr), name_len);
    std::cout << "[HOST STUB _get_table_id] Name: " << name << std::endl;
    if (name == "Counters" || name == "TestNestedStructTable" || name == "AnotherTable" || name == "MyNestedTable") { // Add known test tables
        if (out_table_id_ptr) *out_table_id_ptr = (name == "Counters" ? 1 : (name == "TestNestedStructTable" ? 2 : 3) ); // Dummy table IDs
        return 0; // OK status
    }
    if (out_table_id_ptr) *out_table_id_ptr = 0; // Indicate not found
    return 1; // Error status (e.g., not found)
}


// --- BytesSink and BytesSource Stubs ---
BytesSink _bytes_sink_create() {
    uint16_t id = g_next_sink_source_id++;
    g_mock_sinks_data[id] = {};
    std::cout << "[HOST STUB _bytes_sink_create] ID: " << id << std::endl;
    return {id};
}

void _bytes_sink_done(BytesSink sink_handle) {
    std::cout << "[HOST STUB _bytes_sink_done] ID: " << sink_handle.inner << std::endl;
    // To properly test, data might be inspected here or moved from g_mock_sinks_data
    // For simplicity, we often inspect g_mock_sinks_data directly in tests.
    // g_mock_sinks_data.erase(sink_handle.inner); // Optional: clean up
}

Status _bytes_sink_write(BytesSink sink_handle, const uint8_t* data_ptr, uint32_t data_len) {
    std::cout << "[HOST STUB _bytes_sink_write] ID: " << sink_handle.inner << ", DataLen: " << data_len << std::endl;
    auto it = g_mock_sinks_data.find(sink_handle.inner);
    if (it == g_mock_sinks_data.end()) {
        std::cerr << "Error: Invalid BytesSink handle: " << sink_handle.inner << std::endl;
        return {1}; // Error: invalid sink handle
    }
    const std::byte* byte_ptr = reinterpret_cast<const std::byte*>(data_ptr);
    it->second.insert(it->second.end(), byte_ptr, byte_ptr + data_len);
    return {0}; // OK
}

uint32_t _bytes_sink_get_written_count(BytesSink sink_handle) {
    auto it = g_mock_sinks_data.find(sink_handle.inner);
    if (it != g_mock_sinks_data.end()) {
        uint32_t count = static_cast<uint32_t>(it->second.size());
        std::cout << "[HOST STUB _bytes_sink_get_written_count] ID: " << sink_handle.inner << ", Count: " << count << std::endl;
        return count;
    }
    std::cout << "[HOST STUB _bytes_sink_get_written_count] ID: " << sink_handle.inner << ", Error: Invalid Handle" << std::endl;
    return 0;
}

BytesSource _bytes_source_create_from_bytes(const uint8_t* data_ptr, uint32_t data_len) {
    uint16_t id = g_next_sink_source_id++;
    const std::byte* byte_ptr = reinterpret_cast<const std::byte*>(data_ptr);
    g_mock_sources_data[id] = std::vector<std::byte>(byte_ptr, byte_ptr + data_len);
    g_mock_sources_read_offset[id] = 0;
    std::cout << "[HOST STUB _bytes_source_create_from_bytes] ID: " << id << ", DataLen: " << data_len << std::endl;
    return {id};
}

BytesSource _bytes_source_create_from_sink_bytes(BytesSink sink_handle) {
    std::cout << "[HOST STUB _bytes_source_create_from_sink_bytes] From Sink ID: " << sink_handle.inner << std::endl;
    auto it_sink = g_mock_sinks_data.find(sink_handle.inner);
    if (it_sink == g_mock_sinks_data.end()) {
        std::cerr << "Error: Invalid BytesSink handle in _bytes_source_create_from_sink_bytes: " << sink_handle.inner << std::endl;
        return _bytes_source_create_from_bytes(nullptr, 0); // Return an empty source
    }
    const auto& sink_data = it_sink->second;
    return _bytes_source_create_from_bytes(reinterpret_cast<const uint8_t*>(sink_data.data()), sink_data.size());
}

void _bytes_source_done(BytesSource source_handle) {
    std::cout << "[HOST STUB _bytes_source_done] ID: " << source_handle.inner << std::endl;
    // g_mock_sources_data.erase(source_handle.inner); // Optional
    // g_mock_sources_read_offset.erase(source_handle.inner); // Optional
}

uint32_t _bytes_source_read(BytesSource source_handle, uint8_t* buffer_ptr, uint32_t buffer_len) {
    // std::cout << "[HOST STUB _bytes_source_read] ID: " << source_handle.inner << ", BufferLen: " << buffer_len << std::endl;
    auto it = g_mock_sources_data.find(source_handle.inner);
    if (it == g_mock_sources_data.end()) {
        std::cerr << "Error: Invalid BytesSource handle in _bytes_source_read: " << source_handle.inner << std::endl;
        return 0;
    }

    size_t& offset = g_mock_sources_read_offset[source_handle.inner];
    const std::vector<std::byte>& source_data = it->second;

    size_t remaining_in_source = source_data.size() - offset;
    uint32_t can_read = static_cast<uint32_t>(std::min((size_t)buffer_len, remaining_in_source));

    if (can_read > 0) {
        std::memcpy(buffer_ptr, source_data.data() + offset, can_read);
        offset += can_read;
    }
    // std::cout << "[HOST STUB _bytes_source_read] Read: " << can_read << " bytes." << std::endl;
    return can_read;
}

uint32_t _bytes_source_get_remaining_count(BytesSource source_handle) {
    auto it = g_mock_sources_data.find(source_handle.inner);
    if (it == g_mock_sources_data.end()) {
         std::cerr << "Error: Invalid BytesSource handle in _bytes_source_get_remaining_count: " << source_handle.inner << std::endl;
        return 0;
    }
    size_t offset = g_mock_sources_read_offset[source_handle.inner];
    uint32_t remaining = static_cast<uint32_t>(it->second.size() - offset);
    // std::cout << "[HOST STUB _bytes_source_get_remaining_count] ID: " << source_handle.inner << ", Remaining: " << remaining << std::endl;
    return remaining;
}


// Forward declarations for module exported functions (defined in spacetime_module_abi.cpp etc.)
// This ensures that tests can call these exported functions.
#ifndef SPACETIMEDB_WASM_EXPORT // Guard for the export macro
#define SPACETIMEDB_WASM_EXPORT
#endif

SPACETIMEDB_WASM_EXPORT void __describe_module__(BytesSink description_sink_handle);
SPACETIMEDB_WASM_EXPORT int16_t __call_reducer__(
    uint32_t reducer_id,
    uint64_t sender_identity_p0, uint64_t sender_identity_p1,
    uint64_t sender_identity_p2, uint64_t sender_identity_p3,
    uint64_t connection_id_p0, uint64_t connection_id_p1,
    uint64_t timestamp,
    BytesSource args_source_handle,
    BytesSink error_sink_handle
);

} // extern "C"

#endif // SPACETIME_TEST_COMMON_H
