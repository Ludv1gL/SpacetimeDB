/**
 * Comprehensive Integration Test for SpacetimeDB C++ SDK
 * 
 * This test exercises all major features of the SDK:
 * - Basic table operations (create, read, update, delete)
 * - Built-in reducers (init, connected, disconnected)
 * - Enhanced context features (timestamps, identity, etc.)
 * - Advanced queries (filters, joins, aggregations)
 * - Scheduled reducers
 * - Constraints (unique, foreign key, check)
 * - Transactions
 * - Versioning and migration
 * - Credentials and permissions
 * - Error handling
 */

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/schedule_reducer.h>
#include <spacetimedb/constraint_validation.h>
#include <spacetimedb/transaction.h>
#include <spacetimedb/versioning.h>
#include <spacetimedb/credentials.h>
#include <cassert>
#include <sstream>

// =============================================================================
// TEST DATA STRUCTURES
// =============================================================================

// Basic types for testing
struct User {
    uint64_t id;
    std::string username;
    std::string email;
    uint32_t age;
    spacetimedb::Timestamp created_at;
    bool is_active;
};

struct Post {
    uint64_t id;
    uint64_t user_id;  // Foreign key to User
    std::string title;
    std::string content;
    spacetimedb::Timestamp posted_at;
    uint32_t view_count;
};

struct Tag {
    uint64_t id;
    std::string name;
};

struct PostTag {
    uint64_t post_id;
    uint64_t tag_id;
};

// Test versioned structure
struct UserProfileV1 {
    uint64_t user_id;
    std::string bio;
};

struct UserProfileV2 {
    uint64_t user_id;
    std::string bio;
    std::string avatar_url;
    std::vector<std::string> social_links;
};

// Scheduled task
struct ScheduledTask {
    uint64_t id;
    std::string task_name;
    spacetimedb::Timestamp scheduled_at;
    bool completed;
};

// Permission test
struct SecureData {
    uint64_t id;
    spacetimedb::Identity owner_id;
    std::string data;
    bool is_public;
};

// =============================================================================
// FIELD REGISTRATION
// =============================================================================

SPACETIMEDB_REGISTER_FIELDS(User,
    SPACETIMEDB_FIELD(User, id, uint64_t);
    SPACETIMEDB_FIELD(User, username, std::string);
    SPACETIMEDB_FIELD(User, email, std::string);
    SPACETIMEDB_FIELD(User, age, uint32_t);
    SPACETIMEDB_FIELD(User, created_at, spacetimedb::Timestamp);
    SPACETIMEDB_FIELD(User, is_active, bool);
)

SPACETIMEDB_REGISTER_FIELDS(Post,
    SPACETIMEDB_FIELD(Post, id, uint64_t);
    SPACETIMEDB_FIELD(Post, user_id, uint64_t);
    SPACETIMEDB_FIELD(Post, title, std::string);
    SPACETIMEDB_FIELD(Post, content, std::string);
    SPACETIMEDB_FIELD(Post, posted_at, spacetimedb::Timestamp);
    SPACETIMEDB_FIELD(Post, view_count, uint32_t);
)

SPACETIMEDB_REGISTER_FIELDS(Tag,
    SPACETIMEDB_FIELD(Tag, id, uint64_t);
    SPACETIMEDB_FIELD(Tag, name, std::string);
)

SPACETIMEDB_REGISTER_FIELDS(PostTag,
    SPACETIMEDB_FIELD(PostTag, post_id, uint64_t);
    SPACETIMEDB_FIELD(PostTag, tag_id, uint64_t);
)

SPACETIMEDB_REGISTER_FIELDS(UserProfileV1,
    SPACETIMEDB_FIELD(UserProfileV1, user_id, uint64_t);
    SPACETIMEDB_FIELD(UserProfileV1, bio, std::string);
)

