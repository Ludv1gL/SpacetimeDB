#ifndef SPACETIMEDB_INTERNAL_FFI_H
#define SPACETIMEDB_INTERNAL_FFI_H

#include <cstdint>
#include <cstddef>
#include "spacetimedb/sdk/spacetimedb_sdk_types.h"
#include "spacetimedb/abi/spacetimedb_abi.h"

namespace SpacetimeDb {
namespace Internal {
namespace FFI {

// Error codes matching the actual ABI from crates/primitives/src/errno.rs
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
    NO_SUCH_REDUCER = 999, // Custom value for module errors
    UNKNOWN = 0xFFFF
};

// Use simple types for handles - the ABI functions expect uint32_t
using RowIter = uint32_t;
using TableId = uint32_t;
using BytesSource = uint32_t;
using BytesSink = uint32_t;

// Wrapper functions that convert between our types and ABI types
inline Errno table_id_from_name(const uint8_t* name, uint32_t name_len, TableId* out) {
    return static_cast<Errno>(_get_table_id(name, name_len, out));
}

inline Errno datastore_table_row_count(TableId table_id, uint64_t* count) {
    // TODO: Implement using appropriate ABI function
    return Errno::OK;
}

inline Errno datastore_table_scan_bsatn(TableId table_id, RowIter* out) {
    ::BufferIter iter;
    auto err = _iter_start(table_id, &iter);
    *out = iter;
    return static_cast<Errno>(err);
}

inline Errno datastore_insert_bsatn(TableId table_id, uint8_t* row, uint32_t* row_len) {
    size_t len = *row_len;
    auto err = _insert(table_id, row, len);
    *row_len = static_cast<uint32_t>(len);
    return static_cast<Errno>(err);
}

inline Errno datastore_delete_all_by_eq_bsatn(TableId table_id, const uint8_t* args, uint32_t args_len, uint32_t* count) {
    // TODO: Implement using _delete_by_col_eq
    return Errno::OK;
}

// Iterator operations
inline Errno row_iter_bsatn_advance(RowIter iter, uint8_t* buffer, uint32_t* buffer_len) {
    ::Buffer buf;
    auto err = _iter_next(iter, &buf);
    if (err == 0) {
        size_t len = _buffer_len(buf);
        if (len <= *buffer_len) {
            _buffer_consume(buf, buffer, len);
            *buffer_len = static_cast<uint32_t>(len);
        } else {
            return Errno::BUFFER_TOO_SMALL;
        }
    }
    return static_cast<Errno>(err);
}

inline void row_iter_bsatn_close(RowIter iter) {
    _iter_drop(iter);
}

// Bytes source/sink operations using correct ABI functions
inline int16_t bytes_source_read(BytesSource source, uint8_t* buffer, size_t* buffer_len) {
    ::BytesSource src{static_cast<uint32_t>(source)};
    return ::bytes_source_read(src, buffer, buffer_len);
}

inline Errno bytes_sink_write(BytesSink sink, const uint8_t* buffer, size_t* buffer_len) {
    ::BytesSink snk{static_cast<uint32_t>(sink)};
    return static_cast<Errno>(::bytes_sink_write(snk, buffer, buffer_len));
}

// Console operations
inline void console_log(const uint8_t* message, uint32_t message_len, uint8_t level) {
    _console_log(level, nullptr, 0, nullptr, 0, 0, message, message_len);
}

} // namespace FFI
} // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_FFI_H