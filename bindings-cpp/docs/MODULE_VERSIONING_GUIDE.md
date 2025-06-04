# SpacetimeDB C++ Module Library Versioning Guide

## Overview

The SpacetimeDB C++ Module Library provides comprehensive module versioning support, enabling:
- Semantic versioning for modules
- Schema migration between versions
- Version compatibility checking
- Backward compatibility helpers
- Automated and manual migration support

## Core Concepts

### 1. Module Versioning

Modules use semantic versioning (SemVer) with the format `MAJOR.MINOR.PATCH`:
- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

```cpp
#include <spacetimedb/versioning.h>

// Define module version
SPACETIMEDB_MODULE_VERSION(1, 2, 3)

// Define module metadata
SPACETIMEDB_MODULE_METADATA(
    "MyModule",
    "Author Name",
    "Module description",
    "MIT"
)
```

### 2. Version Compatibility

The SDK automatically checks version compatibility:

```cpp
ModuleVersion v1{1, 0, 0};
ModuleVersion v2{1, 1, 0};
ModuleVersion v3{2, 0, 0};

// Same major version = compatible
assert(v2.is_compatible_with(v1));  // true

// Different major version = incompatible
assert(!v3.is_compatible_with(v1)); // false

// Check for breaking changes
assert(v3.is_breaking_change_from(v1)); // true
```

### 3. Schema Changes

Track schema modifications between versions:

```cpp
SchemaChange change{
    .type = SchemaChangeType::COLUMN_ADDED,
    .table_name = "users",
    .object_name = "avatar_url"
};

// Check if backward compatible
if (change.is_backward_compatible()) {
    // Safe to apply without migration
}
```

## Migration System

### 1. Defining Migrations

Create migrations between module versions:

```cpp
#include <spacetimedb/migration.h>

// Define a migration from v1.0.0 to v2.0.0
DEFINE_MIGRATION(MigrateV1ToV2, 1, 0, 0, 2, 0, 0)

void MigrateV1ToV2::up(MigrationContext& ctx) {
    // Add new table
    ctx.log_info("Creating user_profiles table");
    
    // Migrate data
    auto users = ctx.db().table<User>("users");
    // Transform existing data...
}

void MigrateV1ToV2::down(MigrationContext& ctx) {
    // Rollback changes
    ctx.log_warning("Rolling back to v1.0.0");
}

std::string MigrateV1ToV2::description() const {
    return "Add user profiles and enhanced fields";
}

// Register the migration
REGISTER_MIGRATION(MigrateV1ToV2)
```

### 2. Migration Builder API

Use the fluent API for common migrations:

```cpp
MigrationBuilder builder(v1, v2);

builder
    .add_table("user_profiles")
    .add_column("users", "display_name", "string", "username")
    .add_index("users", "email_idx", {"email"})
    .transform_data("Normalize emails", 
        [](MigrationContext& ctx) {
            // Custom data transformation
        },
        [](MigrationContext& ctx) {
            // Rollback logic
        });

auto migrations = builder.build();
```

### 3. Automatic vs Manual Migrations

#### Automatic Migrations (Non-breaking changes)
- Adding new tables
- Adding optional columns
- Adding indexes
- Adding new reducers

```cpp
// These changes can be applied automatically
plan.add_change({SchemaChangeType::TABLE_ADDED, "new_table"});
plan.add_change({SchemaChangeType::INDEX_ADDED, "users", "email_idx"});
```

#### Manual Migrations (Breaking changes)
- Removing tables/columns
- Changing column types
- Modifying reducer signatures
- Complex data transformations

```cpp
class ComplexMigration : public Migration {
    void up(MigrationContext& ctx) override {
        // Manual migration logic
        ctx.log_info("Performing complex migration");
        
        // Check if table exists
        if (ctx.table_exists("legacy_data")) {
            // Transform and migrate data
        }
    }
};
```

## Version Management in Modules

### 1. Module Initialization

```cpp
SPACETIMEDB_REDUCER(__init__, ReducerContext ctx) {
    auto& manager = ModuleVersionManager::instance();
    
    // Register current version
    manager.register_version(MODULE_VERSION, MODULE_METADATA);
    
    // Check for pending migrations
    auto current = manager.current_version();
    log("Module initialized: " + current.to_string());
}
```

### 2. Version Negotiation

Allow clients to negotiate compatible versions:

```cpp
SPACETIMEDB_REDUCER(negotiate_version, ReducerContext ctx, 
                   std::string client_version_str) {
    auto client_version = ModuleVersion::parse(client_version_str);
    auto module_version = MODULE_VERSION;
    
    if (module_version.is_compatible_with(client_version)) {
        // Return available features for client version
        auto features = get_features_for_version(client_version);
        // Send feature list to client
    } else {
        // Inform client of incompatibility
        log_error("Incompatible client version: " + client_version_str);
    }
}
```

### 3. Migration Execution

Execute migrations when upgrading:

