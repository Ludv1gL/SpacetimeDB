#pragma once

#include <spacetimedb/bsatn/bsatn.h>
#include <spacetimedb/bsatn/algebraic_type.h>
#include <spacetimedb/types.h>  // For TimeDuration
#include <cstdint>
#include <chrono>
#include <ctime>

namespace SpacetimeDb {

/**
 * Represents a point in time as microseconds since the Unix epoch.
 * This corresponds to SpacetimeDB's Timestamp type.
 */
class Timestamp {
private:
    int64_t micros_since_epoch_;

public:
    // Constructors
    explicit Timestamp(int64_t micros_since_epoch = 0) : micros_since_epoch_(micros_since_epoch) {}
    
    // Factory methods
    static Timestamp from_micros_since_epoch(int64_t micros) {
        return Timestamp(micros);
    }
    
    static Timestamp from_millis_since_epoch(int64_t millis) {
        return Timestamp(millis * 1000);
    }
    
    static Timestamp from_seconds_since_epoch(int64_t seconds) {
        return Timestamp(seconds * 1000000);
    }
    
    // Get current timestamp
    static Timestamp now() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        return Timestamp(micros);
    }
    
    // Unix epoch (January 1, 1970 00:00:00 UTC)
    static Timestamp unix_epoch() {
        return Timestamp(0);
    }
    
    // Conversion from std::chrono
    static Timestamp from_chrono(std::chrono::system_clock::time_point tp) {
        auto duration = tp.time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        return Timestamp(micros);
    }
    
    // Getters
    int64_t micros_since_epoch() const { return micros_since_epoch_; }
    int64_t millis_since_epoch() const { return micros_since_epoch_ / 1000; }
    int64_t seconds_since_epoch() const { return micros_since_epoch_ / 1000000; }
    
    // Conversion to std::chrono
    std::chrono::system_clock::time_point to_chrono() const {
        return std::chrono::system_clock::time_point(
            std::chrono::microseconds(micros_since_epoch_)
        );
    }
    
    // Arithmetic operations
    Timestamp operator+(const TimeDuration& duration) const {
        return Timestamp(micros_since_epoch_ + duration.micros());
    }
    
    Timestamp operator-(const TimeDuration& duration) const {
        return Timestamp(micros_since_epoch_ - duration.micros());
    }
    
    TimeDuration operator-(const Timestamp& other) const {
        return TimeDuration::from_micros(micros_since_epoch_ - other.micros_since_epoch_);
    }
    
    // Comparison operators
    bool operator==(const Timestamp& other) const { return micros_since_epoch_ == other.micros_since_epoch_; }
    bool operator!=(const Timestamp& other) const { return micros_since_epoch_ != other.micros_since_epoch_; }
    bool operator<(const Timestamp& other) const { return micros_since_epoch_ < other.micros_since_epoch_; }
    bool operator<=(const Timestamp& other) const { return micros_since_epoch_ <= other.micros_since_epoch_; }
    bool operator>(const Timestamp& other) const { return micros_since_epoch_ > other.micros_since_epoch_; }
    bool operator>=(const Timestamp& other) const { return micros_since_epoch_ >= other.micros_since_epoch_; }
    
    // Duration since another timestamp
    TimeDuration duration_since(const Timestamp& earlier) const {
        if (*this < earlier) {
            return TimeDuration::from_micros(0);  // Return zero for negative durations
        }
        return TimeDuration::from_micros(micros_since_epoch_ - earlier.micros_since_epoch_);
    }
    
    // BSATN serialization (defined in timestamp_bsatn.h to avoid circular dependency)
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const;
    static Timestamp bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);
};

// Convenience operators
inline Timestamp operator+(const TimeDuration& duration, const Timestamp& timestamp) {
    return timestamp + duration;
}

} // namespace SpacetimeDb

// TypeRegistrar specialization will be provided by the type system when needed