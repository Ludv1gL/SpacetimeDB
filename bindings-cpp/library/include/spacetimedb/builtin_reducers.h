#ifndef SPACETIMEDB_BUILTIN_REDUCERS_H
#define SPACETIMEDB_BUILTIN_REDUCERS_H

/**
 * SpacetimeDB C++ SDK - Built-in Reducers Support
 * 
 * This header provides support for built-in lifecycle reducers:
 * - init: Called when the module is first initialized
 * - client_connected: Called when a client connects
 * - client_disconnected: Called when a client disconnects
 */

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <optional>
#include <cstring>
#include "spacetimedb/types.h"

// Helper macros
#define SPACETIMEDB_CAT_IMPL(a, b) a##b
#define SPACETIMEDB_CAT(a, b) SPACETIMEDB_CAT_IMPL(a, b)

namespace SpacetimeDb {

// -----------------------------------------------------------------------------
// Lifecycle Reducer Enum
// -----------------------------------------------------------------------------

enum class Lifecycle : uint8_t {
    Init = 0,
    OnConnect = 1,
    OnDisconnect = 2
};

// -----------------------------------------------------------------------------
// Enhanced Reducer Context for Built-in Reducers
// -----------------------------------------------------------------------------

// Forward declaration
// ReducerContext is defined in reducer_context_enhanced.h and already includes sender identity
struct ReducerContext;

// -----------------------------------------------------------------------------
// Built-in Reducer Registration Helpers
// -----------------------------------------------------------------------------

namespace detail {

// Helper to determine if a reducer is a built-in lifecycle reducer
inline std::optional<Lifecycle> get_lifecycle_for_name(const std::string& name) {
    if (name == "init" || name == "__init__") {
        return Lifecycle::Init;
    } else if (name == "client_connected" || name == "__client_connected__") {
        return Lifecycle::OnConnect;
    } else if (name == "client_disconnected" || name == "__client_disconnected__") {
        return Lifecycle::OnDisconnect;
    }
    return std::nullopt;
}

// Specialized reducer wrapper for built-in reducers
template<typename Func>
void builtin_reducer_wrapper(Func func, ReducerContext& ctx, 
                           uint64_t sender_0, uint64_t sender_1, 
                           uint64_t sender_2, uint64_t sender_3) {
    // Reconstruct identity from parts
    std::array<uint8_t, 32> senderBytes{};
    // Copy the 4 uint64_t values into the byte array
    memcpy(senderBytes.data(), &sender_0, sizeof(uint64_t));
    memcpy(senderBytes.data() + 8, &sender_1, sizeof(uint64_t));
    memcpy(senderBytes.data() + 16, &sender_2, sizeof(uint64_t));
    memcpy(senderBytes.data() + 24, &sender_3, sizeof(uint64_t));
    Identity sender(senderBytes);
    
    if constexpr (std::is_invocable_v<Func, ReducerContext>) {
        // init reducer - no sender
        func(ctx);
    } else if constexpr (std::is_invocable_v<Func, ReducerContext, Identity>) {
        // client_connected/disconnected - includes sender
        func(ctx, sender);
    }
}

} // namespace detail

} // namespace SpacetimeDb

// -----------------------------------------------------------------------------
// Built-in Reducer Macros
// -----------------------------------------------------------------------------

// Macro for the init reducer
#define SPACETIMEDB_INIT(function_name) \
    void function_name(SpacetimeDb::ReducerContext ctx); \
    __attribute__((export_name("__preinit__20_reducer_init"))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_init_reducer_, function_name)() { \
        SpacetimeDb::register_init_reducer(function_name); \
    } \
    void function_name(SpacetimeDb::ReducerContext ctx)

// Macro for client_connected reducer
#define SPACETIMEDB_CLIENT_CONNECTED(function_name) \
    void function_name(SpacetimeDb::ReducerContext ctx, SpacetimeDb::Identity sender); \
    __attribute__((export_name("__preinit__20_reducer_client_connected"))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_client_connected_, function_name)() { \
        SpacetimeDb::register_client_connected_reducer(function_name); \
    } \
    void function_name(SpacetimeDb::ReducerContext ctx, SpacetimeDb::Identity sender)

// Macro for client_disconnected reducer
#define SPACETIMEDB_CLIENT_DISCONNECTED(function_name) \
    void function_name(SpacetimeDb::ReducerContext ctx, SpacetimeDb::Identity sender); \
    __attribute__((export_name("__preinit__20_reducer_client_disconnected"))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_client_disconnected_, function_name)() { \
        SpacetimeDb::register_client_disconnected_reducer(function_name); \
    } \
    void function_name(SpacetimeDb::ReducerContext ctx, SpacetimeDb::Identity sender)

#endif // SPACETIMEDB_BUILTIN_REDUCERS_H