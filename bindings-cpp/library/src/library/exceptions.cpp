#include "spacetimedb/library/exceptions.h"
#include "spacetimedb/library/logging.h"

namespace SpacetimeDB {

void throw_error(Errno error_code, const std::string& message) {
    // Log the error before throwing (for debugging)
    LOG_ERROR("SpacetimeDB error occurred: " + std::to_string(static_cast<int16_t>(error_code)) + 
              (message.empty() ? "" : " - " + message));
    
    // Throw the appropriate exception based on error code
    switch (error_code) {
        case Errno::NOT_IN_TRANSACTION:
            throw NotInTransactionException(message);
        
        case Errno::BSATN_DECODE_ERROR:
            throw BsatnDecodeException(message);
        
        case Errno::NO_SUCH_TABLE:
            throw NoSuchTableException(message);
        
        case Errno::NO_SUCH_INDEX:
            throw NoSuchIndexException(message);
        
        case Errno::INDEX_NOT_UNIQUE:
            throw IndexNotUniqueException(message);
        
        case Errno::NO_SUCH_ROW:
            throw NoSuchRowException(message);
        
        case Errno::UNIQUE_ALREADY_EXISTS:
            throw UniqueConstraintViolationException(message);
        
        case Errno::SCHEDULE_AT_DELAY_TOO_LONG:
            throw ScheduleAtDelayTooLongException(message);
        
        case Errno::BUFFER_TOO_SMALL:
            throw BufferTooSmallException(message);
        
        case Errno::NO_SUCH_ITER:
            throw NoSuchIterException(message);
        
        case Errno::NO_SUCH_CONSOLE_TIMER:
            throw NoSuchLogStopwatchException(message);
        
        case Errno::NO_SUCH_BYTES:
            throw NoSuchBytesException(message);
        
        case Errno::NO_SPACE:
            throw NoSpaceException(message);
        
        case Errno::HOST_CALL_FAILURE:
            throw HostCallFailureException(message);
        
        default:
            throw UnknownException(error_code, message.empty() ? 
                "Unknown error code: " + std::to_string(static_cast<int16_t>(error_code)) : message);
    }
}

} // namespace SpacetimeDB