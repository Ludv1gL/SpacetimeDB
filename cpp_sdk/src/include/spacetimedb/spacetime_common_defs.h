#ifndef SPACETIME_COMMON_DEFS_H
#define SPACETIME_COMMON_DEFS_H

#include <cstdint> // For uint16_t, uint8_t

// These definitions are designed to mirror the opaque types and enums
// found in crates/bindings-csharp/Runtime/bindings.c for ABI compatibility.

#ifdef __cplusplus
extern "C" {
#endif

// OPAQUE_TYPEDEF(Status, uint16_t);
typedef struct { uint16_t inner; } Status;

// OPAQUE_TYPEDEF(TableId, uint16_t);
typedef struct { uint16_t inner; } TableId;

// OPAQUE_TYPEDEF(IndexId, uint16_t);
typedef struct { uint16_t inner; } IndexId;

// OPAQUE_TYPEDEF(ColId, uint16_t);
typedef struct { uint16_t inner; } ColId;

// OPAQUE_TYPEDEF(IndexType, uint8_t);
typedef struct { uint8_t inner; } IndexType; // Range, Hash etc. (specific values TBD by schema)

// OPAQUE_TYPEDEF(LogLevel, uint8_t);
typedef struct { uint8_t inner; } LogLevel; // Error = 0, Warn = 1, Info = 2, Debug = 3, Trace = 4

// OPAQUE_TYPEDEF(BytesSink, uint16_t);
typedef struct { uint16_t inner; } BytesSink;

// OPAQUE_TYPEDEF(BytesSource, uint16_t);
typedef struct { uint16_t inner; } BytesSource;

// OPAQUE_TYPEDEF(RowIter, uint16_t);
typedef struct { uint16_t inner; } RowIter;

// OPAQUE_TYPEDEF(ConsoleTimerId, uint16_t);
typedef struct { uint16_t inner; } ConsoleTimerId;


// Define some common status codes for convenience, if applicable
// These would need to align with the host's definition of Status values.
// Example:
// static const Status STATUS_OK = {0};
// static const Status STATUS_ERROR_GENERIC = {1};
// static const Status STATUS_ERROR_NOT_FOUND = {2};
// static const Status STATUS_ERROR_INVALID_ARG = {3};

#ifdef __cplusplus
} // extern "C"

// C++ helper functions or operators for these types (optional)
namespace SpacetimeDB {
    // Allow easy conversion and comparison for C++ code
    inline bool operator==(const Status& lhs, const Status& rhs) { return lhs.inner == rhs.inner; }
    inline bool operator!=(const Status& lhs, const Status& rhs) { return lhs.inner != rhs.inner; }
    // Example: inline bool is_ok(Status s) { return s.inner == 0; }

    // Similar for other types if needed
    inline bool operator==(const LogLevel& lhs, const LogLevel& rhs) { return lhs.inner == rhs.inner; }
    inline bool operator!=(const LogLevel& lhs, const LogLevel& rhs) { return lhs.inner != rhs.inner; }
}

#endif // __cplusplus

#endif // SPACETIME_COMMON_DEFS_H
