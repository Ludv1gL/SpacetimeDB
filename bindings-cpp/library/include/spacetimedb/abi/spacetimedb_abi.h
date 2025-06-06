#ifndef SPACETIMEDB_ABI_H
#define SPACETIMEDB_ABI_H

#include <cstdint>
#include <cstddef>

/**
 * @file spacetimedb_abi.h
 * @brief Complete SpacetimeDB ABI interface for C++ modules
 * 
 * This file provides all module imports (host functions) and exports (module functions)
 * required for SpacetimeDB WASM modules. It serves as the single source of truth
 * for all ABI definitions in the C++ SDK.
 * 
 * Organization:
 * - Import declarations: Functions provided by the SpacetimeDB host
 * - Export declarations: Functions that modules must/can provide
 * - C++ convenience definitions: Type-safe wrappers and enums
 * 
 * Note: WASI shims for C++ standard library support are provided separately
 * in spacetimedb/abi/wasi_shims.h and implemented in the module library.
 */

// ========================================================================
// SECTION 1: IMPORT DECLARATIONS - Functions provided by SpacetimeDB host
// ========================================================================

// Macro for declaring imported functions from the SpacetimeDB host
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
uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row_ptr, size_t* row_len_ptr);

STDB_IMPORT(datastore_update_bsatn)
uint16_t datastore_update_bsatn(uint32_t table_id, uint32_t index_id, uint8_t* row_ptr, size_t* row_len_ptr);

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
#ifdef SPACETIMEDB_UNSTABLE_FEATURES
STDB_IMPORT(volatile_nonatomic_schedule_immediate)
void volatile_nonatomic_schedule_immediate(
    const uint8_t* name, size_t name_len, const uint8_t* args, size_t args_len);
#endif

// ===== Identity =====
STDB_IMPORT(identity)
void identity(uint8_t* id_ptr);

} // extern "C"

// ========================================================================
// SECTION 2: EXPORT DECLARATIONS - Functions modules provide to SpacetimeDB
// ========================================================================

// Macro for declaring exported functions that the module provides
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

// ===== Optional Module Exports =====

// Optional: Pre-initialization functions
// These run first in alphabetical order before __setup__
// Example naming: __preinit__20_setup_logging()
// STDB_EXPORT(__preinit__XX_XXXX)
// void __preinit__XX_XXXX();

// Optional: Setup function for dynamic languages
// Run after __preinit__ functions, can return an error
// Intended for initializing interpreters and loading user modules
// @return: 0 on success, error code on failure
// STDB_EXPORT(__setup__)
// int16_t __setup__();

// Optional: Database migration functions (currently unused)
// @param sender_0-3: The sender identity performing the migration
// @param timestamp: The timestamp of the migration
// @param something: Migration-specific data
// @return: 0 on success, error code on failure
// STDB_EXPORT(__migrate_database__XXXX)
// int16_t __migrate_database__XXXX(
//     uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
//     uint64_t timestamp,
//     uint32_t something);

} // extern "C"

// ========================================================================
// SECTION 3: C++ CONVENIENCE DEFINITIONS - Type-safe wrappers and utilities
// ========================================================================

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

// ========================================================================
// SECTION 4: LEGACY COMPATIBILITY - Deprecated but maintained for backward compatibility
// ========================================================================

// Legacy underscore-prefixed imports (maintained for compatibility)
extern "C" {

// These match the existing SDK usage but are deprecated
// New code should use the non-underscore versions above
// Note: _get_table_id is an alias to table_id_from_name for API compatibility
#define _get_table_id table_id_from_name

// Wrapper function for _insert to match expected signature
inline uint16_t _insert(uint32_t table_id, uint8_t* row_bsatn_ptr, size_t row_bsatn_len) {
    size_t len = row_bsatn_len;
    return datastore_insert_bsatn(table_id, row_bsatn_ptr, &len);
}

STDB_IMPORT(_delete_by_col_eq)
uint16_t _delete_by_col_eq(uint32_t table_id, uint32_t col_id, const uint8_t* value_bsatn_ptr,
    size_t value_bsatn_len, uint32_t* out_deleted_count_ptr);

STDB_IMPORT(_iter_start)
uint16_t _iter_start(uint32_t table_id, uint32_t* out_iter_ptr);

STDB_IMPORT(_iter_next)
uint16_t _iter_next(uint32_t iter_handle, uint32_t* out_row_data_buf_ptr);

STDB_IMPORT(_iter_drop)
uint16_t _iter_drop(uint32_t iter_handle);

STDB_IMPORT(_iter_by_col_eq)
uint16_t _iter_by_col_eq(uint32_t table_id, uint32_t col_id, const uint8_t* value_bsatn_ptr,
    size_t value_bsatn_len, uint32_t* out_buffer_ptr_with_rows);

STDB_IMPORT(_iter_start_filtered)
uint16_t _iter_start_filtered(uint32_t table_id, const uint8_t* filter_bsatn_ptr,
    size_t filter_bsatn_len, uint32_t* out_iter_ptr);

// Buffer operations
STDB_IMPORT(_buffer_alloc)
uint32_t _buffer_alloc(const uint8_t* data, size_t data_len);

STDB_IMPORT(_buffer_consume)
uint16_t _buffer_consume(uint32_t bufh, uint8_t* into, size_t len);

STDB_IMPORT(_buffer_len)
size_t _buffer_len(uint32_t bufh);

// Scheduling
STDB_IMPORT(_schedule_reducer)
uint16_t _schedule_reducer(const uint8_t* name, size_t name_len, const uint8_t* args,
    size_t args_len, uint64_t time, uint64_t* out_schedule_id_ptr);

STDB_IMPORT(_cancel_reducer)
uint16_t _cancel_reducer(uint64_t id);

// Index operations
STDB_IMPORT(_create_index)
uint16_t _create_index(const uint8_t* index_name, size_t index_name_len, uint32_t table_id,
    uint8_t index_type, const uint8_t* col_ids, size_t col_len);

} // extern "C"

#endif // SPACETIMEDB_ABI_H