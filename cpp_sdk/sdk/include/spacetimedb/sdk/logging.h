#ifndef SPACETIMEDB_SDK_LOGGING_H
#define SPACETIMEDB_SDK_LOGGING_H

#include "spacetimedb/abi/common_defs.h" // For SpacetimeDB::Abi::LogLevelCpp
#include <string>

namespace SpacetimeDB {

// Re-expose LogLevelCpp from common_defs.h for convenience if desired, or use it directly via SpacetimeDB::Abi::LogLevelCpp
using LogLevel = SpacetimeDB::Abi::LogLevelCpp;

/**
 * @brief Logs a message through the SpacetimeDB host environment.
 * @param level The severity level of the message, using `SpacetimeDB::LogLevel`.
 * @param message The message string to log.
 * @ingroup sdk_runtime sdk_logging
 */
void log(LogLevel level, const std::string& message);

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

} // namespace SpacetimeDB

#endif // SPACETIMEDB_SDK_LOGGING_H
