#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/schedule_reducer.h>
#include <spacetimedb/timestamp.h>
#include <spacetimedb/time_duration.h>
#include <iostream>
#include <string>

using namespace SpacetimeDb;
using namespace SpacetimeDb::time_literals;

// Example 1: Simple scheduled task table
struct ScheduledTask {
    uint64_t scheduled_id;
    ScheduleAt scheduled_at;
    std::string task_name;
    int32_t counter;
};

// Register fields for ScheduledTask
SPACETIMEDB_REGISTER_FIELDS(ScheduledTask,
    SPACETIMEDB_FIELD(ScheduledTask, scheduled_id, uint64_t);
    SPACETIMEDB_FIELD(ScheduledTask, scheduled_at, ScheduleAt);
    SPACETIMEDB_FIELD(ScheduledTask, task_name, std::string);
    SPACETIMEDB_FIELD(ScheduledTask, counter, int32_t);
)

// Define a scheduled table that will call the 'process_task' reducer
SPACETIMEDB_SCHEDULED_TABLE(ScheduledTask, scheduled_tasks, true, process_task)

// Example 2: Reminder system with specific times
struct Reminder {
    uint64_t scheduled_id;
    ScheduleAt scheduled_at;
    std::string message;
    std::string recipient;
};

SPACETIMEDB_REGISTER_FIELDS(Reminder,
    SPACETIMEDB_FIELD(Reminder, scheduled_id, uint64_t);
    SPACETIMEDB_FIELD(Reminder, scheduled_at, ScheduleAt);
    SPACETIMEDB_FIELD(Reminder, message, std::string);
    SPACETIMEDB_FIELD(Reminder, recipient, std::string);
)

SPACETIMEDB_SCHEDULED_TABLE(Reminder, reminders, true, send_reminder)

// Example 3: Game event system
struct GameEvent {
    uint64_t scheduled_id;
    ScheduleAt scheduled_at;
    std::string event_type;
    uint32_t player_id;
    int32_t value;
};

SPACETIMEDB_REGISTER_FIELDS(GameEvent,
    SPACETIMEDB_FIELD(GameEvent, scheduled_id, uint64_t);
    SPACETIMEDB_FIELD(GameEvent, scheduled_at, ScheduleAt);
    SPACETIMEDB_FIELD(GameEvent, event_type, std::string);
    SPACETIMEDB_FIELD(GameEvent, player_id, uint32_t);
    SPACETIMEDB_FIELD(GameEvent, value, int32_t);
)

SPACETIMEDB_SCHEDULED_TABLE(GameEvent, game_events, true, process_game_event)

// Log table for recording when scheduled reducers run
struct SchedulerLog {
    uint64_t id;
    Timestamp run_time;
    std::string reducer_name;
    std::string details;
};

SPACETIMEDB_REGISTER_FIELDS(SchedulerLog,
    SPACETIMEDB_FIELD(SchedulerLog, id, uint64_t);
    SPACETIMEDB_FIELD(SchedulerLog, run_time, Timestamp);
    SPACETIMEDB_FIELD(SchedulerLog, reducer_name, std::string);
    SPACETIMEDB_FIELD(SchedulerLog, details, std::string);
)

SPACETIMEDB_TABLE(SchedulerLog, scheduler_logs, true)

// Initialize the module with some scheduled tasks
SPACETIMEDB_REDUCER(init, SpacetimeDb::ReducerContext ctx) {
    // Schedule a one-time task 5 seconds from now
    Timestamp future_time = ctx.timestamp + TimeDuration::from_seconds(5);
    ctx.db.table<ScheduledTask>("scheduled_tasks").insert(ScheduledTask{
        0,  // auto-inc
        schedule_at_time(future_time),
        "One-time cleanup",
        0
    });
    
    // Schedule a repeating task every 10 seconds
    ctx.db.table<ScheduledTask>("scheduled_tasks").insert(ScheduledTask{
        0,  // auto-inc
        schedule_at_interval(10_s),
        "Periodic health check",
        0
    });
    
    // Schedule a repeating task every 500 milliseconds
    ctx.db.table<ScheduledTask>("scheduled_tasks").insert(ScheduledTask{
        0,  // auto-inc
        schedule_at_interval(500_ms),
        "Fast update",
        0
    });
    
    // Log the initialization
    ctx.db.table<SchedulerLog>("scheduler_logs").insert(SchedulerLog{
        0,
        ctx.timestamp,
        "init",
        "Scheduled initial tasks"
    });
}

