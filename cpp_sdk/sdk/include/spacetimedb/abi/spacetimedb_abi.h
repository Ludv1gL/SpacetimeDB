#ifndef SPACETIMEDB_ABI_H
#define SPACETIMEDB_ABI_H

#include <cstdint>
#include <cstddef>

// This file provides the complete SpacetimeDB ABI interface
// All module imports and exports are defined here

// ===== IMPORT DECLARATIONS =====
// Define import attribute macro
#define STDB_IMPORT(name) \
    __attribute__((import_module("spacetime_10.0"), import_name(#name))) extern

extern "C" {

// ===== Table and Index Management =====
STDB_IMPORT(table_id_from_name)
uint16_t table_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* id);

STDB_IMPORT(index_id_from_name)
uint16_t index_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* id);

// ===== Table Operations =====
STDB_IMPORT(datastore_table_row_count)
uint16_t datastore_table_row_count(uint32_t table_id, uint64_t* count);

STDB_IMPORT(datastore_table_scan_bsatn)
uint16_t datastore_table_scan_bsatn(uint32_t table_id, uint32_t* iter);

// ===== Index Scanning =====
STDB_IMPORT(datastore_index_scan_range_bsatn)
uint16_t datastore_index_scan_range_bsatn(
    uint32_t index_id, const uint8_t* prefix, uint32_t prefix_len, uint16_t prefix_elems,
    const uint8_t* rstart, uint32_t rstart_len, const uint8_t* rend, uint32_t rend_len, 
    uint32_t* iter);

STDB_IMPORT(datastore_btree_scan_bsatn)
uint16_t datastore_btree_scan_bsatn(
    uint32_t index_id, const uint8_t* prefix, uint32_t prefix_len, uint16_t prefix_elems,
    const uint8_t* rstart, uint32_t rstart_len, const uint8_t* rend, uint32_t rend_len, 
    uint32_t* iter);

// ===== Row Iterator Operations =====
STDB_IMPORT(row_iter_bsatn_advance)
int16_t row_iter_bsatn_advance(uint32_t iter, uint8_t* buffer_ptr, size_t* buffer_len_ptr);

STDB_IMPORT(row_iter_bsatn_close)
uint16_t row_iter_bsatn_close(uint32_t iter);

// ===== Data Manipulation =====
STDB_IMPORT(datastore_insert_bsatn)
uint16_t datastore_insert_bsatn(uint32_t table_id, const uint8_t* row_ptr, size_t* row_len_ptr);

STDB_IMPORT(datastore_update_bsatn)
uint16_t datastore_update_bsatn(uint32_t table_id, uint32_t index_id, const uint8_t* row_ptr, size_t* row_len_ptr);

// ===== Delete Operations =====
STDB_IMPORT(datastore_delete_by_index_scan_range_bsatn)
uint16_t datastore_delete_by_index_scan_range_bsatn(
    uint32_t index_id, const uint8_t* prefix, uint32_t prefix_len, uint16_t prefix_elems,
    const uint8_t* rstart, uint32_t rstart_len, const uint8_t* rend, uint32_t rend_len, 
    uint32_t* num_deleted);

STDB_IMPORT(datastore_delete_by_btree_scan_bsatn)
uint16_t datastore_delete_by_btree_scan_bsatn(
    uint32_t index_id, const uint8_t* prefix, uint32_t prefix_len, uint16_t prefix_elems,
    const uint8_t* rstart, uint32_t rstart_len, const uint8_t* rend, uint32_t rend_len, 
    uint32_t* num_deleted);

STDB_IMPORT(datastore_delete_all_by_eq_bsatn)
uint16_t datastore_delete_all_by_eq_bsatn(
    uint32_t table_id, const uint8_t* rel_ptr, uint32_t rel_len,
    uint32_t* num_deleted);

// ===== Bytes Source/Sink Operations =====
STDB_IMPORT(bytes_source_read)
int16_t bytes_source_read(uint32_t source, uint8_t* buffer_ptr, size_t* buffer_len_ptr);

STDB_IMPORT(bytes_sink_write)
uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);

// ===== Console/Logging Operations =====
STDB_IMPORT(console_log)
void console_log(
    uint8_t level, const uint8_t* target_ptr, uint32_t target_len,
    const uint8_t* filename_ptr, uint32_t filename_len, uint32_t line_number,
    const uint8_t* message_ptr, uint32_t message_len);

STDB_IMPORT(console_timer_start)
uint32_t console_timer_start(const uint8_t* name, size_t name_len);

STDB_IMPORT(console_timer_end)
uint16_t console_timer_end(uint32_t stopwatch_id);

// ===== Scheduling =====
STDB_IMPORT(volatile_nonatomic_schedule_immediate)
void volatile_nonatomic_schedule_immediate(
    const uint8_t* name, size_t name_len, const uint8_t* args, size_t args_len);

// ===== Identity =====
STDB_IMPORT(identity)
void identity(void* id_ptr);

} // extern "C"

