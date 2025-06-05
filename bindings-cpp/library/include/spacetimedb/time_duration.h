#pragma once

#include <cstdint>
#include <chrono>

// Forward declarations for BSATN
namespace SpacetimeDb {
namespace bsatn {
    class Writer;
    class Reader;
}
}

namespace SpacetimeDb {

/**
 * Represents a duration of time with microsecond precision.
 * This corresponds to SpacetimeDB's TimeDuration type.
 */
class TimeDuration {
private:
    int64_t micros_;

public:
    // Constructors
    explicit TimeDuration(int64_t micros = 0) : micros_(micros) {}
    
    // Factory methods
    static TimeDuration from_micros(int64_t micros) { return TimeDuration(micros); }
    static TimeDuration from_millis(int64_t millis) { return TimeDuration(millis * 1000); }
    static TimeDuration from_seconds(int64_t seconds) { return TimeDuration(seconds * 1000000); }
    static TimeDuration from_minutes(int64_t minutes) { return TimeDuration(minutes * 60000000); }
    static TimeDuration from_hours(int64_t hours) { return TimeDuration(hours * 3600000000); }
    
    // Conversion from std::chrono
    template<typename Rep, typename Period>
    static TimeDuration from_chrono(std::chrono::duration<Rep, Period> duration) {
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        return TimeDuration(static_cast<int64_t>(micros));
    }
    
    // Getters
    int64_t micros() const { return micros_; }
    int64_t millis() const { return micros_ / 1000; }
    int64_t seconds() const { return micros_ / 1000000; }
    
    // Conversion to std::chrono
    std::chrono::microseconds to_chrono() const {
        return std::chrono::microseconds(micros_);
    }
    
    // Arithmetic operations
    TimeDuration operator+(const TimeDuration& other) const {
        return TimeDuration(micros_ + other.micros_);
    }
    
    TimeDuration operator-(const TimeDuration& other) const {
        return TimeDuration(micros_ - other.micros_);
    }
    
    TimeDuration operator*(int64_t scalar) const {
        return TimeDuration(micros_ * scalar);
    }
    
    TimeDuration operator/(int64_t scalar) const {
        return TimeDuration(micros_ / scalar);
    }
    
    // Comparison operators
    bool operator==(const TimeDuration& other) const { return micros_ == other.micros_; }
    bool operator!=(const TimeDuration& other) const { return micros_ != other.micros_; }
    bool operator<(const TimeDuration& other) const { return micros_ < other.micros_; }
    bool operator<=(const TimeDuration& other) const { return micros_ <= other.micros_; }
    bool operator>(const TimeDuration& other) const { return micros_ > other.micros_; }
    bool operator>=(const TimeDuration& other) const { return micros_ >= other.micros_; }
    
    // Absolute value
    TimeDuration abs() const {
        return TimeDuration(micros_ >= 0 ? micros_ : -micros_);
    }
    
    // BSATN serialization (defined out of line to avoid circular dependency)
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const;
    static TimeDuration bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader);
};

// Convenience functions
inline TimeDuration operator*(int64_t scalar, const TimeDuration& duration) {
    return duration * scalar;
}

// User-defined literals for convenient duration creation
namespace time_literals {
    inline TimeDuration operator""_us(unsigned long long micros) {
        return TimeDuration::from_micros(static_cast<int64_t>(micros));
    }
    
    inline TimeDuration operator""_ms(unsigned long long millis) {
        return TimeDuration::from_millis(static_cast<int64_t>(millis));
    }
    
    inline TimeDuration operator""_s(unsigned long long seconds) {
        return TimeDuration::from_seconds(static_cast<int64_t>(seconds));
    }
    
    inline TimeDuration operator""_min(unsigned long long minutes) {
        return TimeDuration::from_minutes(static_cast<int64_t>(minutes));
    }
    
    inline TimeDuration operator""_h(unsigned long long hours) {
        return TimeDuration::from_hours(static_cast<int64_t>(hours));
    }
}

} // namespace SpacetimeDb

// Register TimeDuration for type registration
namespace SpacetimeDb {
// TypeRegistrar specialization removed - will be provided by the type system when needed
} // namespace SpacetimeDb