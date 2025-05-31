#include "spacetimedb/sdk/logging.h"
#include "spacetimedb/abi/spacetimedb_abi.h" // For ::_log_message_abi
#include "spacetimedb/abi/common_defs.h"   // For SpacetimeDB::Abi::to_abi (LogLevelCpp -> ::LogLevel)

namespace SpacetimeDB {

void log(LogLevel level, const std::string& message) {
    // Convert SpacetimeDB::LogLevel (which is Abi::LogLevelCpp) to the C-style ::LogLevel ABI type
    ::LogLevel abi_level = SpacetimeDB::Abi::to_abi(level);
    ::_log_message_abi(abi_level,
                       reinterpret_cast<const uint8_t*>(message.c_str()),
                       static_cast<uint32_t>(message.length()));
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

} // namespace SpacetimeDB
