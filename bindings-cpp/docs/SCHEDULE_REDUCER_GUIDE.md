# SpacetimeDB C++ Module Library - Schedule Reducer Guide

This guide explains how to use scheduled reducers in the SpacetimeDB C++ Module Library. Scheduled reducers allow you to execute reducers at specific times or at regular intervals.

## Overview

Scheduled reducers in SpacetimeDB are implemented using special tables where each row represents a scheduled reducer invocation. The system automatically monitors these tables and executes the associated reducer when the scheduled time arrives.

## Core Components

### 1. ScheduleAt Type

The `ScheduleAt` type represents when a reducer should be executed:

```cpp
#include <spacetimedb/schedule_reducer.h>

// Schedule at a specific time
ScheduleAt at_time = schedule_at_time(timestamp);

// Schedule at regular intervals
ScheduleAt every_second = schedule_at_interval(TimeDuration::from_seconds(1));
ScheduleAt every_100ms = schedule_at_interval(100_ms);  // Using literals
```

### 2. TimeDuration Type

Represents a duration of time with microsecond precision:

```cpp
#include <spacetimedb/time_duration.h>

// Create durations
TimeDuration duration1 = TimeDuration::from_seconds(30);
TimeDuration duration2 = TimeDuration::from_millis(500);
TimeDuration duration3 = TimeDuration::from_micros(1000000);

// Using literals (with namespace spacetimedb::time_literals)
auto d1 = 30_s;    // 30 seconds
auto d2 = 500_ms;  // 500 milliseconds
auto d3 = 1_min;   // 1 minute
auto d4 = 2_h;     // 2 hours
```

### 3. Timestamp Type

Represents a point in time as microseconds since the Unix epoch:

```cpp
#include <spacetimedb/timestamp.h>

// Get current time
Timestamp now = Timestamp::now();

// Create from epoch values
Timestamp t1 = Timestamp::from_seconds_since_epoch(1609459200);  // Jan 1, 2021
Timestamp t2 = Timestamp::from_millis_since_epoch(1609459200000);

// Arithmetic with timestamps
Timestamp future = now + TimeDuration::from_hours(1);
TimeDuration elapsed = future - now;
```

## Creating Scheduled Tables

A scheduled table must have two mandatory fields:
- `scheduled_id`: A primary key (usually `uint64_t` with auto-increment)
- `scheduled_at`: A `ScheduleAt` field that specifies when to run

```cpp
struct MyScheduledTask {
    uint64_t scheduled_id;      // Mandatory: primary key
    ScheduleAt scheduled_at;    // Mandatory: when to execute
    // Additional fields for your data
    std::string task_data;
    int32_t priority;
};

// Register the fields
SPACETIMEDB_REGISTER_FIELDS(MyScheduledTask,
    SPACETIMEDB_FIELD(MyScheduledTask, scheduled_id, uint64_t);
    SPACETIMEDB_FIELD(MyScheduledTask, scheduled_at, ScheduleAt);
    SPACETIMEDB_FIELD(MyScheduledTask, task_data, std::string);
    SPACETIMEDB_FIELD(MyScheduledTask, priority, int32_t);
)

// Define as a scheduled table
SPACETIMEDB_SCHEDULED_TABLE(MyScheduledTask, my_scheduled_tasks, true, process_task)
```

## Implementing Scheduled Reducers

The scheduled reducer receives the entire row as its argument:

```cpp
SPACETIMEDB_REDUCER(process_task, spacetimedb::ReducerContext ctx, MyScheduledTask task) {
    // Process the task
    std::cout << "Processing task: " << task.task_data << std::endl;
    
    // For one-time tasks: The row is automatically deleted after execution
    // For interval tasks: The row remains and the reducer is called repeatedly
}
```

## Usage Examples

### Example 1: One-Time Scheduled Task

```cpp
SPACETIMEDB_REDUCER(schedule_backup, spacetimedb::ReducerContext ctx) {
    // Schedule a backup to run in 1 hour
    Timestamp backup_time = ctx.timestamp + TimeDuration::from_hours(1);
    
    ctx.db.table<MyScheduledTask>("my_scheduled_tasks").insert(MyScheduledTask{
        0,  // auto-increment
        schedule_at_time(backup_time),
        "backup_database",
        1   // high priority
    });
}
```

### Example 2: Repeating Task

```cpp
SPACETIMEDB_REDUCER(start_monitoring, spacetimedb::ReducerContext ctx) {
    // Schedule a health check every 30 seconds
    ctx.db.table<MyScheduledTask>("my_scheduled_tasks").insert(MyScheduledTask{
        0,  // auto-increment
        schedule_at_interval(30_s),
        "health_check",
        2   // medium priority
    });
}
```

