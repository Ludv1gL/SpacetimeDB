#ifndef SPACETIMEDB_LIBRARY_LOGGING_H
#define SPACETIMEDB_LIBRARY_LOGGING_H

#include "spacetimedb/abi/common_defs.h" // For SpacetimeDB::Abi::LogLevelCpp
#include <string>
#include <chrono>
#include <memory>

namespace SpacetimeDb {

// Re-expose LogLevelCpp from common_defs.h for convenience if desired, or use it directly via SpacetimeDb::Abi::LogLevelCpp
using LogLevel = SpacetimeDb::Abi::LogLevelCpp;

/**
 * @brief Enhanced logging with caller information injection.
 * @param level The severity level of the message.
 * @param message The message string to log.
 * @param target The function/method name (automatically injected).
 * @param filename The source file name (automatically injected).
 * @param line_number The source line number (automatically injected).
 * @ingroup sdk_runtime sdk_logging
 */
void log_with_caller_info(LogLevel level, const std::string& message, 
                         const char* target = nullptr, 
                         const char* filename = nullptr, 
                         uint32_t line_number = 0);

/**
 * @brief Logs a message through the SpacetimeDB host environment (legacy API).
 * @param level The severity level of the message, using `SpacetimeDB::LogLevel`.
 * @param message The message string to log.
 * @ingroup sdk_runtime sdk_logging
 */
void log(LogLevel level, const std::string& message);

// Enhanced logging macros with automatic caller information injection
#define LOG_ERROR(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::Error, (message), __func__, __FILE__, __LINE__)

#define LOG_WARN(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::Warn, (message), __func__, __FILE__, __LINE__)

#define LOG_INFO(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::Info, (message), __func__, __FILE__, __LINE__)

#define LOG_DEBUG(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::Debug, (message), __func__, __FILE__, __LINE__)

#define LOG_TRACE(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::Trace, (message), __func__, __FILE__, __LINE__)

/**
 * @brief Logs an error message. Convenience wrapper for `log(LogLevel::Error, ...)`.
 * @param message The message string to log.
 * @ingroup sdk_runtime sdk_logging
 */
void log_error(const std::string& message);

/**
 * @brief Logs a warning message. Convenience wrapper for `log(LogLevel::Warn, ...)`.
 * @param message The message string to log.
 * @ingroup sdk_runtime sdk_logging
 */
void log_warn(const std::string& message);

/**
 * @brief Logs an informational message. Convenience wrapper for `log(LogLevel::Info, ...)`.
 * @param message The message string to log.
 * @ingroup sdk_runtime sdk_logging
 */
void log_info(const std::string& message);

/**
 * @brief Logs a debug message. Convenience wrapper for `log(LogLevel::Debug, ...)`.
 * @param message The message string to log.
 * @ingroup sdk_runtime sdk_logging
 */
void log_debug(const std::string& message);

/**
 * @brief Logs a trace message. Convenience wrapper for `log(LogLevel::Trace, ...)`.
 * @param message The message string to log.
 * @ingroup sdk_runtime sdk_logging
 */
void log_trace(const std::string& message);

/**
 * @brief RAII performance measurement utility.
 * 
 * This class provides automatic performance timing with SpacetimeDB's
 * console timer system. The timer starts when constructed and automatically
 * ends when the object is destroyed (RAII pattern).
 * 
 * Example usage:
 * @code
 * {
 *     LogStopwatch timer("database_operation");
 *     // ... perform database operations ...
 *     // Timer automatically ends when timer goes out of scope
 * }
 * @endcode
 * 
 * @ingroup sdk_runtime sdk_logging
 */
class LogStopwatch {
public:
    /**
     * @brief Start a performance timer with the given name.
     * @param name The name of the operation being timed.
     */
    explicit LogStopwatch(const std::string& name);
    
    /**
     * @brief Destructor automatically ends the timer.
     */
    ~LogStopwatch();
    
    /**
     * @brief Manually end the timer (optional - destructor will do this automatically).
     */
    void end();
    
    // Disable copy construction and assignment
    LogStopwatch(const LogStopwatch&) = delete;
    LogStopwatch& operator=(const LogStopwatch&) = delete;
    
    // Allow move construction and assignment
    LogStopwatch(LogStopwatch&& other) noexcept;
    LogStopwatch& operator=(LogStopwatch&& other) noexcept;

private:
    uint32_t timer_id_;
    bool ended_;
};

// Inline implementations for header-only usage

#include <cstring>

// Include centralized ABI
#include "spacetimedb/abi/spacetimedb_abi.h"

// Simple filename extraction without filesystem dependency
inline const char* extract_filename(const char* path) {
    if (!path) return "";
    const char* last_slash = nullptr;
    const char* last_backslash = nullptr;
    
    // Find last occurrence of '/' or '\'
    last_slash = strrchr(path, '/');
    last_backslash = strrchr(path, '\\');
    
    // Return the part after the last slash/backslash
    const char* filename = path;
    if (last_slash && (!last_backslash || last_slash > last_backslash)) {
        filename = last_slash + 1;
    } else if (last_backslash) {
        filename = last_backslash + 1;
    }
    
    return filename;
}

inline void log_with_caller_info(LogLevel level, const std::string& message, 
                         const char* target, const char* filename, uint32_t line_number) {
    // Convert SpacetimeDb::LogLevel to uint8_t for ABI
    uint8_t abi_level = static_cast<uint8_t>(level);
    
    // Handle nullptr parameters
    const char* safe_target = target ? target : "";
    const char* safe_filename = extract_filename(filename);
    
    // Call the enhanced console_log function with caller information
    console_log(abi_level,
                reinterpret_cast<const uint8_t*>(safe_target), std::strlen(safe_target),
                reinterpret_cast<const uint8_t*>(safe_filename), std::strlen(safe_filename),
                line_number,
                reinterpret_cast<const uint8_t*>(message.c_str()), message.length());
}

inline void log(LogLevel level, const std::string& message) {
    // Legacy API - use enhanced version with empty caller info
    log_with_caller_info(level, message, nullptr, nullptr, 0);
}

inline void log_error(const std::string& message) {
    log(LogLevel::Error, message);
}

inline void log_warn(const std::string& message) {
    log(LogLevel::Warn, message);
}

inline void log_info(const std::string& message) {
    log(LogLevel::Info, message);
}

inline void log_debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

inline void log_trace(const std::string& message) {
    log(LogLevel::Trace, message);
}

// LogStopwatch inline implementations
inline LogStopwatch::LogStopwatch(const std::string& name) : ended_(false) {
    timer_id_ = console_timer_start(
        reinterpret_cast<const uint8_t*>(name.c_str()),
        name.length()
    );
}

inline LogStopwatch::~LogStopwatch() {
    if (!ended_) {
        end();
    }
}

inline void LogStopwatch::end() {
    if (!ended_) {
        uint16_t status = console_timer_end(timer_id_);
        ended_ = true;
        // TODO: Add error handling when we implement exception system
        (void)status; // Suppress unused variable warning for now
    }
}

inline LogStopwatch::LogStopwatch(LogStopwatch&& other) noexcept 
    : timer_id_(other.timer_id_), ended_(other.ended_) {
    other.ended_ = true; // Prevent double-ending
}

inline LogStopwatch& LogStopwatch::operator=(LogStopwatch&& other) noexcept {
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

} // namespace SpacetimeDb

#endif // SPACETIMEDB_LIBRARY_LOGGING_H
