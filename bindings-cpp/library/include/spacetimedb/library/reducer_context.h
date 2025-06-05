#ifndef REDUCER_CONTEXT_H
#define REDUCER_CONTEXT_H

#include <spacetimedb/library/spacetimedb_library_types.h> // For Identity, Timestamp, ConnectionId
#include <optional>
#include <array>

namespace SpacetimeDb {
namespace library {

// Forward declaration
class Database;

// Simple ReducerContext for hybrid approach - matches Rust pattern
struct ReducerContext {
    // Core fields - directly accessible like in Rust
    Identity sender;
    std::optional<ConnectionId> connection_id;
    Timestamp timestamp;
    
    // Database access - will be added later
    // Database& db;
    
    // Constructor
    ReducerContext() = default;
    
    ReducerContext(Identity s, std::optional<ConnectionId> cid, Timestamp ts)
        : sender(s), connection_id(cid), timestamp(ts) {}
};

} // namespace library
} // namespace SpacetimeDb

// Alias for consistency with old code
namespace SpacetimeDb {
    using ReducerContext = SpacetimeDb::library::ReducerContext;
}

#endif // REDUCER_CONTEXT_H
