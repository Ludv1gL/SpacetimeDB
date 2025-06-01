#ifndef SPACETIMEDB_SDK_TYPES_H
#define SPACETIMEDB_SDK_TYPES_H

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <chrono> // For Timestamp::current()
#include <stdexcept> // For std::runtime_error in Identity deserialization

// Include actual definitions for Reader and Writer
#include "reader.h"
#include "writer.h"

// Forward declarations from bsatn.h - needed for BsatnSerializable usage
namespace spacetimedb {
namespace bsatn {
// class Writer; // Full definition included above
// class Reader; // Full definition included above
class BsatnSerializable; // Ensure BsatnSerializable itself is forward-declared or included if it's a base
} // namespace bsatn
} // namespace spacetimedb


namespace spacetimedb {
namespace sdk {

const size_t IDENTITY_SIZE = 32;

class Identity {
public:
    Identity();
    explicit Identity(const std::array<uint8_t, IDENTITY_SIZE>& bytes);
    // static Identity from_hex_string(const std::string& hex_str); // Implementation can be added if needed

    const std::array<uint8_t, IDENTITY_SIZE>& get_bytes() const;
    std::string to_hex_string() const;

    bool operator==(const Identity& other) const;
    bool operator!=(const Identity& other) const;
    bool operator<(const Identity& other) const; // For std::map keys

    // BSATN Serialization methods (duck-typed, or inherit BsatnSerializable)
    void bsatn_serialize(::bsatn::Writer& writer) const;
    void bsatn_deserialize(::bsatn::Reader& reader);

private:
    std::array<uint8_t, IDENTITY_SIZE> value;
};

class Timestamp {
public:
    Timestamp();
    explicit Timestamp(uint64_t milliseconds_since_epoch);

    uint64_t as_milliseconds() const;

    static Timestamp current();

    bool operator==(const Timestamp& other) const;
    bool operator!=(const Timestamp& other) const;
    bool operator<(const Timestamp& other) const;
    bool operator<=(const Timestamp& other) const;
    bool operator>(const Timestamp& other) const;
    bool operator>=(const Timestamp& other) const;

    // BSATN Serialization methods
    void bsatn_serialize(::bsatn::Writer& writer) const;
    void bsatn_deserialize(::bsatn::Reader& reader);

private:
    uint64_t ms_since_epoch;
};

// Placeholder for a type that might be used with scheduled tables/reducers
struct ScheduleAt {
    uint64_t timestamp_micros; // Or whatever precision/representation is decided

    ScheduleAt() : timestamp_micros(0) {}
    explicit ScheduleAt(uint64_t ts_micros) : timestamp_micros(ts_micros) {}

    // BSATN Serialization methods
    void bsatn_serialize(::bsatn::Writer& writer) const {
        writer.write_u64_le(timestamp_micros); // Assuming write_u64_le for consistency
    }
    void bsatn_deserialize(::bsatn::Reader& reader) {
        timestamp_micros = reader.read_u64_le(); // Assuming read_u64_le for consistency
    }

    bool operator==(const ScheduleAt& other) const {
        return timestamp_micros == other.timestamp_micros;
    }
    // Add other operators as needed
};

// Placeholder for ConnectionId
struct ConnectionId {
    uint64_t id;

    ConnectionId(uint64_t val = 0) : id(val) {}

    void bsatn_serialize(::bsatn::Writer& writer) const {
        writer.write_u64_le(id);
    }
    void bsatn_deserialize(::bsatn::Reader& reader) {
        id = reader.read_u64_le();
    }
     bool operator==(const ConnectionId& other) const { return id == other.id; }
     bool operator<(const ConnectionId& other) const { return id < other.id; } // For map keys
};

// Placeholder for TimeDuration
struct TimeDuration {
    int64_t nanoseconds; // Example: represent as nanoseconds

    TimeDuration(int64_t val = 0) : nanoseconds(val) {}

    void bsatn_serialize(::bsatn::Writer& writer) const {
        writer.write_i64_le(nanoseconds);
    }
    void bsatn_deserialize(::bsatn::Reader& reader) {
        nanoseconds = reader.read_i64_le();
    }
    bool operator==(const TimeDuration& other) const { return nanoseconds == other.nanoseconds; }
    bool operator<(const TimeDuration& other) const { return nanoseconds < other.nanoseconds; } // For map keys
};


} // namespace sdk
} // namespace spacetimedb

#endif // SPACETIMEDB_SDK_TYPES_H
