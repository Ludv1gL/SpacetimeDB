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

// Helper macros
#define SPACETIMEDB_CAT_IMPL(a, b) a##b
#define SPACETIMEDB_CAT(a, b) SPACETIMEDB_CAT_IMPL(a, b)

namespace spacetimedb {

// -----------------------------------------------------------------------------
// Identity Type for Built-in Reducers
// -----------------------------------------------------------------------------

class Identity {
public:
    static constexpr size_t SIZE = 32;
    using ByteArray = std::array<uint8_t, SIZE>;
    
    Identity() : bytes_{} {}
    explicit Identity(const ByteArray& bytes) : bytes_(bytes) {}
    
    // Construct from the raw parts passed to __call_reducer__
    Identity(uint64_t part0, uint64_t part1, uint64_t part2, uint64_t part3) {
        // Identity is little-endian encoded in 4 u64 parts
        std::memcpy(&bytes_[0], &part0, 8);
        std::memcpy(&bytes_[8], &part1, 8);
        std::memcpy(&bytes_[16], &part2, 8);
        std::memcpy(&bytes_[24], &part3, 8);
    }
    
    const ByteArray& to_byte_array() const { return bytes_; }
    
    std::string to_hex_string() const {
        static const char hex_chars[] = "0123456789abcdef";
        std::string result;
        result.reserve(SIZE * 2);
        for (uint8_t byte : bytes_) {
            result.push_back(hex_chars[byte >> 4]);
            result.push_back(hex_chars[byte & 0x0F]);
        }
        return result;
    }
    
    bool operator==(const Identity& other) const {
        return bytes_ == other.bytes_;
    }
    
    bool operator!=(const Identity& other) const {
        return bytes_ != other.bytes_;
    }
    
private:
    ByteArray bytes_;
};

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
class ReducerContext;

// Extended context that includes sender identity for built-in reducers
class BuiltinReducerContext : public ReducerContext {
public:
    Identity sender;
    
    BuiltinReducerContext(const Identity& identity) : sender(identity) {}
};

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
void builtin_reducer_wrapper(Func func, spacetimedb::ReducerContext& ctx, 
                           uint64_t sender_0, uint64_t sender_1, 
                           uint64_t sender_2, uint64_t sender_3) {
    Identity sender(sender_0, sender_1, sender_2, sender_3);
    
    if constexpr (std::is_invocable_v<Func, spacetimedb::ReducerContext>) {
        // init reducer - no sender
        func(ctx);
    } else if constexpr (std::is_invocable_v<Func, spacetimedb::ReducerContext, Identity>) {
        // client_connected/disconnected - includes sender
        func(ctx, sender);
    }
}

} // namespace detail

} // namespace spacetimedb

// -----------------------------------------------------------------------------
// Built-in Reducer Macros
// -----------------------------------------------------------------------------

// Macro for the init reducer
#define SPACETIMEDB_INIT(function_name) \
    void function_name(spacetimedb::ReducerContext ctx); \
    __attribute__((export_name("__preinit__20_reducer_init"))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_init_reducer_, function_name)() { \
        spacetimedb::register_init_reducer(function_name); \
    } \
    void function_name(spacetimedb::ReducerContext ctx)

// Macro for client_connected reducer
#define SPACETIMEDB_CLIENT_CONNECTED(function_name) \
    void function_name(spacetimedb::ReducerContext ctx, spacetimedb::Identity sender); \
    __attribute__((export_name("__preinit__20_reducer_client_connected"))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_client_connected_, function_name)() { \
        spacetimedb::register_client_connected_reducer(function_name); \
    } \
    void function_name(spacetimedb::ReducerContext ctx, spacetimedb::Identity sender)

// Macro for client_disconnected reducer
#define SPACETIMEDB_CLIENT_DISCONNECTED(function_name) \
    void function_name(spacetimedb::ReducerContext ctx, spacetimedb::Identity sender); \
    __attribute__((export_name("__preinit__20_reducer_client_disconnected"))) \
    extern "C" void SPACETIMEDB_CAT(_preinit_register_client_disconnected_, function_name)() { \
        spacetimedb::register_client_disconnected_reducer(function_name); \
    } \
    void function_name(spacetimedb::ReducerContext ctx, spacetimedb::Identity sender)

#endif // SPACETIMEDB_BUILTIN_REDUCERS_H