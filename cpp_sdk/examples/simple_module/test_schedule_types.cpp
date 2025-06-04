#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/schedule_reducer.h>
#include <spacetimedb/timestamp.h>
#include <spacetimedb/time_duration.h>
#include <iostream>

using namespace spacetimedb;
using namespace spacetimedb::time_literals;

// Test struct with scheduled fields
struct TestSchedule {
    uint64_t scheduled_id;
    ScheduleAt scheduled_at;
    std::string message;
};

// Register fields
SPACETIMEDB_REGISTER_FIELDS(TestSchedule,
    SPACETIMEDB_FIELD(TestSchedule, scheduled_id, uint64_t);
    SPACETIMEDB_FIELD(TestSchedule, scheduled_at, ScheduleAt);
    SPACETIMEDB_FIELD(TestSchedule, message, std::string);
)

// Define scheduled table
SPACETIMEDB_SCHEDULED_TABLE(TestSchedule, test_schedules, true, test_reducer)

// Test reducer
SPACETIMEDB_REDUCER(test_reducer, spacetimedb::ReducerContext ctx, TestSchedule schedule) {
    // Process the scheduled task
}

// Test creating different schedule types
SPACETIMEDB_REDUCER(test_schedule_creation, spacetimedb::ReducerContext ctx) {
    // Test timestamp creation
    Timestamp now = Timestamp::now();
    Timestamp future = now + TimeDuration::from_seconds(60);
    
    // Test duration creation
    TimeDuration d1 = TimeDuration::from_seconds(30);
    TimeDuration d2 = 100_ms;
    TimeDuration d3 = 5_min;
    
    // Test ScheduleAt creation
    ScheduleAt at_time = schedule_at_time(future);
    ScheduleAt every_sec = schedule_at_interval(1_s);
    ScheduleAt every_100ms = schedule_at_interval(100_ms);
    
    // Insert a scheduled task
    ctx.db.table<TestSchedule>("test_schedules").insert(TestSchedule{
        0,  // auto-inc
        at_time,
        "Test scheduled task"
    });
}