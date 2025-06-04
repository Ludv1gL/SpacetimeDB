/**
 * Simple test for built-in reducers
 * 
 * This minimal example tests the three built-in lifecycle reducers
 */

#include <spacetimedb/spacetimedb.h>

// Simple table to track events
struct Event {
    uint8_t event_type;  // 0=init, 1=connect, 2=disconnect
    uint8_t data[32];    // Store identity bytes for connect/disconnect
};

SPACETIMEDB_TABLE(Event, events, true)

// Init reducer - called on module initialization
SPACETIMEDB_INIT(module_init) {
    Event evt{0, {0}};  // event_type = 0 for init
    ctx.db.table<Event>("events").insert(evt);
}

// Client connected reducer
SPACETIMEDB_CLIENT_CONNECTED(client_conn) {
    Event evt{1, {0}};  // event_type = 1 for connect
    
    // Copy identity bytes to data field
    auto identity_bytes = sender.to_byte_array();
    std::memcpy(evt.data, identity_bytes.data(), 32);
    
    ctx.db.table<Event>("events").insert(evt);
}

// Client disconnected reducer
SPACETIMEDB_CLIENT_DISCONNECTED(client_disc) {
    Event evt{2, {0}};  // event_type = 2 for disconnect
    
    // Copy identity bytes to data field
    auto identity_bytes = sender.to_byte_array();
    std::memcpy(evt.data, identity_bytes.data(), 32);
    
    ctx.db.table<Event>("events").insert(evt);
}

// Regular reducer to count events
SPACETIMEDB_REDUCER(count_events, spacetimedb::ReducerContext ctx) {
    auto events_table = ctx.db.table<Event>("events");
    uint64_t count = events_table.count();
    
    // Log the count (in a real module, you might return this somehow)
    LOG_INFO("Total events: " + std::to_string(count));
}