// ===== EXPORT DECLARATIONS =====
// Define export attribute macro
#define STDB_EXPORT(name) __attribute__((export_name(#name)))

extern "C" {

// ===== Required Module Exports =====

// Describe the module's schema (tables, reducers, types)
// @param description: A BytesSink handle to write the module description to
STDB_EXPORT(__describe_module__)
void __describe_module__(uint32_t description);

// Call a reducer function
// @param id: The reducer ID to call
// @param sender_0-3: The sender identity (128-bit value split into 4 uint64_t)
// @param conn_id_0-1: The connection ID (64-bit value split into 2 uint64_t)
// @param timestamp: The timestamp in microseconds since epoch
// @param args: A BytesSource handle to read arguments from
// @param error: A BytesSink handle to write error messages to
// @return: 0 on success, negative error code on failure
STDB_EXPORT(__call_reducer__)
int16_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error);

} // extern "C"

// ===== C++ CONVENIENCE DEFINITIONS =====
namespace SpacetimeDb {
namespace Abi {

// Error codes matching Rust errno.rs
enum class Errno : uint16_t {
    OK = 0,
    HOST_CALL_FAILURE = 1,
    NOT_IN_TRANSACTION = 2,
    BSATN_DECODE_ERROR = 3,
    NO_SUCH_TABLE = 4,
    NO_SUCH_INDEX = 5,
    NO_SUCH_ITER = 6,
    NO_SUCH_CONSOLE_TIMER = 7,
    NO_SUCH_BYTES = 8,
    NO_SPACE = 9,
    BUFFER_TOO_SMALL = 11,
    UNIQUE_ALREADY_EXISTS = 12,
    SCHEDULE_AT_DELAY_TOO_LONG = 13,
    INDEX_NOT_UNIQUE = 14,
    NO_SUCH_ROW = 15,
    // Custom values for module operations
    NO_SUCH_REDUCER = 999,
    UNKNOWN = 0xFFFF
};

// Log levels
enum class LogLevel : uint8_t {
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3,
    TRACE = 4
};

// Index types
enum class IndexType : uint8_t {
    BTREE = 0,
    HASH = 1
};

// Helper functions
inline bool is_ok(uint16_t status) { return status == 0; }
inline bool is_error(uint16_t status) { return status != 0; }

// Utility functions for common operations
namespace Utils {

// Helper to write data to a BytesSink
inline void write_bytes_to_sink(uint32_t sink_handle, const uint8_t* data, size_t len) {
    size_t buffer_len = len;
    uint16_t status = bytes_sink_write(sink_handle, data, &buffer_len);
    if (status != 0) {
        // In a real implementation, this would use the SDK's exception system
        // For now, we just ignore errors in this utility function
    }
}

// Helper to read all data from a BytesSource
inline bool read_all_from_source(uint32_t source_handle, uint8_t* buffer, size_t* buffer_len) {
    int16_t result = bytes_source_read(source_handle, buffer, buffer_len);
    return result >= 0;
}

} // namespace Utils

} // namespace Abi
} // namespace SpacetimeDb

#endif // SPACETIMEDB_ABI_H