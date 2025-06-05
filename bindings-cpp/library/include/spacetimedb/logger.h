#ifndef SPACETIMEDB_LIBRARY_LOGGING_H
#define SPACETIMEDB_LIBRARY_LOGGING_H

#include "spacetimedb/abi/spacetimedb_abi.h" // For LogLevel
#include <string>
#include <chrono>
#include <memory>

namespace SpacetimeDb {

// Re-expose LogLevel from spacetimedb_abi.h for convenience
using LogLevel = Abi::LogLevel;

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
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::ERROR, (message), __func__, __FILE__, __LINE__)

#define LOG_WARN(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::WARN, (message), __func__, __FILE__, __LINE__)

#define LOG_INFO(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::INFO, (message), __func__, __FILE__, __LINE__)

#define LOG_DEBUG(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::DEBUG, (message), __func__, __FILE__, __LINE__)

#define LOG_TRACE(message) \
    SpacetimeDb::log_with_caller_info(SpacetimeDb::LogLevel::TRACE, (message), __func__, __FILE__, __LINE__)

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

// Function implementations are in logging.cpp

// LogStopwatch implementations are in logging.cpp

} // namespace SpacetimeDb

#endif // SPACETIMEDB_LIBRARY_LOGGING_H
