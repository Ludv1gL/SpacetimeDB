#ifndef REDUCER_CONTEXT_H
#define REDUCER_CONTEXT_H

#include <spacetimedb/types.h> // For Identity, ConnectionId
#include <spacetimedb/timestamp.h> // For Timestamp
#include <optional>
#include <array>

namespace SpacetimeDb {

// Forward declaration
class Database;

} // namespace SpacetimeDb

// Forward declare ModuleDatabase from the global namespace
class ModuleDatabase;

namespace SpacetimeDb {

// Simple ReducerContext for hybrid approach - matches Rust pattern
struct ReducerContext {
    // Core fields - directly accessible like in Rust
    Identity sender;
    std::optional<ConnectionId> connection_id;
    Timestamp timestamp;
    
    // Database access - implemented as a function to avoid circular dependency
    // The actual db member is created inline in spacetimedb.h after ModuleDatabase is defined
    inline ::ModuleDatabase& get_db();
    
    // Property-like accessor - just make it a simple pointer-like interface
    struct {
        ReducerContext* ctx;
        ::ModuleDatabase* operator->() { return &ctx->get_db(); }
        ::ModuleDatabase& operator*() { return ctx->get_db(); }
    } db{this};
    
    // Constructor
    ReducerContext() : db{this} {}
    
    ReducerContext(Identity s, std::optional<ConnectionId> cid, Timestamp ts)
        : sender(s), connection_id(cid), timestamp(ts), db{this} {}
        
    ReducerContext(Identity s, Timestamp ts, Database& database)
        : sender(s), timestamp(ts), db{this} {}
    
    // Table operations
    template<typename T>
    void insert(const T& row) {
        // This is a placeholder - in a real implementation this would
        // call the FFI functions to insert into the database
        // For now, we'll do nothing to allow compilation
    }
    
    template<typename T>
    void update(const T& row) {
        // Placeholder for table updates
    }
    
    template<typename T>
    void delete_row(const T& row) {
        // Placeholder for table deletes
    }
};

} // namespace SpacetimeDb

#endif // REDUCER_CONTEXT_H