// Scheduled reducer for processing tasks
SPACETIMEDB_REDUCER(process_task, SpacetimeDb::ReducerContext ctx, ScheduledTask task) {
    // Log that the task ran
    std::string details = "Task: " + task.task_name + ", Counter: " + std::to_string(task.counter);
    ctx.db.table<SchedulerLog>("scheduler_logs").insert(SchedulerLog{
        0,
        ctx.timestamp,
        "process_task",
        details
    });
    
    // For repeating tasks, increment the counter
    if (task.scheduled_at.type() == ScheduleAt::Interval) {
        // Note: In a real implementation, you would update the task in the table
        // to increment its counter. For now, we just log it.
    }
}

// Scheduled reducer for sending reminders
SPACETIMEDB_REDUCER(send_reminder, SpacetimeDb::ReducerContext ctx, Reminder reminder) {
    // Log the reminder
    std::string details = "To: " + reminder.recipient + ", Message: " + reminder.message;
    ctx.db.table<SchedulerLog>("scheduler_logs").insert(SchedulerLog{
        0,
        ctx.timestamp,
        "send_reminder",
        details
    });
    
    // The reminder will be automatically removed from the table after running
    // (for one-time scheduled reducers)
}

// Scheduled reducer for game events
SPACETIMEDB_REDUCER(process_game_event, SpacetimeDb::ReducerContext ctx, GameEvent event) {
    std::string details = "Event: " + event.event_type + 
                         ", Player: " + std::to_string(event.player_id) +
                         ", Value: " + std::to_string(event.value);
    ctx.db.table<SchedulerLog>("scheduler_logs").insert(SchedulerLog{
        0,
        ctx.timestamp,
        "process_game_event",
        details
    });
}

// Manual reducer to schedule a reminder
SPACETIMEDB_REDUCER(schedule_reminder, SpacetimeDb::ReducerContext ctx, 
                   std::string recipient, std::string message, int64_t delay_seconds) {
    Timestamp remind_at = ctx.timestamp + TimeDuration::from_seconds(delay_seconds);
    ctx.db.table<Reminder>("reminders").insert(Reminder{
        0,  // auto-inc
        schedule_at_time(remind_at),
        message,
        recipient
    });
}

// Manual reducer to schedule a repeating game event
SPACETIMEDB_REDUCER(schedule_game_event, SpacetimeDb::ReducerContext ctx,
                   std::string event_type, uint32_t player_id, 
                   int32_t value, int64_t interval_millis) {
    ctx.db.table<GameEvent>("game_events").insert(GameEvent{
        0,  // auto-inc
        schedule_at_interval(TimeDuration::from_millis(interval_millis)),
        event_type,
        player_id,
        value
    });
}

// Reducer to cancel a scheduled task
SPACETIMEDB_REDUCER(cancel_scheduled_task, SpacetimeDb::ReducerContext ctx, uint64_t task_id) {
    auto& tasks = ctx.db.table<ScheduledTask>("scheduled_tasks");
    // In a real implementation, you would use:
    // tasks.scheduled_id().delete(task_id);
    // For now, we just log the cancellation
    ctx.db.table<SchedulerLog>("scheduler_logs").insert(SchedulerLog{
        0,
        ctx.timestamp,
        "cancel_scheduled_task",
        "Cancelled task ID: " + std::to_string(task_id)
    });
}

// Reducer to view all scheduled tasks
SPACETIMEDB_REDUCER(list_scheduled_tasks, SpacetimeDb::ReducerContext ctx) {
    auto& tasks = ctx.db.table<ScheduledTask>("scheduled_tasks");
    int count = 0;
    // In a real implementation, you would iterate:
    // for (const auto& task : tasks.iter()) { count++; }
    
    ctx.db.table<SchedulerLog>("scheduler_logs").insert(SchedulerLog{
        0,
        ctx.timestamp,
        "list_scheduled_tasks",
        "Total scheduled tasks: " + std::to_string(count)
    });
}