SPACETIMEDB_REGISTER_FIELDS(UserProfileV2,
    SPACETIMEDB_FIELD(UserProfileV2, user_id, uint64_t);
    SPACETIMEDB_FIELD(UserProfileV2, bio, std::string);
    SPACETIMEDB_FIELD(UserProfileV2, avatar_url, std::string);
    SPACETIMEDB_FIELD(UserProfileV2, social_links, std::vector<std::string>);
)

SPACETIMEDB_REGISTER_FIELDS(ScheduledTask,
    SPACETIMEDB_FIELD(ScheduledTask, id, uint64_t);
    SPACETIMEDB_FIELD(ScheduledTask, task_name, std::string);
    SPACETIMEDB_FIELD(ScheduledTask, scheduled_at, spacetimedb::Timestamp);
    SPACETIMEDB_FIELD(ScheduledTask, completed, bool);
)

SPACETIMEDB_REGISTER_FIELDS(SecureData,
    SPACETIMEDB_FIELD(SecureData, id, uint64_t);
    SPACETIMEDB_FIELD(SecureData, owner_id, spacetimedb::Identity);
    SPACETIMEDB_FIELD(SecureData, data, std::string);
    SPACETIMEDB_FIELD(SecureData, is_public, bool);
)

// =============================================================================
// TABLE DECLARATIONS WITH CONSTRAINTS
// =============================================================================

// Basic tables
SPACETIMEDB_TABLE(User, users, true)
SPACETIMEDB_TABLE(Post, posts, true)
SPACETIMEDB_TABLE(Tag, tags, true)
SPACETIMEDB_TABLE(PostTag, post_tags, true)

// Versioned tables
SPACETIMEDB_VERSIONED_TABLE(UserProfileV1, user_profiles, true, 1)
SPACETIMEDB_VERSIONED_TABLE(UserProfileV2, user_profiles, true, 2)

// Other tables
SPACETIMEDB_TABLE(ScheduledTask, scheduled_tasks, true)
SPACETIMEDB_TABLE(SecureData, secure_data, true)

// Constraint definitions
SPACETIMEDB_UNIQUE_CONSTRAINT(User, username)
SPACETIMEDB_UNIQUE_CONSTRAINT(User, email)
SPACETIMEDB_CHECK_CONSTRAINT(User, age_check, age >= 18)

SPACETIMEDB_FOREIGN_KEY_CONSTRAINT(Post, user_id, User, id)
SPACETIMEDB_FOREIGN_KEY_CONSTRAINT(PostTag, post_id, Post, id)
SPACETIMEDB_FOREIGN_KEY_CONSTRAINT(PostTag, tag_id, Tag, id)

SPACETIMEDB_UNIQUE_CONSTRAINT(Tag, name)

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static uint64_t next_id = 1;

uint64_t generate_id() {
    return next_id++;
}

void log_test(const std::string& test_name, bool passed) {
    if (passed) {
        spacetimedb::log::info("✓ TEST PASSED: " + test_name);
    } else {
        spacetimedb::log::error("✗ TEST FAILED: " + test_name);
    }
}

// =============================================================================
// BUILT-IN REDUCERS
// =============================================================================

SPACETIMEDB_INIT() {
    spacetimedb::log::info("=== Module Initialization ===");
    
    // Create default admin user
    User admin{
        .id = generate_id(),
        .username = "admin",
        .email = "admin@example.com",
        .age = 30,
        .created_at = spacetimedb::Timestamp::now(),
        .is_active = true
    };
    
    try {
        spacetimedb::ModuleDatabase::get_instance().table<User>("users").insert(admin);
        spacetimedb::log::info("Admin user created successfully");
    } catch (const std::exception& e) {
        spacetimedb::log::error("Failed to create admin user: " + std::string(e.what()));
    }
}

SPACETIMEDB_CLIENT_CONNECTED(spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("Client connected: " + ctx.sender.to_hex());
    
    // Track connection in secure data
    SecureData connection_record{
        .id = generate_id(),
        .owner_id = ctx.sender,
        .data = "Connected at " + ctx.timestamp.to_string(),
        .is_public = false
    };
    
    ctx.db.table<SecureData>("secure_data").insert(connection_record);
}

