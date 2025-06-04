#ifndef REDUCER_CONTEXT_ENHANCED_H
#define REDUCER_CONTEXT_ENHANCED_H

// Forward declarations and necessary includes
#include <spacetimedb/abi/spacetimedb_abi.h>
#include <optional>
#include <array>
#include <random>
#include <memory>
#include <chrono>

namespace spacetimedb {

// Forward declarations and type aliases
class ModuleDatabase;
// Identity is defined in builtin_reducers.h which should be included before this file
using Timestamp = uint64_t;  // Microseconds since Unix epoch
using Address = std::array<uint8_t, 16>;  // Network address

// Helper function to convert bytes to hex string
inline std::string bytes_to_hex(const uint8_t* bytes, size_t len) {
    std::string result;
    result.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", bytes[i]);
        result += hex;
    }
    return result;
}

// Random number generator for reducer context
class StdbRng {
private:
    mutable std::mt19937_64 rng;
    bool initialized = false;
    
    void init_if_needed(uint64_t seed) const {
        if (!initialized) {
            rng.seed(seed);
            const_cast<StdbRng*>(this)->initialized = true;
        }
    }
    
public:
    explicit StdbRng(uint64_t seed) : rng(seed), initialized(true) {}
    
    // Generate random values
    template<typename T>
    T gen() const {
        init_if_needed(0);
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist;
            return dist(rng);
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(0.0, 1.0);
            return dist(rng);
        }
    }
    
    // Generate random value in range [min, max]
    template<typename T>
    T gen_range(T min, T max) const {
        init_if_needed(0);
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(rng);
        } else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(rng);
        }
    }
    
    // Generate random boolean
    bool gen_bool() const {
        return gen<uint32_t>() & 1;
    }
    
    // Get raw generator for advanced usage
    std::mt19937_64& get_rng() const {
        init_if_needed(0);
        return rng;
    }
};

// Enhanced ReducerContext with all runtime information
struct ReducerContext {
    // Core fields - matching Rust SDK
    Identity sender;
    Timestamp timestamp;
    std::optional<Address> connection_id;  // Using Address as connection identifier
    
    // Database access - will be set by the framework
    ModuleDatabase* db = nullptr;
    
    // Network address of the caller (if available)
    std::optional<std::string> address;
    
    // Random number generator
    mutable std::unique_ptr<StdbRng> rng;
    
    // Constructor
    ReducerContext() = default;
    
    ReducerContext(Identity s, Timestamp ts, std::optional<Address> cid = std::nullopt)
        : sender(s), timestamp(ts), connection_id(cid) {}
    
    // Get the module's own identity
    Identity identity() const {
        std::array<uint8_t, 32> buf;
        ::identity(buf.data());
        return Identity(buf);
    }
    
    // Get the timestamp as microseconds since Unix epoch
    uint64_t timestamp_micros() const {
        return timestamp.microseconds_since_epoch;
    }
    
    // Get the timestamp as milliseconds since Unix epoch
    uint64_t timestamp_millis() const {
        return timestamp.microseconds_since_epoch / 1000;
    }
    
    // Get a random number generator seeded by the timestamp
    StdbRng& get_rng() const {
        if (!rng) {
            rng = std::make_unique<StdbRng>(timestamp_micros());
        }
        return *rng;
    }
    
    // Convenience method for generating random values
    template<typename T>
    T random() const {
        return get_rng().gen<T>();
    }
    
    // Convenience method for generating random values in range
    template<typename T>
    T random_range(T min, T max) const {
        return get_rng().gen_range(min, max);
    }
    
    // Check if this is a connected client (has connection_id)
    bool is_connected_client() const {
        return connection_id.has_value();
    }
    
    // Check if this is the module itself (no connection_id)
    bool is_module() const {
        return !connection_id.has_value();
    }
    
    // Helper to log context information
    std::string to_string() const {
        std::string result = "ReducerContext {\n";
        result += "  sender: 0x" + bytes_to_hex(sender.bytes.data(), sender.bytes.size()) + "\n";
        result += "  timestamp: " + std::to_string(timestamp_micros()) + " us\n";
        result += "  connection_id: ";
        if (connection_id) {
            result += "0x" + bytes_to_hex(connection_id->bytes.data(), connection_id->bytes.size());
        } else {
            result += "None (module)";
        }
        result += "\n";
        if (address) {
            result += "  address: " + *address + "\n";
        }
        result += "}";
        return result;
    }
    
    // Create a ReducerContext from __call_reducer__ parameters
    static ReducerContext from_reducer_call(
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp_us)
    {
        // Reconstruct Identity from 4 uint64_t parts (little-endian)
        std::array<uint8_t, 32> identity_bytes;
        auto* parts = reinterpret_cast<uint64_t*>(identity_bytes.data());
        parts[0] = sender_0;
        parts[1] = sender_1;
        parts[2] = sender_2;
        parts[3] = sender_3;
        
        Identity sender{identity_bytes};
        
        // Create timestamp
        Timestamp ts(timestamp_us);
        
        // Check if connection_id is valid (non-zero)
        std::optional<Address> conn_id;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            // Reconstruct Address from 2 uint64_t parts (16 bytes total)
            std::array<uint8_t, 16> addr_bytes;
            auto* addr_parts = reinterpret_cast<uint64_t*>(addr_bytes.data());
            addr_parts[0] = conn_id_0;
            addr_parts[1] = conn_id_1;
            conn_id = Address{addr_bytes};
        }
        
        return ReducerContext(sender, ts, conn_id);
    }
};

} // namespace spacetimedb

#endif // REDUCER_CONTEXT_ENHANCED_H