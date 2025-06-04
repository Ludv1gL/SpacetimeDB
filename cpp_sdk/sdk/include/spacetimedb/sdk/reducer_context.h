#ifndef REDUCER_CONTEXT_H
#define REDUCER_CONTEXT_H

#include <spacetimedb/sdk/spacetimedb_sdk_types.h> // For Identity, Timestamp, ConnectionId
#include <optional>
#include <array>

namespace SpacetimeDb {
namespace sdk {

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

} // namespace sdk
} // namespace SpacetimeDb

// Alias for consistency with old code
namespace spacetimedb {
    using ReducerContext = SpacetimeDb::sdk::ReducerContext;
}

#endif // REDUCER_CONTEXT_H
