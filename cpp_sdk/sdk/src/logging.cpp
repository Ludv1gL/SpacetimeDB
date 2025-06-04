#include "spacetimedb/sdk/logging.h"
#include "spacetimedb/abi/spacetimedb_abi.h" // For ABI definitions

namespace SpacetimeDb {

void log(LogLevel level, const std::string& message) {
    // Convert LogLevel to uint8_t for console_log ABI
    uint8_t abi_level = static_cast<uint8_t>(level);
    console_log(abi_level, nullptr, 0, nullptr, 0, 0,
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

} // namespace SpacetimeDb
