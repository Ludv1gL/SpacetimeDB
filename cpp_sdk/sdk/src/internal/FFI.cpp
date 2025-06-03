#include "spacetimedb/internal/FFI.h"

// Define the FFI functions as weak symbols that will be provided by the SpacetimeDB runtime
extern "C" {

__attribute__((weak))
SpacetimeDb::Internal::FFI::Errno table_id_from_name(
    const uint8_t* name, uint32_t name_len, SpacetimeDb::Internal::FFI::TableId* out
) {
    // Weak symbol - will be overridden by runtime
    return SpacetimeDb::Internal::FFI::Errno::NO_SUCH_TABLE;
}

__attribute__((weak))
SpacetimeDb::Internal::FFI::Errno datastore_table_row_count(
    SpacetimeDb::Internal::FFI::TableId table_id, uint64_t* count
) {
    // Weak symbol - will be overridden by runtime
    *count = 0;
    return SpacetimeDb::Internal::FFI::Errno::OK;
}

__attribute__((weak))
SpacetimeDb::Internal::FFI::Errno datastore_table_scan_bsatn(
    SpacetimeDb::Internal::FFI::TableId table_id, SpacetimeDb::Internal::FFI::RowIter* out
) {
    // Weak symbol - will be overridden by runtime
    *out = SpacetimeDb::Internal::FFI::RowIter::INVALID;
    return SpacetimeDb::Internal::FFI::Errno::OK;
}

__attribute__((weak))
SpacetimeDb::Internal::FFI::Errno datastore_insert_bsatn(
    SpacetimeDb::Internal::FFI::TableId table_id, const uint8_t* row, uint32_t* row_len
) {
    // Weak symbol - will be overridden by runtime
    return SpacetimeDb::Internal::FFI::Errno::OK;
}

__attribute__((weak))
SpacetimeDb::Internal::FFI::Errno datastore_delete_all_by_eq_bsatn(
    SpacetimeDb::Internal::FFI::TableId table_id, const uint8_t* args, uint32_t args_len, uint32_t* count
) {
    // Weak symbol - will be overridden by runtime
    *count = 0;
    return SpacetimeDb::Internal::FFI::Errno::OK;
}

__attribute__((weak))
SpacetimeDb::Internal::FFI::Errno row_iter_bsatn_advance(
    SpacetimeDb::Internal::FFI::RowIter iter, uint8_t* buffer, uint32_t* buffer_len
) {
    // Weak symbol - will be overridden by runtime
    *buffer_len = 0;
    return SpacetimeDb::Internal::FFI::Errno::EXHAUSTED;
}

__attribute__((weak))
void row_iter_bsatn_close(SpacetimeDb::Internal::FFI::RowIter iter) {
    // Weak symbol - will be overridden by runtime
}

__attribute__((weak))
SpacetimeDb::Internal::FFI::Errno bytes_source_read(
    SpacetimeDb::Internal::FFI::BytesSource source, uint8_t* buffer, uint32_t* buffer_len
) {
    // Weak symbol - will be overridden by runtime
    *buffer_len = 0;
    return SpacetimeDb::Internal::FFI::Errno::EXHAUSTED;
}

__attribute__((weak))
void bytes_sink_write(
    SpacetimeDb::Internal::FFI::BytesSink sink, const uint8_t* buffer, uint32_t* buffer_len
) {
    // Weak symbol - will be overridden by runtime
}

__attribute__((weak))
void console_log(const uint8_t* message, uint32_t message_len, uint8_t level) {
    // Weak symbol - will be overridden by runtime
}

} // extern "C"