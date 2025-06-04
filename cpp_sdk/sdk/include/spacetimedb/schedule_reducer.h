#pragma once

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/time_duration.h>
#include <spacetimedb/timestamp.h>
#include <cstdint>
#include <chrono>

namespace spacetimedb {

/**
 * Represents when a scheduled reducer should execute.
 * Either at a specific point in time or at regular intervals.
 */
class ScheduleAt {
public:
    enum Type { Time, Interval };

private:
    Type type_;
    union {
        Timestamp time_;
        TimeDuration interval_;
    };

public:
    // Constructors
    explicit ScheduleAt(Timestamp time) : type_(Time), time_(time) {}
    explicit ScheduleAt(TimeDuration interval) : type_(Interval), interval_(interval) {}
    
    // Convenience constructors from std::chrono types
    explicit ScheduleAt(std::chrono::microseconds interval) 
        : ScheduleAt(TimeDuration::from_micros(interval.count())) {}
    
    template<typename Rep, typename Period>
    explicit ScheduleAt(std::chrono::duration<Rep, Period> duration)
        : ScheduleAt(std::chrono::duration_cast<std::chrono::microseconds>(duration)) {}

    // Getters
    Type type() const { return type_; }
    
    Timestamp time() const {
        if (type_ != Time) {
            throw std::runtime_error("ScheduleAt is not a Time");
        }
        return time_;
    }
    
    TimeDuration interval() const {
        if (type_ != Interval) {
            throw std::runtime_error("ScheduleAt is not an Interval");
        }
        return interval_;
    }

    // BSATN serialization
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        if (type_ == Time) {
            writer.write_u8(1);  // Time variant tag
            time_.bsatn_serialize(writer);
        } else {
            writer.write_u8(0);  // Interval variant tag
            interval_.bsatn_serialize(writer);
        }
    }

    static ScheduleAt bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        uint8_t tag = reader.read_u8();
        switch (tag) {
            case 0:  // Interval
                return ScheduleAt(TimeDuration::bsatn_deserialize(reader));
            case 1:  // Time
                return ScheduleAt(Timestamp::bsatn_deserialize(reader));
            default:
                throw std::runtime_error("Invalid ScheduleAt variant tag");
        }
    }
};

// Helper functions for creating ScheduleAt instances
inline ScheduleAt schedule_at_time(Timestamp time) {
    return ScheduleAt(time);
}

inline ScheduleAt schedule_at_interval(TimeDuration interval) {
    return ScheduleAt(interval);
}

template<typename Rep, typename Period>
inline ScheduleAt schedule_at_interval(std::chrono::duration<Rep, Period> duration) {
    return ScheduleAt(duration);
}

// Convenience functions for common intervals
inline ScheduleAt schedule_every_seconds(int64_t seconds) {
    return ScheduleAt(TimeDuration::from_seconds(seconds));
}

inline ScheduleAt schedule_every_millis(int64_t millis) {
    return ScheduleAt(TimeDuration::from_millis(millis));
}

inline ScheduleAt schedule_every_micros(int64_t micros) {
    return ScheduleAt(TimeDuration::from_micros(micros));
}

} // namespace spacetimedb

// Macro for defining scheduled tables
#define SPACETIMEDB_SCHEDULED_TABLE(StructType, table_name, is_public, reducer_name) \
    struct __##table_name##_scheduled_marker { \
        static constexpr const char* reducer = #reducer_name; \
    }; \
    SPACETIMEDB_TABLE(StructType, table_name, is_public)

// Register ScheduleAt type for field registration
namespace spacetimedb {
namespace detail {

template<>
struct TypeRegistrar<ScheduleAt> {
    static AlgebraicTypeRef register_type(TypeContext& ctx) {
        // ScheduleAt is a sum type with variants:
        // - Interval(TimeDuration)
        // - Time(Timestamp)
        std::vector<SumTypeVariant> variants;
        variants.emplace_back("Interval", TypeRegistrar<TimeDuration>::register_type(ctx));
        variants.emplace_back("Time", TypeRegistrar<Timestamp>::register_type(ctx));
        
        SumType sum_type{std::move(variants)};
        AlgebraicType algebraic_type = AlgebraicType::sum(std::move(sum_type));
        return ctx.add(std::move(algebraic_type));
    }
};

} // namespace detail
} // namespace spacetimedb