SPACETIMEDB_CLIENT_DISCONNECTED(spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("Client disconnected: " + ctx.sender.to_hex());
    
    // Clean up any session data
    auto secure_table = ctx.db.table<SecureData>("secure_data");
    secure_table.delete_where([&ctx](const SecureData& data) {
        return data.owner_id == ctx.sender;
    });
}

// =============================================================================
// TEST REDUCERS
// =============================================================================

// Test 1: Basic CRUD operations
SPACETIMEDB_REDUCER(test_basic_crud, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 1: Basic CRUD Operations ===");
    
    auto users_table = ctx.db.table<User>("users");
    bool test_passed = true;
    
    try {
        // CREATE
        User new_user{
            .id = generate_id(),
            .username = "testuser",
            .email = "test@example.com",
            .age = 25,
            .created_at = ctx.timestamp,
            .is_active = true
        };
        
        users_table.insert(new_user);
        spacetimedb::log::info("CREATE: User inserted successfully");
        
        // READ
        auto user_opt = users_table.find_by_unique("username", "testuser");
        if (user_opt && user_opt->email == "test@example.com") {
            spacetimedb::log::info("READ: User found successfully");
        } else {
            spacetimedb::log::error("READ: Failed to find user");
            test_passed = false;
        }
        
        // UPDATE
        if (user_opt) {
            User updated_user = *user_opt;
            updated_user.age = 26;
            users_table.update(updated_user);
            spacetimedb::log::info("UPDATE: User updated successfully");
        }
        
        // DELETE
        users_table.delete_where([](const User& u) {
            return u.username == "testuser";
        });
        spacetimedb::log::info("DELETE: User deleted successfully");
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("CRUD test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Basic CRUD Operations", test_passed);
}

// Test 2: Constraint validation
SPACETIMEDB_REDUCER(test_constraints, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 2: Constraint Validation ===");
    
    auto users_table = ctx.db.table<User>("users");
    bool test_passed = true;
    
    try {
        // Test unique constraint violation
        User duplicate_user{
            .id = generate_id(),
            .username = "admin",  // Should fail - already exists
            .email = "admin2@example.com",
            .age = 25,
            .created_at = ctx.timestamp,
            .is_active = true
        };
        
        try {
            users_table.insert(duplicate_user);
            spacetimedb::log::error("Unique constraint test failed - duplicate allowed");
            test_passed = false;
        } catch (const spacetimedb::ConstraintViolationException& e) {
            spacetimedb::log::info("Unique constraint correctly enforced");
        }
        
        // Test check constraint violation
        User young_user{
            .id = generate_id(),
            .username = "younguser",
            .email = "young@example.com",
            .age = 16,  // Should fail - age < 18
            .created_at = ctx.timestamp,
            .is_active = true
        };
        
        try {
            users_table.insert(young_user);
            spacetimedb::log::error("Check constraint test failed - invalid age allowed");
            test_passed = false;
        } catch (const spacetimedb::ConstraintViolationException& e) {
            spacetimedb::log::info("Check constraint correctly enforced");
        }
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Constraint test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Constraint Validation", test_passed);
}

// Test 3: Foreign key relationships
SPACETIMEDB_REDUCER(test_foreign_keys, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 3: Foreign Key Relationships ===");
    
    bool test_passed = true;
    
    try {
        // Create a user
        User author{
            .id = generate_id(),
            .username = "author1",
            .email = "author@example.com",
            .age = 28,
            .created_at = ctx.timestamp,
            .is_active = true
        };
        uint64_t author_id = author.id;
        ctx.db.table<User>("users").insert(author);
        
        // Create a post
        Post post{
            .id = generate_id(),
            .user_id = author_id,
            .title = "Test Post",
            .content = "This is a test post",
            .posted_at = ctx.timestamp,
            .view_count = 0
        };
        uint64_t post_id = post.id;
        ctx.db.table<Post>("posts").insert(post);
        
        // Create tags
        Tag tag1{.id = generate_id(), .name = "test"};
        Tag tag2{.id = generate_id(), .name = "integration"};
        ctx.db.table<Tag>("tags").insert(tag1);
        ctx.db.table<Tag>("tags").insert(tag2);
        
        // Create post-tag relationships
        PostTag pt1{.post_id = post_id, .tag_id = tag1.id};
        PostTag pt2{.post_id = post_id, .tag_id = tag2.id};
        ctx.db.table<PostTag>("post_tags").insert(pt1);
        ctx.db.table<PostTag>("post_tags").insert(pt2);
        
        spacetimedb::log::info("Foreign key relationships created successfully");
        
        // Test cascade delete (if supported)
        // Note: This would depend on FK cascade settings
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Foreign key test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Foreign Key Relationships", test_passed);
}

// Test 4: Advanced queries (requires advanced header)
SPACETIMEDB_REDUCER(test_advanced_queries, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 4: Advanced Queries ===");
    
    bool test_passed = true;
    
    try {
        // Create test data
        for (int i = 0; i < 5; i++) {
            User user{
                .id = generate_id(),
                .username = "user" + std::to_string(i),
                .email = "user" + std::to_string(i) + "@example.com",
                .age = static_cast<uint32_t>(20 + i * 5),
                .created_at = ctx.timestamp,
                .is_active = (i % 2 == 0)
            };
            ctx.db.table<User>("users").insert(user);
        }
        
        // Test filter query
        auto enhanced_users = spacetimedb::EnhancedTableHandle<User>("users");
        auto active_users = enhanced_users.filter([](const User& u) {
            return u.is_active;
        });
        
        spacetimedb::log::info("Found " + std::to_string(active_users.size()) + " active users");
        
        // Test update_where
        size_t updated = enhanced_users.update_where(
            [](const User& u) { return u.age >= 30; },
            [](User& u) { u.is_active = false; }
        );
        
        spacetimedb::log::info("Updated " + std::to_string(updated) + " users");
        
        // Test aggregation
        auto stats = enhanced_users.aggregate<double>(
            [](const User& u) { return static_cast<double>(u.age); }
        );
        
        spacetimedb::log::info("Average age: " + std::to_string(stats.average));
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Advanced query test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Advanced Queries", test_passed);
}

// Test 5: Transactions
SPACETIMEDB_REDUCER(test_transactions, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 5: Transactions ===");
    
    bool test_passed = true;
    
    try {
        // Start a transaction
        auto tx = spacetimedb::Transaction::begin();
        
        try {
            // Create multiple related records
            User tx_user{
                .id = generate_id(),
                .username = "tx_user",
                .email = "tx@example.com",
                .age = 30,
                .created_at = ctx.timestamp,
                .is_active = true
            };
            uint64_t user_id = tx_user.id;
            
            tx.table<User>("users").insert(tx_user);
            
            // Create posts in transaction
            for (int i = 0; i < 3; i++) {
                Post tx_post{
                    .id = generate_id(),
                    .user_id = user_id,
                    .title = "Transaction Post " + std::to_string(i),
                    .content = "Content " + std::to_string(i),
                    .posted_at = ctx.timestamp,
                    .view_count = 0
                };
                tx.table<Post>("posts").insert(tx_post);
            }
            
            // Commit transaction
            tx.commit();
            spacetimedb::log::info("Transaction committed successfully");
            
        } catch (const std::exception& e) {
            tx.rollback();
            spacetimedb::log::info("Transaction rolled back: " + std::string(e.what()));
        }
        
        // Test rollback scenario
        auto tx2 = spacetimedb::Transaction::begin();
        try {
            User bad_user{
                .id = generate_id(),
                .username = "rollback_test",
                .email = "rollback@example.com",
                .age = 15,  // Should fail check constraint
                .created_at = ctx.timestamp,
                .is_active = true
            };
            tx2.table<User>("users").insert(bad_user);
            tx2.commit();
            
            spacetimedb::log::error("Transaction should have failed");
            test_passed = false;
            
        } catch (const std::exception& e) {
            tx2.rollback();
            spacetimedb::log::info("Transaction correctly rolled back on constraint violation");
        }
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Transaction test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Transactions", test_passed);
}

// Test 6: Scheduled reducers
SPACETIMEDB_SCHEDULED_REDUCER(cleanup_old_tasks, spacetimedb::Schedule::every_minutes(5)) {
    spacetimedb::ReducerContext ctx;
    spacetimedb::log::info("=== Scheduled Cleanup Task Running ===");
    
    auto tasks = ctx.db.table<ScheduledTask>("scheduled_tasks");
    auto now = spacetimedb::Timestamp::now();
    
    // Mark overdue tasks as completed
    size_t completed = 0;
    tasks.iter([&](ScheduledTask& task) {
        if (!task.completed && task.scheduled_at < now) {
            task.completed = true;
            tasks.update(task);
            completed++;
        }
    });
    
    spacetimedb::log::info("Completed " + std::to_string(completed) + " overdue tasks");
}

SPACETIMEDB_REDUCER(test_scheduled_tasks, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 6: Scheduled Tasks ===");
    
    bool test_passed = true;
    
    try {
        // Create scheduled tasks
        auto tasks = ctx.db.table<ScheduledTask>("scheduled_tasks");
        
        // Task due in past (should be cleaned up)
        ScheduledTask past_task{
            .id = generate_id(),
            .task_name = "Past Task",
            .scheduled_at = spacetimedb::Timestamp::from_seconds_since_epoch(
                ctx.timestamp.seconds_since_epoch() - 3600
            ),
            .completed = false
        };
        tasks.insert(past_task);
        
        // Task due in future
        ScheduledTask future_task{
            .id = generate_id(),
            .task_name = "Future Task",
            .scheduled_at = spacetimedb::Timestamp::from_seconds_since_epoch(
                ctx.timestamp.seconds_since_epoch() + 3600
            ),
            .completed = false
        };
        tasks.insert(future_task);
        
        spacetimedb::log::info("Created scheduled tasks for cleanup test");
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Scheduled task test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Scheduled Tasks", test_passed);
}

// Test 7: Versioning and migration
SPACETIMEDB_REDUCER(test_versioning, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 7: Versioning and Migration ===");
    
    bool test_passed = true;
    
    try {
        // Create v1 profile
        UserProfileV1 profile_v1{
            .user_id = 1,
            .bio = "Original bio"
        };
        
        // In a real scenario, this would be handled by migration system
        // For testing, we'll simulate the migration
        UserProfileV2 profile_v2{
            .user_id = profile_v1.user_id,
            .bio = profile_v1.bio,
            .avatar_url = "",  // Default for new field
            .social_links = {}  // Default empty vector
        };
        
        ctx.db.table<UserProfileV2>("user_profiles").insert(profile_v2);
        spacetimedb::log::info("Profile migrated from v1 to v2");
        
        // Update with v2 features
        profile_v2.avatar_url = "https://example.com/avatar.jpg";
        profile_v2.social_links.push_back("https://twitter.com/user");
        profile_v2.social_links.push_back("https://github.com/user");
        
        ctx.db.table<UserProfileV2>("user_profiles").update(profile_v2);
        spacetimedb::log::info("Profile updated with v2 features");
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Versioning test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Versioning and Migration", test_passed);
}

// Test 8: Credentials and permissions
SPACETIMEDB_REDUCER(test_credentials, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 8: Credentials and Permissions ===");
    
    bool test_passed = true;
    
    try {
        // Get current user credentials
        auto creds = spacetimedb::Credentials::get_current();
        spacetimedb::log::info("Current identity: " + ctx.sender.to_hex());
        
        // Test permission check
        SecureData private_data{
            .id = generate_id(),
            .owner_id = ctx.sender,
            .data = "Private information",
            .is_public = false
        };
        
        ctx.db.table<SecureData>("secure_data").insert(private_data);
        
        // In a real scenario, we'd have permission checks
        // For testing, we'll simulate access control
        auto secure_table = ctx.db.table<SecureData>("secure_data");
        auto my_data = secure_table.filter([&ctx](const SecureData& data) {
            return data.owner_id == ctx.sender || data.is_public;
        });
        
        spacetimedb::log::info("Found " + std::to_string(my_data.size()) + " accessible records");
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Credentials test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Credentials and Permissions", test_passed);
}

// Test 9: Error handling
SPACETIMEDB_REDUCER(test_error_handling, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 9: Error Handling ===");
    
    bool test_passed = true;
    
    try {
        // Test various error scenarios
        auto users = ctx.db.table<User>("users");
        
        // 1. Record not found
        try {
            auto result = users.find_by_unique("username", "nonexistent");
            if (!result) {
                spacetimedb::log::info("Correctly handled missing record");
            }
        } catch (const spacetimedb::RecordNotFoundException& e) {
            spacetimedb::log::info("Record not found exception caught");
        }
        
        // 2. Invalid operation
        try {
            // Attempt to insert invalid data
            User invalid_user{};  // All fields uninitialized
            users.insert(invalid_user);
            
            spacetimedb::log::error("Invalid insert should have failed");
            test_passed = false;
            
        } catch (const spacetimedb::InvalidOperationException& e) {
            spacetimedb::log::info("Invalid operation correctly caught");
        }
        
        // 3. Type mismatch (would be caught at compile time in C++)
        // Testing runtime validation instead
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Error handling test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Error Handling", test_passed);
}

// Test 10: Performance and stress test
SPACETIMEDB_REDUCER(test_performance, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("=== Test 10: Performance Test ===");
    
    bool test_passed = true;
    
    try {
        auto start_time = ctx.timestamp;
        const int RECORD_COUNT = 1000;
        
        // Bulk insert test
        auto users = ctx.db.table<User>("users");
        for (int i = 0; i < RECORD_COUNT; i++) {
            User perf_user{
                .id = generate_id(),
                .username = "perf_user_" + std::to_string(i),
                .email = "perf" + std::to_string(i) + "@example.com",
                .age = static_cast<uint32_t>(18 + (i % 50)),
                .created_at = ctx.timestamp,
                .is_active = (i % 3 != 0)
            };
            users.insert(perf_user);
        }
        
        auto insert_time = spacetimedb::Timestamp::now();
        spacetimedb::log::info("Inserted " + std::to_string(RECORD_COUNT) + " records");
        
        // Query performance test
        auto enhanced_users = spacetimedb::EnhancedTableHandle<User>("users");
        auto active_count = enhanced_users.count([](const User& u) {
            return u.is_active;
        });
        
        spacetimedb::log::info("Found " + std::to_string(active_count) + " active users");
        
        // Cleanup
        users.delete_where([](const User& u) {
            return u.username.find("perf_user_") == 0;
        });
        
        spacetimedb::log::info("Performance test completed");
        
    } catch (const std::exception& e) {
        spacetimedb::log::error("Performance test failed: " + std::string(e.what()));
        test_passed = false;
    }
    
    log_test("Performance Test", test_passed);
}

// Master test runner
SPACETIMEDB_REDUCER(run_all_tests, spacetimedb::ReducerContext ctx) {
    spacetimedb::log::info("==================================================");
    spacetimedb::log::info("Running Comprehensive Integration Test Suite");
    spacetimedb::log::info("==================================================");
    
    // Run all tests in sequence
    test_basic_crud(ctx);
    test_constraints(ctx);
    test_foreign_keys(ctx);
    test_advanced_queries(ctx);
    test_transactions(ctx);
    test_scheduled_tasks(ctx);
    test_versioning(ctx);
    test_credentials(ctx);
    test_error_handling(ctx);
    test_performance(ctx);
    
    spacetimedb::log::info("==================================================");
    spacetimedb::log::info("Integration Test Suite Completed");
    spacetimedb::log::info("==================================================");
}