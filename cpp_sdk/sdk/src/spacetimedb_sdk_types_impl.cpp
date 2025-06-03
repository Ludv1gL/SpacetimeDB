#include "spacetimedb/sdk/spacetimedb_sdk_types.h"

namespace SpacetimeDb {
namespace sdk {

// Identity implementation
Identity::Identity() : value{} {}

Identity::Identity(const std::array<uint8_t, IDENTITY_SIZE>& bytes) : value(bytes) {}

bool Identity::operator==(const Identity& other) const {
    return value == other.value;
}

// Timestamp implementation
Timestamp::Timestamp() : ms_since_epoch(0) {}

Timestamp::Timestamp(uint64_t milliseconds_since_epoch) : ms_since_epoch(milliseconds_since_epoch) {}

bool Timestamp::operator==(const Timestamp& other) const {
    return ms_since_epoch == other.ms_since_epoch;
}

} // namespace sdk
} // namespace SpacetimeDb