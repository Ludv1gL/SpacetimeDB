/**
 * SpacetimeDB C++ SDK Connect/Disconnect Test Module
 * 
 * Tests client connection and disconnection lifecycle events.
 */

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/spacetimedb_autogen.h>

using namespace spacetimedb;

// Track connected clients
struct ConnectedClient {
    SpacetimeDb::sdk::Identity identity;
    SpacetimeDb::sdk::ConnectionId connection_id;
    SpacetimeDb::sdk::Timestamp connected_at;
    std::string status;  // "connected" or "disconnected"
};

SPACETIMEDB_REGISTER_TYPE(ConnectedClient,
    SPACETIMEDB_FIELD(ConnectedClient, identity),
    SPACETIMEDB_FIELD(ConnectedClient, connection_id),
    SPACETIMEDB_FIELD(ConnectedClient, connected_at),
    SPACETIMEDB_FIELD(ConnectedClient, status)
)

SPACETIMEDB_TABLE(ConnectedClient, "connected_clients", true)

// Track all events
struct ConnectionEvent {
    uint64_t id;
    SpacetimeDb::sdk::Identity identity;
    std::string event_type;  // "connect", "disconnect", "reducer_call"
    SpacetimeDb::sdk::Timestamp timestamp;
    std::optional<std::string> details;
};

SPACETIMEDB_REGISTER_TYPE(ConnectionEvent,
    SPACETIMEDB_FIELD(ConnectionEvent, id),
    SPACETIMEDB_FIELD(ConnectionEvent, identity),
    SPACETIMEDB_FIELD(ConnectionEvent, event_type),
    SPACETIMEDB_FIELD(ConnectionEvent, timestamp),
    SPACETIMEDB_FIELD(ConnectionEvent, details)
)

SPACETIMEDB_TABLE(ConnectionEvent, "connection_events", true)
SPACETIMEDB_INDEX(ConnectionEvent, idx_identity, identity)
SPACETIMEDB_INDEX(ConnectionEvent, idx_event_type, event_type)

// Counter for event IDs
static uint64_t next_event_id = 1;

// Helper to log an event
void log_event(const SpacetimeDb::sdk::Identity& identity, 
               const std::string& event_type,
               const std::optional<std::string>& details = std::nullopt) {
    ConnectionEvent event{
        .id = next_event_id++,
        .identity = identity,
        .event_type = event_type,
        .timestamp = SpacetimeDb::sdk::Timestamp::now(),
        .details = details
    };
    
    auto event_table = get_ConnectionEvent_table();
    event_table.insert(event);
}

// =============================================================================
// LIFECYCLE REDUCERS
// =============================================================================

SPACETIMEDB_INIT(init, ctx)
{
    LOG_INFO("Connect/disconnect test module initialized");
    next_event_id = 1;
}

SPACETIMEDB_CLIENT_CONNECTED(on_connect, ctx)
{
    LOG_INFO("Client connected: " + ctx.sender.to_string());
    
    // Add to connected clients table
    ConnectedClient client{
        .identity = ctx.sender,
        .connection_id = ctx.connection_id,
        .connected_at = SpacetimeDb::sdk::Timestamp::now(),
        .status = "connected"
    };
    
    auto client_table = get_ConnectedClient_table();
    client_table.insert(client);
    
    // Log the event
    log_event(ctx.sender, "connect", "Connection ID: " + std::to_string(ctx.connection_id.id));
}

SPACETIMEDB_CLIENT_DISCONNECTED(on_disconnect, ctx)
{
    LOG_INFO("Client disconnected: " + ctx.sender.to_string());
    
    // Update client status
    auto client_table = get_ConnectedClient_table();
    for (auto& client : client_table.iter()) {
        if (client.identity == ctx.sender && client.connection_id == ctx.connection_id) {
            auto old_client = client;
            client.status = "disconnected";
            client_table.update(old_client, client);
            break;
        }
    }
    
    // Log the event
    log_event(ctx.sender, "disconnect", "Connection ID: " + std::to_string(ctx.connection_id.id));
}

// =============================================================================
// TEST REDUCERS
// =============================================================================

SPACETIMEDB_REDUCER(test_connection, UserDefined, ctx)
{
    LOG_INFO("Test reducer called by: " + ctx.sender.to_string());
    log_event(ctx.sender, "reducer_call", "test_connection");
}

SPACETIMEDB_REDUCER(get_connection_status, UserDefined, ctx)
{
    auto client_table = get_ConnectedClient_table();
    uint32_t connected = 0;
    uint32_t disconnected = 0;
    
    for (const auto& client : client_table.iter()) {
        if (client.identity == ctx.sender) {
            if (client.status == "connected") {
                connected++;
            } else {
                disconnected++;
            }
        }
    }
    
    LOG_INFO("Connection status - Connected: " + std::to_string(connected) + 
             ", Disconnected: " + std::to_string(disconnected));
    
    log_event(ctx.sender, "reducer_call", 
              "get_connection_status - Connected: " + std::to_string(connected));
}

SPACETIMEDB_REDUCER(get_all_events, UserDefined, ctx, std::string event_type)
{
    auto event_table = get_ConnectionEvent_table();
    uint32_t count = 0;
    
    for (const auto& event : event_table.iter()) {
        if (event.event_type == event_type) {
            count++;
        }
    }
    
    LOG_INFO("Found " + std::to_string(count) + " events of type: " + event_type);
    log_event(ctx.sender, "reducer_call", 
              "get_all_events - Type: " + event_type + ", Count: " + std::to_string(count));
}

SPACETIMEDB_REDUCER(clear_old_events, UserDefined, ctx, uint64_t keep_recent_count)
{
    auto event_table = get_ConnectionEvent_table();
    
    // Get all events sorted by ID (descending)
    std::vector<ConnectionEvent> all_events;
    for (const auto& event : event_table.iter()) {
        all_events.push_back(event);
    }
    
    // Sort by ID descending
    std::sort(all_events.begin(), all_events.end(), 
              [](const auto& a, const auto& b) { return a.id > b.id; });
    
    // Delete old events
    uint32_t deleted = 0;
    for (size_t i = keep_recent_count; i < all_events.size(); ++i) {
        if (event_table.delete_row(all_events[i])) {
            deleted++;
        }
    }
    
    LOG_INFO("Deleted " + std::to_string(deleted) + " old events");
    log_event(ctx.sender, "reducer_call", 
              "clear_old_events - Deleted: " + std::to_string(deleted));
}

// =============================================================================
// MODULE METADATA
// =============================================================================

SPACETIMEDB_MODULE_METADATA(
    "sdk-test-connect-disconnect-cpp",
    "SpacetimeDB",
    "C++ SDK connection lifecycle test module",
    "Apache-2.0"
)

SPACETIMEDB_MODULE_VERSION(1, 0, 0)