```cpp
SPACETIMEDB_REDUCER(__migrate__, ReducerContext ctx,
                   std::string from_ver, std::string to_ver) {
    auto from = ModuleVersion::parse(from_ver);
    auto to = ModuleVersion::parse(to_ver);
    
    // Find migration path
    auto& registry = MigrationRegistry::instance();
    auto path = registry.find_migration_path(from, to);
    
    if (!path.has_value()) {
        log_error("No migration path found");
        return;
    }
    
    // Execute migrations in order
    MigrationContext ctx(&ctx, from, to);
    for (auto* migration : path.value()) {
        log("Executing: " + migration->description());
        migration->up(ctx);
    }
}
```

## Best Practices

### 1. Version Planning

- Use **MAJOR** version for breaking changes
- Use **MINOR** version for new features
- Use **PATCH** version for bug fixes
- Plan migrations early in development

### 2. Backward Compatibility

Maintain compatibility within major versions:

```cpp
// Good: Adding optional field
struct UserV2 {
    uint64_t id;
    std::string name;
    std::optional<std::string> bio;  // New optional field
};

// Bad: Changing field type (requires major version)
struct UserV2 {
    uint64_t id;
    std::string name;
    uint32_t age;  // Was uint16_t - breaking change!
};
```

### 3. Feature Flags

Use version-based feature flags:

```cpp
struct FeatureFlags {
    bool has_profiles = false;
    bool has_analytics = false;
    
    static FeatureFlags for_version(const ModuleVersion& v) {
        FeatureFlags flags;
        if (v >= ModuleVersion{1, 1, 0}) {
            flags.has_profiles = true;
        }
        if (v >= ModuleVersion{1, 2, 0}) {
            flags.has_analytics = true;
        }
        return flags;
    }
};
```

### 4. Migration Testing

Always test migrations thoroughly:

```cpp
// Test migration up and down
void test_migration() {
    auto v1 = ModuleVersion{1, 0, 0};
    auto v2 = ModuleVersion{2, 0, 0};
    
    MigrationContext ctx(nullptr, v1, v2);
    
    MigrateV1ToV2 migration;
    
    // Test forward migration
    migration.up(ctx);
    assert(ctx.table_exists("user_profiles"));
    
    // Test rollback
    migration.down(ctx);
    assert(!ctx.table_exists("user_profiles"));
}
```

## Example: Complete Versioned Module

```cpp
#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/versioning.h>
#include <spacetimedb/migration.h>

// Version 2.1.0
SPACETIMEDB_MODULE_VERSION(2, 1, 0)

SPACETIMEDB_MODULE_METADATA(
    "UserService",
    "MyCompany",
    "User management service with profiles",
    "Apache-2.0"
)

// Schema
struct User {
    uint64_t id;
    std::string username;
    std::string email;
    std::string display_name;
    uint64_t created_at;
    uint64_t updated_at;
};

struct UserProfile {
    uint64_t user_id;
    std::optional<std::string> bio;
    std::optional<std::string> avatar_url;
};

// Register tables
SPACETIMEDB_TABLE(User, users, true)
SPACETIMEDB_TABLE(UserProfile, profiles, true)

// Version-aware reducer
SPACETIMEDB_REDUCER(create_user, ReducerContext ctx,
                   std::string username, std::string email) {
    auto features = FeatureFlags::for_version(MODULE_VERSION);
    
    User user{/* ... */};
    ctx.db.table<User>("users").insert(user);
    
    if (features.has_profiles) {
        UserProfile profile{/* ... */};
        ctx.db.table<UserProfile>("profiles").insert(profile);
    }
}

// Module info reducer
SPACETIMEDB_REDUCER(get_version_info, ReducerContext ctx) {
    auto version = MODULE_VERSION;
    log("Module version: " + version.to_string());
    log("API compatibility: " + 
        std::to_string(version.major) + ".x");
}
```

## Troubleshooting

### Common Issues

1. **Version Parse Errors**
   ```cpp
   // Invalid: Missing patch version
   auto v = ModuleVersion::parse("1.0");  // Error!
   
   // Valid
   auto v = ModuleVersion::parse("1.0.0"); // OK
   ```

2. **Migration Order**
   - Always define migrations in version order
   - Test migration paths thoroughly
   - Ensure rollback logic is correct

3. **Compatibility Checks**
   - Client version <= Module version (within same major)
   - Different major versions are always incompatible

### Debug Helpers

```cpp
// Log version information
void debug_version_info(const ModuleVersion& v) {
    log("Version: " + v.to_string());
    log("Major: " + std::to_string(v.major));
    log("Minor: " + std::to_string(v.minor));
    log("Patch: " + std::to_string(v.patch));
    if (v.prerelease) {
        log("Pre-release: " + v.prerelease.value());
    }
}

// Log migration plan
void debug_migration_plan(const MigrationPlan& plan) {
    log("Migration: " + plan.from_version().to_string() + 
        " -> " + plan.to_version().to_string());
    log("Automatic: " + std::to_string(plan.is_automatic()));
    log("Changes: " + std::to_string(plan.changes().size()));
}
```