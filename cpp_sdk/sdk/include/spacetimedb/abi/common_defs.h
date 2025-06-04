#ifndef SPACETIMEDB_ABI_COMMON_DEFS_H
#define SPACETIMEDB_ABI_COMMON_DEFS_H

#include <cstdint> // For uint16_t, uint8_t

// These definitions are designed to mirror the opaque types and enums
// found in crates/bindings-csharp/Runtime/bindings.c for ABI compatibility.

#ifdef __cplusplus
extern "C" {
#endif

// OPAQUE_TYPEDEF(Status, uint16_t);
typedef struct { uint16_t inner; } Status;

// OPAQUE_TYPEDEF(TableId, uint32_t);
typedef struct { uint32_t inner; } TableId;

// OPAQUE_TYPEDEF(IndexId, uint32_t);
typedef struct { uint32_t inner; } IndexId;

// OPAQUE_TYPEDEF(ColId, uint16_t);
typedef struct { uint16_t inner; } ColId;

// OPAQUE_TYPEDEF(IndexType, uint8_t);
typedef struct { uint8_t inner; } IndexType; // Range, Hash etc. (specific values TBD by schema)

// OPAQUE_TYPEDEF(LogLevel, uint8_t);
typedef struct { uint8_t inner; } LogLevel; // Error = 0, Warn = 1, Info = 2, Debug = 3, Trace = 4

// OPAQUE_TYPEDEF(BytesSink, uint32_t);
typedef struct { uint32_t inner; } BytesSink;

// OPAQUE_TYPEDEF(BytesSource, uint32_t);
typedef struct { uint32_t inner; } BytesSource;

// OPAQUE_TYPEDEF(RowIter, uint32_t);
typedef struct { uint32_t inner; } RowIter;

// OPAQUE_TYPEDEF(ConsoleTimerId, uint32_t);
typedef struct { uint32_t inner; } ConsoleTimerId;

#ifdef __cplusplus
} // extern "C"

// C++ helper functions or operators for these types (optional)
namespace SpacetimeDB {
namespace Abi {
    // Allow easy conversion and comparison for C++ code
    inline bool operator==(const ::Status& lhs, const ::Status& rhs) { return lhs.inner == rhs.inner; }
    inline bool operator!=(const ::Status& lhs, const ::Status& rhs) { return lhs.inner != rhs.inner; }
    inline bool is_ok(::Status s) { return s.inner == 0; } // Assuming 0 is OK

    inline bool operator==(const ::LogLevel& lhs, const ::LogLevel& rhs) { return lhs.inner == rhs.inner; }
    inline bool operator!=(const ::LogLevel& lhs, const ::LogLevel& rhs) { return lhs.inner != rhs.inner; }

    // It can be useful to define C++ enum class versions that map to these ABI types
    enum class StatusCpp : uint16_t {
        Ok = 0,
        ErrorGeneric = 1, // Example
        ErrorNotFound = 2,
        ErrorInvalidArgument = 3,
        // ... other status codes ...
    };
    inline Status to_abi(StatusCpp s) { return {static_cast<uint16_t>(s)}; }
    inline StatusCpp from_abi(Status s) { return static_cast<StatusCpp>(s.inner); }


    enum class LogLevelCpp : uint8_t {
        Error = 0,
        Warn = 1,
        Info = 2,
        Debug = 3,
        Trace = 4
    };
    inline LogLevel to_abi(LogLevelCpp l) { return {static_cast<uint8_t>(l)}; }
    inline LogLevelCpp from_abi(LogLevel l) { return static_cast<LogLevelCpp>(l.inner); }

} // namespace Abi
} // namespace SpacetimeDB

#endif // __cplusplus

#endif // SPACETIMEDB_ABI_COMMON_DEFS_H
