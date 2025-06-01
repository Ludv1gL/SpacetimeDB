#ifndef SPACETIME_HOST_ABI_H
#define SPACETIME_HOST_ABI_H

#include "spacetime_common_defs.h" // For Status, LogLevel, BytesSink, BytesSource etc.
#include <cstdint>
#include <cstddef> // For size_t (though uint32_t is often used for ABI lengths)

// These are function prototypes that the C++ Wasm module *imports* from the SpacetimeDB host.
// They are based on the functions marked IMPORT in crates/bindings-csharp/Runtime/bindings.c
//
// String parameters are passed as a pointer to the first character (UTF-8) and a length.
// Byte buffers are passed as a pointer to the first byte and a length.

#ifdef __cplusplus
extern "C" {
#endif

// --- Logging ---
// IMPORT void log_message(LogLevel level, String message_str);
void log_message(LogLevel level, const uint8_t* message_ptr, uint32_t message_len);

// --- Time ---
// IMPORT uint64_t unixtime_nanos();
uint64_t unixtime_nanos();

// --- Random ---
// IMPORT void get_random_bytes(Bytes message_bytes); // Assumes message_bytes is an out-param struct with ptr/len
// Simplified: provide buffer to be filled.
// IMPORT void fill_random_bytes(uint8_t* buffer_ptr, uint32_t buffer_len);
void fill_random_bytes(uint8_t* buffer_ptr, uint32_t buffer_len);

// --- Table Operations ---
// IMPORT Status table_insert(String table_name_str, Bytes row_bytes);
Status table_insert(const uint8_t* table_name_ptr, uint32_t table_name_len,
                    const uint8_t* row_data_ptr, uint32_t row_data_len);

// IMPORT Status table_update_by_pk(String table_name_str, Bytes pk_bytes, Bytes row_bytes);
Status table_update_by_pk(const uint8_t* table_name_ptr, uint32_t table_name_len,
                          const uint8_t* pk_data_ptr, uint32_t pk_data_len,
                          const uint8_t* row_data_ptr, uint32_t row_data_len);

// IMPORT Status table_delete_by_pk(String table_name_str, Bytes pk_bytes);
Status table_delete_by_pk(const uint8_t* table_name_ptr, uint32_t table_name_len,
                          const uint8_t* pk_data_ptr, uint32_t pk_data_len);

// IMPORT RowIter table_iter(String table_name_str);
RowIter table_iter(const uint8_t* table_name_ptr, uint32_t table_name_len);

// IMPORT Status row_iter_next(RowIter iter_handle, BytesSink pk_sink_handle, BytesSink row_sink_handle);
Status row_iter_next(RowIter iter_handle, BytesSink pk_sink_handle, BytesSink row_sink_handle);

// IMPORT void row_iter_done(RowIter iter_handle);
void row_iter_done(RowIter iter_handle);


// --- Configuration ---
// IMPORT uint32_t own_identity(BytesSink identity_sink_handle);
uint32_t own_identity(BytesSink identity_sink_handle);

// IMPORT uint64_t own_address(); // This might be different in bindings.c, e.g. returning bytes
// Let's assume it returns bytes similar to own_identity for consistency
uint32_t own_address(BytesSink address_sink_handle);

// IMPORT uint64_t transaction_timestamp_nanos();
uint64_t transaction_timestamp_nanos();


// --- BytesSink and BytesSource Operations ---
// IMPORT BytesSink bytes_sink_create();
BytesSink bytes_sink_create();

// IMPORT void bytes_sink_done(BytesSink sink_handle);
void bytes_sink_done(BytesSink sink_handle);

// IMPORT Status bytes_sink_write(BytesSink sink_handle, Bytes bytes);
Status bytes_sink_write(BytesSink sink_handle, const uint8_t* data_ptr, uint32_t data_len);

// IMPORT uint32_t bytes_sink_get_written_count(BytesSink sink_handle);
uint32_t bytes_sink_get_written_count(BytesSink sink_handle);


// IMPORT BytesSource bytes_source_create_from_bytes(Bytes bytes);
BytesSource bytes_source_create_from_bytes(const uint8_t* data_ptr, uint32_t data_len);

// IMPORT BytesSource bytes_source_create_from_sink_bytes(BytesSink sink_handle);
BytesSource bytes_source_create_from_sink_bytes(BytesSink sink_handle);

// IMPORT void bytes_source_done(BytesSource source_handle);
void bytes_source_done(BytesSource source_handle);

// IMPORT uint32_t bytes_source_read(BytesSource source_handle, Bytes bytes);
// (Bytes here is an out-parameter: a buffer to fill)
// Returns number of bytes read.
uint32_t bytes_source_read(BytesSource source_handle, uint8_t* buffer_ptr, uint32_t buffer_len);

// IMPORT uint32_t bytes_source_get_remaining_count(BytesSource source_handle);
uint32_t bytes_source_get_remaining_count(BytesSource source_handle);


// --- Reducer Scheduling (Example, might differ from actual bindings.c) ---
// IMPORT Status schedule_event_bytes_args(uint64_t nanoseconds_from_now, String reducer_name_str, Bytes args_bytes);
Status schedule_event_bytes_args(uint64_t nanoseconds_from_now,
                                 const uint8_t* reducer_name_ptr, uint32_t reducer_name_len,
                                 const uint8_t* args_data_ptr, uint32_t args_data_len);


// --- Console Timer (Example) ---
// IMPORT ConsoleTimerId set_timeout(uint32_t delay_millis, uint32_t callback_id); // callback_id might be func ptr or index
// void clear_timeout(ConsoleTimerId timer_id);


// --- WASI Shims (Minimal set, others might be needed) ---
// These are typically provided by the host or a WASI runtime environment.
// If SpacetimeDB host requires the module to *not* import these directly from WASI,
// then the host would provide them (possibly with its own namespacing).
// The bindings.c lists: fd_write, environ_sizes_get, environ_get, proc_exit.
// For C++, these are often part of the standard library's implementation when targeting WASI.
// It's unusual for a module to directly import these by these exact names unless it's very low-level.
// We'll assume for now that the C++ standard library + Emscripten deals with these,
// or if specific shims are needed, they'd be part of wasi_shims.cpp.
// For direct ABI compatibility with bindings.c, we'd declare them:
/*
typedef struct { uint32_t iovs_ptr; uint32_t iovs_len; } CiovecArray; // Helper for fd_write
Status fd_write(uint32_t fd, CiovecArrayiovs, uint32_t* nwritten_ptr);
Status environ_sizes_get(uint32_t* environ_count_ptr, uint32_t* environ_buf_size_ptr);
Status environ_get(uint8_t** environ_data_ptr, uint8_t* environ_buf_ptr);
void proc_exit(uint32_t rval);
*/

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SPACETIME_HOST_ABI_H
