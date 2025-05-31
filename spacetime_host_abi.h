#ifndef SPACETIME_HOST_ABI_H
#define SPACETIME_HOST_ABI_H

#include <cstdint> // For standard integer types
#include <cstddef> // For size_t (though uint32_t is often used for lengths in ABI)

// These are function prototypes that the C++ Wasm module *imports* from the host.
// The SpacetimeDB runtime/host environment is responsible for providing these implementations.
//
// Naming convention: spacetimedb_host_module_operation
// - module: e.g., table, log, context, reducer
// - operation: e.g., insert_row, get_row, log_message, get_transaction_id
//
// String parameters are typically passed as a pointer to the first character and a length.
// Byte buffers are passed as a pointer to the first byte and a length.
// Return values: Often int32_t, where 0 means success and non-zero indicates an error code.

#ifdef __cplusplus
extern "C" {
#endif

// --- Logging ---
void spacetimedb_host_log_message(const char* message_ptr, uint32_t message_len, uint8_t level); // level e.g., 0:Error, 1:Warn, 2:Info, 3:Debug

// --- Table Operations ---
// Inserts a new row into the specified table.
// `table_name_ptr`/`table_name_len`: Specifies the target table.
// `row_data_ptr`/`row_data_len`: BSATN-serialized row data.
// Returns 0 on success, or an error code (e.g., if table not found, data malformed, PK conflict).
int32_t spacetimedb_host_table_insert(const char* table_name_ptr, uint32_t table_name_len,
                                   const unsigned char* row_data_ptr, uint32_t row_data_len);

// Updates an existing row in the specified table, identified by its primary key.
// `table_name_ptr`/`table_name_len`: Specifies the target table.
// `pk_data_ptr`/`pk_data_len`: BSATN-serialized primary key value of the row to update.
// `row_data_ptr`/`row_data_len`: BSATN-serialized new row data.
// Returns 0 on success, or an error code (e.g., if table/row not found, data malformed).
// Note: A more common pattern might be to include the PK in row_data and have a single update function.
// Or, the host might provide specific update functions based on PK type if known.
// For a generic approach, sending PK separately might be one way.
// Let's simplify: assume row_data contains the PK and identifies the row.
// int32_t spacetimedb_host_table_update_by_pk(const char* table_name_ptr, uint32_t table_name_len,
//                                          const unsigned char* pk_data_ptr, uint32_t pk_data_len,
//                                          const unsigned char* row_data_ptr, uint32_t row_data_len);

// Deletes a row from the specified table, identified by its primary key.
// `table_name_ptr`/`table_name_len`: Specifies the target table.
// `pk_data_ptr`/`pk_data_len`: BSATN-serialized primary key value of the row to delete.
// Returns 0 on success, or an error code (e.g., if table/row not found).
int32_t spacetimedb_host_table_delete_by_pk(const char* table_name_ptr, uint32_t table_name_len,
                                         const unsigned char* pk_data_ptr, uint32_t pk_data_len);


// --- Transaction Context ---
// Gets the current SpacetimeDB transaction ID.
// `out_transaction_id_ptr`: Pointer to a buffer where the transaction ID (uint64_t) will be written.
// Must point to at least 8 bytes of writable memory.
// void spacetimedb_host_context_get_transaction_id(unsigned char* out_transaction_id_ptr); // uint64_t

// Gets the identity of the sender of the current transaction/event.
// `out_identity_ptr_ptr`: Pointer to a `unsigned char*`. The host will allocate memory (using its allocator, e.g. Wasm's)
//                         and write the pointer to this allocated memory here.
// `out_identity_len_ptr`: Pointer to a `uint32_t` where the length of the identity data will be written.
// The Wasm module is responsible for eventually calling `spacetimedb_host_memory_free` on the allocated pointer.
// void spacetimedb_host_context_get_sender_identity(unsigned char** out_identity_ptr_ptr, uint32_t* out_identity_len_ptr);

// Gets the current host timestamp (e.g., nanoseconds since a SpacetimeDB epoch).
// `out_timestamp_ptr`: Pointer to a buffer where the timestamp (uint64_t) will be written.
// void spacetimedb_host_context_get_timestamp(unsigned char* out_timestamp_ptr); // uint64_t


// --- Memory Management for Host-Allocated Data ---
// (If any host functions allocate memory and pass ownership to Wasm)
// `ptr`: A pointer previously returned by a host function that allocated memory.
// void spacetimedb_host_memory_free(unsigned char* ptr);


// --- Scheduling Future Events/Reducers ---
// Schedules a reducer to be called after a delay.
// `nanoseconds_from_now`: Delay in nanoseconds.
// `reducer_name_ptr`/`reducer_name_len`: Name of the reducer to schedule.
// `args_data_ptr`/`args_data_len`: BSATN-serialized arguments for the reducer.
// Returns 0 on success, or an error code.
// int32_t spacetimedb_host_schedule_reducer(uint64_t nanoseconds_from_now,
//                                        const char* reducer_name_ptr, uint32_t reducer_name_len,
//                                        const unsigned char* args_data_ptr, uint32_t args_data_len);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // SPACETIME_HOST_ABI_H