### Example 3: Canceling Scheduled Tasks

```cpp
SPACETIMEDB_REDUCER(cancel_task, spacetimedb::ReducerContext ctx, uint64_t task_id) {
    // Cancel a scheduled task by deleting its row
    ctx.db.table<MyScheduledTask>("my_scheduled_tasks")
        .scheduled_id()
        .delete(task_id);
}
```

### Example 4: Game Timer System

```cpp
struct GameTimer {
    uint64_t scheduled_id;
    ScheduleAt scheduled_at;
    uint32_t player_id;
    std::string timer_type;  // "power_up_expire", "respawn", etc.
    int32_t data;
};

SPACETIMEDB_REGISTER_FIELDS(GameTimer,
    SPACETIMEDB_FIELD(GameTimer, scheduled_id, uint64_t);
    SPACETIMEDB_FIELD(GameTimer, scheduled_at, ScheduleAt);
    SPACETIMEDB_FIELD(GameTimer, player_id, uint32_t);
    SPACETIMEDB_FIELD(GameTimer, timer_type, std::string);
    SPACETIMEDB_FIELD(GameTimer, data, int32_t);
)

SPACETIMEDB_SCHEDULED_TABLE(GameTimer, game_timers, true, handle_game_timer)

SPACETIMEDB_REDUCER(handle_game_timer, spacetimedb::ReducerContext ctx, GameTimer timer) {
    if (timer.timer_type == "power_up_expire") {
        // Remove power-up from player
    } else if (timer.timer_type == "respawn") {
        // Respawn the player
    }
}

// Give a player a temporary power-up
SPACETIMEDB_REDUCER(apply_power_up, spacetimedb::ReducerContext ctx, 
                   uint32_t player_id, int32_t power_up_id) {
    // Apply the power-up effect
    // ...
    
    // Schedule its expiration in 30 seconds
    ctx.db.table<GameTimer>("game_timers").insert(GameTimer{
        0,
        schedule_at_time(ctx.timestamp + 30_s),
        player_id,
        "power_up_expire",
        power_up_id
    });
}
```

## Best Practices

1. **Unique Scheduled IDs**: Always use auto-increment for `scheduled_id` to ensure uniqueness.

2. **Cancellation**: Store the `scheduled_id` if you need to cancel tasks later:
   ```cpp
   auto result = ctx.db.table<MyScheduledTask>("my_scheduled_tasks").insert(task);
   uint64_t task_id = result.scheduled_id;  // Save for later cancellation
   ```

3. **Error Handling**: Scheduled reducers should handle errors gracefully as they run asynchronously.

4. **Time Precision**: Remember that scheduled reducers run on a best-effort basis and may be slightly delayed under heavy load.

5. **Interval vs One-Time**: 
   - One-time tasks are automatically removed after execution
   - Interval tasks continue running until manually deleted

## Limitations

1. **Maximum Delay**: Scheduled reducers can be scheduled up to approximately 2.17 years in the future.

2. **No Direct Modification**: To change a scheduled task's timing, you must delete and recreate it.

3. **Module Identity**: Scheduled reducers run with the module's identity, not the original caller's identity.

## Advanced Usage

### Restricting Access to Scheduled Reducers

To prevent clients from directly calling scheduled reducers:

```cpp
SPACETIMEDB_REDUCER(process_task, spacetimedb::ReducerContext ctx, MyScheduledTask task) {
    // Check if caller is the module itself
    if (ctx.sender != ctx.identity()) {
        // Log error or return early
        return;
    }
    
    // Process the task...
}
```

### Using with std::chrono

The Module Library integrates with C++ standard library time types:

```cpp
// Convert from std::chrono
auto duration = std::chrono::minutes(5);
auto time_duration = TimeDuration::from_chrono(duration);

auto time_point = std::chrono::system_clock::now() + std::chrono::hours(1);
auto timestamp = Timestamp::from_chrono(time_point);

// Schedule using std::chrono types
ctx.db.table<MyScheduledTask>("my_scheduled_tasks").insert(MyScheduledTask{
    0,
    schedule_at_interval(std::chrono::seconds(30)),
    "periodic_task",
    1
});
```

## Complete Example

See `scheduled_example.cpp` for a complete working example demonstrating:
- One-time scheduled tasks
- Repeating tasks at various intervals
- Task cancellation
- Multiple scheduled tables for different purposes
- Logging of scheduled reducer executions