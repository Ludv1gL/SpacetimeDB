#ifndef SPACETIMEDB_SDK_EXCEPTIONS_H
#define SPACETIMEDB_SDK_EXCEPTIONS_H

#include <exception>
#include <string>
#include <cstdint>

namespace SpacetimeDB {

/**
 * @brief Error codes returned by SpacetimeDB host functions.
 * 
 * Maps to the error codes defined in spacetimedb_primitives::errno.
 * @ingroup sdk_runtime sdk_errors
 */
enum class Errno : int16_t {
    EXHAUSTED = -1,                     ///< Iterator exhausted
    OK = 0,                            ///< Success
    HOST_CALL_FAILURE = 1,             ///< General host call failure
    NOT_IN_TRANSACTION = 2,            ///< Operation requires transaction
    BSATN_DECODE_ERROR = 3,            ///< Serialization error
    NO_SUCH_TABLE = 4,                 ///< Table not found
    NO_SUCH_INDEX = 5,                 ///< Index not found
    NO_SUCH_ITER = 6,                  ///< Iterator not found
    NO_SUCH_CONSOLE_TIMER = 7,         ///< Timer not found
    NO_SUCH_BYTES = 8,                 ///< Bytes handle not found
    NO_SPACE = 9,                      ///< No space available
    BUFFER_TOO_SMALL = 11,             ///< Buffer size insufficient
    UNIQUE_ALREADY_EXISTS = 12,        ///< Unique constraint violation
    SCHEDULE_AT_DELAY_TOO_LONG = 13,   ///< Schedule delay too long
    INDEX_NOT_UNIQUE = 14,             ///< Index not unique
    NO_SUCH_ROW = 15,                  ///< Row not found
};

/**
 * @brief Base exception class for all SpacetimeDB-specific errors.
 * 
 * Provides a common base for all SpacetimeDB exceptions with
 * automatic error code preservation and consistent error messaging.
 * 
 * @ingroup sdk_runtime sdk_errors
 */
class StdbException : public std::exception {
public:
    /**
     * @brief Construct exception with error code and message.
     * @param error_code The SpacetimeDB error code.
     * @param message Custom error message.
     */
    explicit StdbException(Errno error_code, const std::string& message = "")
        : error_code_(error_code), message_(message) {}
    
    /**
     * @brief Get the error message.
     * @return C-style string containing the error message.
     */
    const char* what() const noexcept override {
        if (message_.empty()) {
            return get_default_message();
        }
        return message_.c_str();
    }
    
    /**
     * @brief Get the SpacetimeDB error code.
     * @return The error code that caused this exception.
     */
    Errno error_code() const noexcept { return error_code_; }
    
