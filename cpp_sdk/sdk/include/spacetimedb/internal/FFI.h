#ifndef SPACETIMEDB_INTERNAL_FFI_H
#define SPACETIMEDB_INTERNAL_FFI_H

#include <cstdint>
#include <cstddef>
#include "spacetimedb/sdk/spacetimedb_sdk_types.h"

namespace SpacetimeDb {
namespace Internal {
namespace FFI {

// Error codes matching the ABI
enum class Errno : uint16_t {
    OK = 0,
    HOST_CALL_FAILURE = 1,
    EXHAUSTED = 2,
    NO_SUCH_ITER = 3,
    NO_SUCH_BYTES = 4,
    NO_SUCH_TABLE = 5,
    BUFFER_TOO_SMALL = 6,
    UNIQUE_CONSTRAINT_VIOLATION = 7,
    BSATN_DECODE_ERROR = 8,
    NO_SUCH_REDUCER = 9,
    REDUCER_ARG_DECODE_ERROR = 10,
    NO_SUCH_SCHEDULED_REDUCER = 11,
    NOT_IN_TRANSACTION = 12,
    UNKNOWN = 0xFFFF
};

// Opaque handle types
struct RowIter {
    uint32_t handle;
    
    static const RowIter INVALID;
    
    bool operator==(const RowIter& other) const {
        return handle == other.handle;
    }
    
    bool operator!=(const RowIter& other) const {
        return handle != other.handle;
    }
};

struct TableId {
    uint32_t id;
};

struct BytesSource {
    uint32_t handle;
    
    static const BytesSource INVALID;
};

struct BytesSink {
    uint32_t handle;
};

// FFI function declarations matching the SpacetimeDB ABI

// Table operations
extern "C" {
    Errno table_id_from_name(const uint8_t* name, uint32_t name_len, TableId* out);
    Errno datastore_table_row_count(TableId table_id, uint64_t* count);
    Errno datastore_table_scan_bsatn(TableId table_id, RowIter* out);
    Errno datastore_insert_bsatn(TableId table_id, const uint8_t* row, uint32_t* row_len);
    Errno datastore_delete_all_by_eq_bsatn(TableId table_id, const uint8_t* args, uint32_t args_len, uint32_t* count);
    
    // Iterator operations
    Errno row_iter_bsatn_advance(RowIter iter, uint8_t* buffer, uint32_t* buffer_len);
    void row_iter_bsatn_close(RowIter iter);
    
    // Bytes source/sink operations
    Errno bytes_source_read(BytesSource source, uint8_t* buffer, uint32_t* buffer_len);
    void bytes_sink_write(BytesSink sink, const uint8_t* buffer, uint32_t* buffer_len);
    
    // Console operations
    void console_log(const uint8_t* message, uint32_t message_len, uint8_t level);
}

} // namespace FFI
} // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_FFI_H