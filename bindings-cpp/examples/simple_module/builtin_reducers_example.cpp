/**
 * Example demonstrating the use of built-in reducers in SpacetimeDB C++ SDK
 * 
 * This module shows how to use:
 * - init: Called when the module is first initialized
 * - client_connected: Called when a client connects
 * - client_disconnected: Called when a client disconnects
 */

#include <spacetimedb/spacetimedb.h>

// Define tables to track connections
struct Connection {
    SpacetimeDb::Identity identity;
    uint64_t connect_time;
};

struct Disconnection {
    SpacetimeDb::Identity identity;
    uint64_t disconnect_time;
};

struct InitLog {
    std::string message;
    uint64_t timestamp;
};

// Register fields for complex types
SPACETIMEDB_REGISTER_FIELDS(Connection,
    SPACETIMEDB_FIELD(Connection, identity, SpacetimeDb::Identity);
    SPACETIMEDB_FIELD(Connection, connect_time, uint64_t);
)

SPACETIMEDB_REGISTER_FIELDS(Disconnection,
    SPACETIMEDB_FIELD(Disconnection, identity, SpacetimeDb::Identity);
    SPACETIMEDB_FIELD(Disconnection, disconnect_time, uint64_t);
)

SPACETIMEDB_REGISTER_FIELDS(InitLog,
    SPACETIMEDB_FIELD(InitLog, message, std::string);
    SPACETIMEDB_FIELD(InitLog, timestamp, uint64_t);
)

// Register tables
SPACETIMEDB_TABLE(Connection, connections, true)
SPACETIMEDB_TABLE(Disconnection, disconnections, true)
SPACETIMEDB_TABLE(InitLog, init_logs, true)

// Built-in reducer: init
// This is called when the module is first initialized
SPACETIMEDB_INIT(on_module_init) {
    LOG_INFO("Module initialized!");
    
    InitLog log{
        "Module initialized successfully",
        0  // In a real module, you'd get this from ctx.timestamp
    };
    
    ctx.db.table<InitLog>("init_logs").insert(log);
}

// Built-in reducer: client_connected
// This is called when a client connects to the module
SPACETIMEDB_CLIENT_CONNECTED(on_client_connect) {
    LOG_INFO("Client connected: " + sender.to_hex_string());
    
    Connection conn{
        sender,
        0  // In a real module, you'd get this from ctx.timestamp
    };
    
    ctx.db.table<Connection>("connections").insert(conn);
}

// Built-in reducer: client_disconnected
// This is called when a client disconnects from the module
SPACETIMEDB_CLIENT_DISCONNECTED(on_client_disconnect) {
    LOG_INFO("Client disconnected: " + sender.to_hex_string());
    
    Disconnection disc{
        sender,
        0  // In a real module, you'd get this from ctx.timestamp
    };
    
    ctx.db.table<Disconnection>("disconnections").insert(disc);
}

// Regular reducer to query connection history
SPACETIMEDB_REDUCER(get_connection_count, SpacetimeDb::ReducerContext ctx) {
    auto connections_table = ctx.db.table<Connection>("connections");
    auto disconnections_table = ctx.db.table<Disconnection>("disconnections");
    
    uint64_t connect_count = connections_table.count();
    uint64_t disconnect_count = disconnections_table.count();
    
    LOG_INFO("Total connections: " + std::to_string(connect_count));
    LOG_INFO("Total disconnections: " + std::to_string(disconnect_count));
    LOG_INFO("Currently connected: " + std::to_string(connect_count - disconnect_count));
}

// Regular reducer to check if a specific client is connected
SPACETIMEDB_REDUCER(is_client_connected, SpacetimeDb::ReducerContext ctx, std::string identity_hex) {
    // In a real implementation, you would:
    // 1. Parse the hex string to Identity
    // 2. Check if they're in connections but not in disconnections
    // 3. Return the result
    
    LOG_INFO("Checking connection status for: " + identity_hex);
    // Implementation left as an exercise
}