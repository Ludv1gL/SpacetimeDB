#include "spacetimedb/sdk/logging.h"
#include "spacetimedb/abi/spacetimedb_abi.h" // For ABI functions
#include "spacetimedb/abi/common_defs.h"   // For SpacetimeDB::Abi::to_abi (LogLevelCpp -> ::LogLevel)
#include <cstring>
#include <filesystem>

namespace SpacetimeDB {

void log_with_caller_info(LogLevel level, const std::string& message, 
                         const char* target, const char* filename, uint32_t line_number) {
    // Convert SpacetimeDB::LogLevel to uint8_t for ABI
    uint8_t abi_level = static_cast<uint8_t>(level);
    
    // Handle nullptr parameters
    const char* safe_target = target ? target : "";
    const char* safe_filename = filename ? filename : "";
    
    // Extract just the filename from full path for cleaner logs
    std::string filename_only;
    if (filename && std::strlen(filename) > 0) {
        std::filesystem::path path(filename);
        filename_only = path.filename().string();
        safe_filename = filename_only.c_str();
    }
    
    // Call the enhanced console_log function with caller information
    ::_console_log(abi_level,
                   reinterpret_cast<const uint8_t*>(safe_target), std::strlen(safe_target),
                   reinterpret_cast<const uint8_t*>(safe_filename), std::strlen(safe_filename),
                   line_number,
                   reinterpret_cast<const uint8_t*>(message.c_str()), message.length());
}

void log(LogLevel level, const std::string& message) {
    // Legacy API - use enhanced version with empty caller info
    log_with_caller_info(level, message, nullptr, nullptr, 0);
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

// LogStopwatch implementation
LogStopwatch::LogStopwatch(const std::string& name) : ended_(false) {
    timer_id_ = ::_console_timer_start(
        reinterpret_cast<const uint8_t*>(name.c_str()),
        name.length()
    );
}

LogStopwatch::~LogStopwatch() {
    if (!ended_) {
        end();
    }
}

void LogStopwatch::end() {
    if (!ended_) {
        uint16_t status = ::_console_timer_end(timer_id_);
        ended_ = true;
        // TODO: Add error handling when we implement exception system
        (void)status; // Suppress unused variable warning for now
    }
}

LogStopwatch::LogStopwatch(LogStopwatch&& other) noexcept 
    : timer_id_(other.timer_id_), ended_(other.ended_) {
    other.ended_ = true; // Prevent double-ending
}

LogStopwatch& LogStopwatch::operator=(LogStopwatch&& other) noexcept {
    if (this != &other) {
        if (!ended_) {
            end(); // End current timer
        }
        timer_id_ = other.timer_id_;
        ended_ = other.ended_;
        other.ended_ = true; // Prevent double-ending
    }
    return *this;
}

} // namespace SpacetimeDB
