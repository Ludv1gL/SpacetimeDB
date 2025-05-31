#include "spacetime_sdk_runtime.h"
#include "spacetime_host_abi.h" // For the actual extern "C" declarations

namespace SpacetimeDB {

void log(LogLevel level, const std::string& message) {
    spacetimedb_host_log_message(message.c_str(), static_cast<uint32_t>(message.length()), static_cast<uint8_t>(level));
}

void log_error(const std::string& message) {
    log(LogLevel::Error, message);
}

void log_warn(const std::string& message) {
    log(LogLevel::Warn, message);
}

void log_info(const std::string& message) {
    log(LogLevel::Info, message);
}

void log_debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void log_trace(const std::string& message) {
    log(LogLevel::Trace, message);
}

// Implementations for context or other non-templated functions would go here.
// Example:
// uint64_t context_get_transaction_id() {
//     uint64_t transaction_id_val;
//     spacetimedb_host_context_get_transaction_id(reinterpret_cast<unsigned char*>(&transaction_id_val));
//     return transaction_id_val; // Assuming direct u64 write and correct endianness from host
// }

} // namespace SpacetimeDB
