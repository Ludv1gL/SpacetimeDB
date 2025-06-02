// Placeholder for WASI shims or other low-level OS interface polyfills.
//
// When C++ code (including the standard library) is compiled to WebAssembly,
// it may rely on certain system calls or environment features that are part of
// the WebAssembly System Interface (WASI). Examples include file I/O, environment
// variables, aspects of process management, etc.
//
// If the SpacetimeDB host environment does not provide a full WASI-compliant runtime,
// or if specific system calls need to be stubbed out or handled differently,
// this file would be the place to provide those shims.
//
// The `crates/bindings-csharp/Runtime/bindings.c` file includes shims for:
// - `fd_write` (for console output, often `stdout`/`stderr`)
// - `environ_sizes_get` and `environ_get` (for environment variables)
// - `proc_exit` (to handle program termination)
// - `clock_time_get` (for time functions)
//
// If the C++ standard library used with Emscripten (like LLVM's libc++) requires
// these symbols and they are not provided by the SpacetimeDB host directly,
// minimal implementations or stubs would be needed here.
//
// For example, a very basic `fd_write` might be routed to a host logging function
// if `stdout/stderr` are not otherwise captured:
/*
#include <cstdint>
#include <unistd.h> // For STDOUT_FILENO, STDERR_FILENO (conceptual)
#include <iostream> // For temporary logging of shim calls

// Assuming spacetime_host_abi.h provides the logging function
// #include "spacetimedb/abi/spacetimedb_abi.h"

extern "C" {

// Example: Minimal fd_write shim
// This is a simplified version. A proper shim would need to handle iovecs correctly.
// __attribute__((import_module("wasi_snapshot_preview1"), import_name("fd_write"))) // If importing from actual WASI
// int32_t wasi_fd_write(int32_t fd, const uint8_t *iovs_ptr, int32_t iovs_len, uint32_t *nwritten_ptr) {
//     if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
//         size_t total_written = 0;
//         // Naive iovec handling: assumes iovs_ptr points to a single iovec for simplicity
//         // struct iovec { void *iov_base; size_t iov_len; };
//         // const struct iovec *iovs = (const struct iovec *)iovs_ptr;
//         // for (int32_t i = 0; i < iovs_len; ++i) {
//         //     std::string message(reinterpret_cast<const char*>(iovs[i].iov_base), iovs[i].iov_len);
//         //     // Replace with host logging if available and appropriate:
//         //     // _log_message_abi(..., message.data(), message.length());
//         //     if (fd == STDOUT_FILENO) std::cout << message;
//         //     else std::cerr << message;
//         //     total_written += iovs[i].iov_len;
//         // }
//         // if (nwritten_ptr) {
//         //     *nwritten_ptr = total_written;
//         // }
//         // For now, let's assume simple string output if iovs_len == 1 and iovs_ptr points to content
//         std::string message(reinterpret_cast<const char*>(iovs_ptr), iovs_len); // This is incorrect iovec interpretation
//         std::cerr << "[wasi_shim fd_write fd=" << fd << "] " << message << std::endl;
//         if (nwritten_ptr) *nwritten_ptr = iovs_len; // Assuming all written for simplicity
//         return 0; // Success
//     }
//     // For other file descriptors, typically return EBADF or similar
//     return -1; // Indicate error
// }

// Minimal proc_exit
// __attribute__((import_module("wasi_snapshot_preview1"), import_name("proc_exit")))
// void wasi_proc_exit(int32_t rval) {
//     std::cerr << "[wasi_shim proc_exit] Code: " << rval << ". Module attempting to exit." << std::endl;
//     // In a Wasm environment, proc_exit might trap or signal the host.
//     // For a SpacetimeDB module, exiting the process is generally not desired.
//     // This could log and then perhaps loop or trap to indicate an issue.
//     if (rval != 0) {
//         // Maybe log as error via host ABI if possible
//     }
//     // It's unlikely a SpacetimeDB module should actually terminate the process.
//     // This might just be an infinite loop or a trap.
//     while(1); // Trap
// }

// Other shims like environ_get, environ_sizes_get, clock_time_get would be here if needed.
// Emscripten often provides its own implementations or alternatives for these,
// so direct shimming might only be necessary if specific low-level WASI imports are being
// explicitly called by the module or if the Emscripten runtime expects them from the host.

} // extern "C"
*/

// For now, this file is a placeholder. Actual shims depend on specific needs identified
// during compilation and linking with the target Wasm environment (SpacetimeDB host).
// If the host provides these WASI functions, then no shims are needed from the SDK.
// If Emscripten's JS support layer provides them, also no shims needed here.
// These are primarily if the C++ stdlib compiles to raw WASI calls that the host must fulfill.
// The `bindings.c` approach is to re-export these from the Wasm module itself, which means
// the Wasm module must provide them if they are not linked in from elsewhere.
// This usually means the host is expected to link them *into* the module, or the module
// itself contains a WASI libc subset.

// The most common one needed if `printf` or iostreams are used is `fd_write`.
// Emscripten can often route this to `console.log` or similar JS facilities.
// If direct syscalls are made, then the host must provide `fd_write`.

// This C++ SDK currently uses iostream for debug/error messages in some places (stubs, etc.).
// In a proper Wasm build for SpacetimeDB, these should be replaced by calls to the
// host logging ABI (e.g., `_log_message_abi`). If iostreams are still linked and attempt
// to call `fd_write`, then a shim or host-provided `fd_write` would be necessary.

#include <iostream> // For a dummy symbol if file is empty

void spacetimedb_sdk_wasi_shims_placeholder() {
    // std::cout << "WASI shims placeholder linked." << std::endl;
}