    /**
     * @brief Get the numeric error code value.
     * @return The numeric value of the error code.
     */
    int16_t error_value() const noexcept { return static_cast<int16_t>(error_code_); }

protected:
    /**
     * @brief Get the default error message for this exception type.
     * @return Default error message.
     */
    virtual const char* get_default_message() const noexcept = 0;

private:
    Errno error_code_;
    std::string message_;
};

/**
 * @brief Exception thrown when ABI calls are made outside of a transaction.
 * @ingroup sdk_runtime sdk_errors
 */
class NotInTransactionException : public StdbException {
public:
    explicit NotInTransactionException(const std::string& message = "")
        : StdbException(Errno::NOT_IN_TRANSACTION, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Operation requires a transaction context";
    }
};

/**
 * @brief Exception thrown when BSATN decoding fails.
 * @ingroup sdk_runtime sdk_errors
 */
class BsatnDecodeException : public StdbException {
public:
    explicit BsatnDecodeException(const std::string& message = "")
        : StdbException(Errno::BSATN_DECODE_ERROR, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Failed to decode BSATN data";
    }
};

/**
 * @brief Exception thrown when a table is not found.
 * @ingroup sdk_runtime sdk_errors
 */
class NoSuchTableException : public StdbException {
public:
    explicit NoSuchTableException(const std::string& message = "")
        : StdbException(Errno::NO_SUCH_TABLE, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Table not found";
    }
};

/**
 * @brief Exception thrown when an index is not found.
 * @ingroup sdk_runtime sdk_errors
 */
class NoSuchIndexException : public StdbException {
public:
    explicit NoSuchIndexException(const std::string& message = "")
        : StdbException(Errno::NO_SUCH_INDEX, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Index not found";
    }
};

/**
 * @brief Exception thrown when an index was expected to be unique but isn't.
 * @ingroup sdk_runtime sdk_errors
 */
class IndexNotUniqueException : public StdbException {
public:
    explicit IndexNotUniqueException(const std::string& message = "")
        : StdbException(Errno::INDEX_NOT_UNIQUE, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Index is not unique";
    }
};

/**
 * @brief Exception thrown when a row is not found.
 * @ingroup sdk_runtime sdk_errors
 */
class NoSuchRowException : public StdbException {
public:
    explicit NoSuchRowException(const std::string& message = "")
        : StdbException(Errno::NO_SUCH_ROW, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Row not found";
    }
};

/**
 * @brief Exception thrown when a unique constraint is violated.
 * @ingroup sdk_runtime sdk_errors
 */
class UniqueConstraintViolationException : public StdbException {
public:
    explicit UniqueConstraintViolationException(const std::string& message = "")
        : StdbException(Errno::UNIQUE_ALREADY_EXISTS, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Unique constraint violation";
    }
};

/**
 * @brief Exception thrown when a scheduling delay is too long.
 * @ingroup sdk_runtime sdk_errors
 */
class ScheduleAtDelayTooLongException : public StdbException {
public:
    explicit ScheduleAtDelayTooLongException(const std::string& message = "")
        : StdbException(Errno::SCHEDULE_AT_DELAY_TOO_LONG, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Schedule delay is too long";
    }
};

/**
 * @brief Exception thrown when a buffer is too small for the requested operation.
 * @ingroup sdk_runtime sdk_errors
 */
class BufferTooSmallException : public StdbException {
public:
    explicit BufferTooSmallException(const std::string& message = "")
        : StdbException(Errno::BUFFER_TOO_SMALL, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Buffer is too small for the requested operation";
    }
};

/**
 * @brief Exception thrown when a row iterator does not exist.
 * @ingroup sdk_runtime sdk_errors
 */
class NoSuchIterException : public StdbException {
public:
    explicit NoSuchIterException(const std::string& message = "")
        : StdbException(Errno::NO_SUCH_ITER, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Row iterator does not exist";
    }
};

/**
 * @brief Exception thrown when a log stopwatch does not exist.
 * @ingroup sdk_runtime sdk_errors
 */
class NoSuchLogStopwatchException : public StdbException {
public:
    explicit NoSuchLogStopwatchException(const std::string& message = "")
        : StdbException(Errno::NO_SUCH_CONSOLE_TIMER, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Log stopwatch does not exist";
    }
};

/**
 * @brief Exception thrown when a bytes source/sink does not exist.
 * @ingroup sdk_runtime sdk_errors
 */
class NoSuchBytesException : public StdbException {
public:
    explicit NoSuchBytesException(const std::string& message = "")
        : StdbException(Errno::NO_SUCH_BYTES, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Bytes source/sink does not exist";
    }
};

/**
 * @brief Exception thrown when a bytes sink has no more room.
 * @ingroup sdk_runtime sdk_errors
 */
class NoSpaceException : public StdbException {
public:
    explicit NoSpaceException(const std::string& message = "")
        : StdbException(Errno::NO_SPACE, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Bytes sink has no more room";
    }
};

/**
 * @brief Exception thrown for general host call failures.
 * @ingroup sdk_runtime sdk_errors
 */
class HostCallFailureException : public StdbException {
public:
    explicit HostCallFailureException(const std::string& message = "")
        : StdbException(Errno::HOST_CALL_FAILURE, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Host call failed";
    }
};

/**
 * @brief Exception thrown for unknown or unrecognized error codes.
 * @ingroup sdk_runtime sdk_errors
 */
class UnknownException : public StdbException {
public:
    explicit UnknownException(Errno error_code, const std::string& message = "")
        : StdbException(error_code, message) {}

protected:
    const char* get_default_message() const noexcept override {
        return "Unknown error occurred";
    }
};

/**
 * @brief Converts an error code to the appropriate exception.
 * 
 * This function implements automatic error marshalling by converting
 * SpacetimeDB error codes into their corresponding exception types.
 * 
 * @param error_code The error code to convert.
 * @param message Optional custom error message.
 * @throws The appropriate SpacetimeDB exception based on the error code.
 * @ingroup sdk_runtime sdk_errors
 */
[[noreturn]] void throw_error(Errno error_code, const std::string& message = "");

/**
 * @brief Checks a return code and throws an exception if it indicates an error.
 * 
 * This function provides automatic error checking for SpacetimeDB host calls.
 * If the return code indicates success (0), nothing happens. If it indicates
 * an error, the appropriate exception is thrown.
 * 
 * @param return_code The return code from a host function.
 * @param message Optional custom error message.
 * @throws The appropriate SpacetimeDB exception if return_code != 0.
 * @ingroup sdk_runtime sdk_errors
 */
inline void check_error(uint16_t return_code, const std::string& message = "") {
    if (return_code != 0) {
        throw_error(static_cast<Errno>(return_code), message);
    }
}

/**
 * @brief RAII helper for automatic resource cleanup on exceptions.
 * 
 * This class provides automatic cleanup of resources when exceptions
 * are thrown, following the RAII pattern.
 * 
 * @tparam Cleanup A callable type for the cleanup function.
 * @ingroup sdk_runtime sdk_errors
 */
template<typename Cleanup>
class ScopeGuard {
public:
    explicit ScopeGuard(Cleanup&& cleanup) 
        : cleanup_(std::move(cleanup)), active_(true) {}
    
    ~ScopeGuard() {
        if (active_) {
            cleanup_();
        }
    }
    
    /**
     * @brief Release the guard without executing cleanup.
     */
    void release() { active_ = false; }
    
    // Non-copyable, moveable
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept 
        : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
        other.active_ = false;
    }
    ScopeGuard& operator=(ScopeGuard&& other) noexcept {
        if (this != &other) {
            if (active_) cleanup_();
            cleanup_ = std::move(other.cleanup_);
            active_ = other.active_;
            other.active_ = false;
        }
        return *this;
    }

private:
    Cleanup cleanup_;
    bool active_;
};

/**
 * @brief Creates a scope guard for automatic resource cleanup.
 * 
 * @tparam Cleanup The type of the cleanup function.
 * @param cleanup The cleanup function to execute on scope exit.
 * @return A ScopeGuard object that will execute cleanup on destruction.
 * @ingroup sdk_runtime sdk_errors
 */
template<typename Cleanup>
auto make_scope_guard(Cleanup&& cleanup) {
    return ScopeGuard<Cleanup>(std::forward<Cleanup>(cleanup));
}

} // namespace SpacetimeDB

#endif // SPACETIMEDB_SDK_EXCEPTIONS_H