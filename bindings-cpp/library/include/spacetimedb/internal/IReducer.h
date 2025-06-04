#ifndef SPACETIMEDB_INTERNAL_IREDUCER_H
#define SPACETIMEDB_INTERNAL_IREDUCER_H

#include <memory>
#include <vector>
#include <optional>
#include "spacetimedb/bsatn/bsatn.h"
#include "spacetimedb/internal/autogen/RawReducerDefV9.g.h"
#include "spacetimedb/library/spacetimedb_library_types.h"

namespace SpacetimeDb {
namespace Internal {

// Forward declarations
class ITypeRegistrar;
class IReducerContext;

// Base interface for reducers
class IReducer {
public:
    virtual ~IReducer() = default;
    
    // Create the reducer definition for module registration
    virtual RawReducerDefV9 MakeReducerDef(ITypeRegistrar& registrar) const = 0;
    
    // Invoke the reducer with deserialized arguments
    virtual void Invoke(bsatn::Reader& reader, IReducerContext& ctx) = 0;
};

// Interface for reducer context - provides access to database and metadata
class IReducerContext {
public:
    virtual ~IReducerContext() = default;
    
    // Get the sender identity
    virtual library::Identity GetSender() const = 0;
    
    // Get the connection ID (if available)
    virtual std::optional<library::ConnectionId> GetConnectionId() const = 0;
    
    // Get the timestamp
    virtual library::Timestamp GetTimestamp() const = 0;
    
    // Convert to Module Library context for user code
    virtual spacetimedb::ReducerContext ToSdkContext() = 0;
    
    // TODO: Add database access methods
};

} // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_IREDUCER_H