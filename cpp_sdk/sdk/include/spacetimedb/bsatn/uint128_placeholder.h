#ifndef UINT128_PLACEHOLDER_H
#define UINT128_PLACEHOLDER_H

#include <cstdint>
#include <array>
#include <string> // For to_string (placeholder)

// Placeholder for uint128_t and int128_t for BSATN interface demonstration
// In a real SDK, use a proper __uint128_t or a library type.

namespace SpacetimeDB {
namespace Types {

struct uint128_t_placeholder {
    uint64_t low;
    uint64_t high;

    uint128_t_placeholder(uint64_t l = 0, uint64_t h = 0) : low(l), high(h) {}

    // Add necessary operators if they were to be used, e.g. comparison
    bool operator==(const uint128_t_placeholder& other) const {
        return low == other.low && high == other.high;
    }
    // Placeholder to_string
    std::string to_string() const { return std::to_string(high) + ":" + std::to_string(low); }
};

struct int128_t_placeholder {
    uint64_t low; // Could also be int64_t low and int64_t high for signed representation details
    int64_t high; // Using int64_t for high part to easily get sign

    int128_t_placeholder(uint64_t l = 0, int64_t h = 0) : low(l), high(h) {}

    bool operator==(const int128_t_placeholder& other) const {
        return low == other.low && high == other.high;
    }
    std::string to_string() const { return std::to_string(high) + ":" + std::to_string(low); }
};

} // namespace Types
} // namespace SpacetimeDB

// Make them usable by bsatn functions if they are in global or std namespace
// For now, assume they are passed around as SpacetimeDB::Types::uint128_t_placeholder

#endif // UINT128_PLACEHOLDER_H
