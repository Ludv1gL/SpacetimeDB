#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <map>

namespace spacetimedb {

/**
 * Semantic version representation for module versioning
 */
struct ModuleVersion {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    std::optional<std::string> prerelease;
    std::optional<std::string> metadata;
    
    ModuleVersion(uint16_t maj = 0, uint16_t min = 0, uint16_t pat = 0)
        : major(maj), minor(min), patch(pat) {}
    
    // Parse from string (e.g., "1.2.3-alpha+build123")
    static ModuleVersion parse(const std::string& version_string);
    
    // Convert to string
    std::string to_string() const;
    
    // Compare versions
    bool operator==(const ModuleVersion& other) const;
    bool operator!=(const ModuleVersion& other) const;
    bool operator<(const ModuleVersion& other) const;
    bool operator>(const ModuleVersion& other) const;
    bool operator<=(const ModuleVersion& other) const;
    bool operator>=(const ModuleVersion& other) const;
    
    // Check if compatible with another version
    bool is_compatible_with(const ModuleVersion& other) const;
    
    // Check if this is a breaking change from another version
    bool is_breaking_change_from(const ModuleVersion& other) const;
};

/**
 * Module metadata including version information
 */
struct ModuleMetadata {
    std::string name;
    ModuleVersion version;
    std::string author;
    std::string description;
    std::string license;
    std::map<std::string, std::string> custom_metadata;
    
    // Serialization
    std::vector<uint8_t> to_bsatn() const;
    static ModuleMetadata from_bsatn(const uint8_t* data, size_t len);
};

/**
 * Schema change types for migration tracking
 */
enum class SchemaChangeType {
    TABLE_ADDED,
    TABLE_REMOVED,
    TABLE_RENAMED,
    COLUMN_ADDED,
    COLUMN_REMOVED,
    COLUMN_RENAMED,
    COLUMN_TYPE_CHANGED,
    INDEX_ADDED,
    INDEX_REMOVED,
    INDEX_MODIFIED,
    CONSTRAINT_ADDED,
    CONSTRAINT_REMOVED,
    SEQUENCE_ADDED,
    SEQUENCE_REMOVED,
    REDUCER_ADDED,
    REDUCER_REMOVED,
    REDUCER_SIGNATURE_CHANGED
};

/**
 * Represents a single schema change
 */
struct SchemaChange {
    SchemaChangeType type;
    std::string table_name;
    std::string object_name;  // column, index, constraint name etc.
    std::optional<std::string> old_value;  // for renames/type changes
    std::optional<std::string> new_value;
    
    // Check if this change is backward compatible
    bool is_backward_compatible() const;
};

/**
 * Migration step interface
 */
class MigrationStep {
public:
    virtual ~MigrationStep() = default;
    
    // Execute the migration step
    virtual bool execute() = 0;
    
    // Rollback the migration step (if possible)
    virtual bool rollback() = 0;
    
    // Get description of this step
    virtual std::string description() const = 0;
    
    // Check if this step can be automatically executed
    virtual bool is_automatic() const = 0;
};

/**
 * Table migration helper
 */
class TableMigration : public MigrationStep {
private:
    std::string table_name_;
    std::function<void()> up_;
    std::function<void()> down_;
    std::string desc_;
    
public:
    TableMigration(const std::string& table, 
                   std::function<void()> up,
                   std::function<void()> down,
                   const std::string& description)
        : table_name_(table), up_(up), down_(down), desc_(description) {}
    
    bool execute() override { up_(); return true; }
    bool rollback() override { down_(); return true; }
    std::string description() const override { return desc_; }
    bool is_automatic() const override { return true; }
};

/**
 * Migration plan for upgrading between module versions
 */
class MigrationPlan {
private:
    ModuleVersion from_version_;
    ModuleVersion to_version_;
    std::vector<std::unique_ptr<MigrationStep>> steps_;
    std::vector<SchemaChange> changes_;
    
public:
    MigrationPlan(const ModuleVersion& from, const ModuleVersion& to)
        : from_version_(from), to_version_(to) {}
    
    // Add a migration step
    void add_step(std::unique_ptr<MigrationStep> step);
    
    // Add a schema change
    void add_change(const SchemaChange& change);
    
    // Check if migration can be performed automatically
    bool is_automatic() const;
    
    // Check if migration is backward compatible
    bool is_backward_compatible() const;
    
    // Execute the migration plan
    bool execute();
    
    // Get summary of changes
    std::string summary() const;
    
    // Getters
    const ModuleVersion& from_version() const { return from_version_; }
    const ModuleVersion& to_version() const { return to_version_; }
    const std::vector<SchemaChange>& changes() const { return changes_; }
};

/**
 * Version registry for tracking module version history
 */
class VersionRegistry {
private:
    std::map<ModuleVersion, ModuleMetadata> versions_;
    std::map<std::pair<ModuleVersion, ModuleVersion>, MigrationPlan> migrations_;
    
public:
    // Register a module version
    void register_version(const ModuleVersion& version, const ModuleMetadata& metadata);
    
    // Register a migration between versions
    void register_migration(const ModuleVersion& from, const ModuleVersion& to,
                          MigrationPlan plan);
    
    // Find migration path between versions
    std::optional<std::vector<MigrationPlan*>> find_migration_path(
        const ModuleVersion& from, const ModuleVersion& to);
    
    // Get all registered versions
    std::vector<ModuleVersion> get_versions() const;
    
    // Check if a version is registered
    bool has_version(const ModuleVersion& version) const;
};

/**
 * Module versioning configuration
 */
struct VersioningConfig {
    bool allow_breaking_changes = false;
    bool auto_migrate = true;
    bool require_migration_for_major = true;
    bool backup_before_migration = true;
    std::string migration_table_name = "__migrations__";
};

/**
 * Main versioning manager
 */
class ModuleVersionManager {
private:
    ModuleMetadata current_metadata_;
    VersionRegistry registry_;
    VersioningConfig config_;
    
public:
    ModuleVersionManager(const ModuleMetadata& metadata, 
                        const VersioningConfig& config = {})
        : current_metadata_(metadata), config_(config) {}
    
    // Get current module version
    const ModuleVersion& current_version() const { 
        return current_metadata_.version; 
    }
    
    // Get current metadata
    const ModuleMetadata& metadata() const { 
        return current_metadata_; 
    }
    
    // Register a new version
    void register_version(const ModuleVersion& version, 
                         const ModuleMetadata& metadata);
    
    // Plan migration to a new version
    std::optional<MigrationPlan> plan_migration(const ModuleVersion& to_version);
    
    // Check if can upgrade to a version
    bool can_upgrade_to(const ModuleVersion& version) const;
    
    // Get migration history
    std::vector<std::pair<ModuleVersion, ModuleVersion>> get_migration_history() const;
};

/**
 * Helper macros for defining module versions and migrations
 */
#define SPACETIMEDB_MODULE_VERSION(major, minor, patch) \
    static const spacetimedb::ModuleVersion MODULE_VERSION{major, minor, patch};

#define SPACETIMEDB_MODULE_METADATA(name, author, desc, license) \
    static const spacetimedb::ModuleMetadata MODULE_METADATA{ \
        name, MODULE_VERSION, author, desc, license, {} \
    };

// Helper function to include version info in module description
void add_version_to_module_def(void* module_def_builder, const ModuleMetadata& metadata);

} // namespace spacetimedb