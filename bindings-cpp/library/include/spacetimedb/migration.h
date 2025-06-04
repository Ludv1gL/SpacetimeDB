#pragma once

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/versioning.h>
#include <functional>
#include <memory>

namespace spacetimedb {

/**
 * Migration context provides access to database during migrations
 */
class MigrationContext {
private:
    ReducerContext* ctx_;
    ModuleVersion from_version_;
    ModuleVersion to_version_;
    
public:
    MigrationContext(ReducerContext* ctx, 
                    const ModuleVersion& from, 
                    const ModuleVersion& to)
        : ctx_(ctx), from_version_(from), to_version_(to) {}
    
    // Access to database
    ModuleDatabase& db() { return ctx_->db; }
    
    // Version info
    const ModuleVersion& from_version() const { return from_version_; }
    const ModuleVersion& to_version() const { return to_version_; }
    
    // Migration helpers
    template<typename T>
    void migrate_table(const std::string& old_name, const std::string& new_name,
                      std::function<T(const T&)> transform);
    
    // Check if table exists
    bool table_exists(const std::string& name) const;
    
    // Get row count for a table
    size_t table_row_count(const std::string& name) const;
    
    // Logging
    void log_info(const std::string& message);
    void log_warning(const std::string& message);
    void log_error(const std::string& message);
};

/**
 * Base class for custom migrations
 */
class Migration {
protected:
    ModuleVersion from_version_;
    ModuleVersion to_version_;
    
public:
    Migration(const ModuleVersion& from, const ModuleVersion& to)
        : from_version_(from), to_version_(to) {}
    
    virtual ~Migration() = default;
    
    // Execute the migration
    virtual void up(MigrationContext& ctx) = 0;
    
    // Rollback the migration (if possible)
    virtual void down(MigrationContext& ctx) = 0;
    
    // Get description
    virtual std::string description() const = 0;
    
    // Check if can auto-migrate
    virtual bool is_automatic() const { return false; }
    
    // Version getters
    const ModuleVersion& from_version() const { return from_version_; }
    const ModuleVersion& to_version() const { return to_version_; }
};

/**
 * Schema migration for structural changes
 */
class SchemaMigration : public Migration {
private:
    std::vector<SchemaChange> changes_;
    
public:
    using Migration::Migration;
    
    // Add a schema change
    void add_change(const SchemaChange& change) {
        changes_.push_back(change);
    }
    
    // Implementation
    void up(MigrationContext& ctx) override;
    void down(MigrationContext& ctx) override;
    std::string description() const override;
    bool is_automatic() const override;
    
    const std::vector<SchemaChange>& changes() const { return changes_; }
};

/**
 * Data migration for transforming existing data
 */
class DataMigration : public Migration {
private:
    std::function<void(MigrationContext&)> up_fn_;
    std::function<void(MigrationContext&)> down_fn_;
    std::string desc_;
    
public:
    DataMigration(const ModuleVersion& from, const ModuleVersion& to,
                  std::function<void(MigrationContext&)> up,
                  std::function<void(MigrationContext&)> down,
                  const std::string& description)
        : Migration(from, to), up_fn_(up), down_fn_(down), desc_(description) {}
    
    void up(MigrationContext& ctx) override { up_fn_(ctx); }
    void down(MigrationContext& ctx) override { down_fn_(ctx); }
    std::string description() const override { return desc_; }
};

/**
 * Migration builder for fluent API
 */
class MigrationBuilder {
private:
    ModuleVersion from_;
    ModuleVersion to_;
    std::vector<std::unique_ptr<Migration>> migrations_;
    
public:
    MigrationBuilder(const ModuleVersion& from, const ModuleVersion& to)
        : from_(from), to_(to) {}
    
    // Add table
    MigrationBuilder& add_table(const std::string& name);
    
    // Remove table
    MigrationBuilder& remove_table(const std::string& name);
    
    // Rename table
    MigrationBuilder& rename_table(const std::string& old_name, 
                                  const std::string& new_name);
    
    // Add column
    MigrationBuilder& add_column(const std::string& table,
                                const std::string& column,
                                const std::string& type,
                                const std::optional<std::string>& default_value = {});
    
    // Remove column
    MigrationBuilder& remove_column(const std::string& table,
                                   const std::string& column);
    
    // Rename column
    MigrationBuilder& rename_column(const std::string& table,
                                   const std::string& old_name,
                                   const std::string& new_name);
    
    // Add index
    MigrationBuilder& add_index(const std::string& table,
                               const std::string& index_name,
                               const std::vector<std::string>& columns);
    
    // Remove index
    MigrationBuilder& remove_index(const std::string& index_name);
    
    // Add custom migration
    MigrationBuilder& add_migration(std::unique_ptr<Migration> migration);
    
    // Add data migration
    MigrationBuilder& transform_data(const std::string& description,
                                    std::function<void(MigrationContext&)> up,
                                    std::function<void(MigrationContext&)> down);
    
    // Build the migrations
    std::vector<std::unique_ptr<Migration>> build();
};

/**
 * Migration registry for managing migrations
 */
class MigrationRegistry {
private:
    static MigrationRegistry* instance_;
    std::map<std::pair<ModuleVersion, ModuleVersion>, 
             std::vector<std::unique_ptr<Migration>>> migrations_;
    
    MigrationRegistry() = default;
    
public:
    static MigrationRegistry& instance();
    
    // Register a migration
    void register_migration(const ModuleVersion& from,
                          const ModuleVersion& to,
                          std::unique_ptr<Migration> migration);
    
    // Register multiple migrations
    void register_migrations(const ModuleVersion& from,
                           const ModuleVersion& to,
                           std::vector<std::unique_ptr<Migration>> migrations);
    
    // Get migrations for version transition
    std::vector<Migration*> get_migrations(const ModuleVersion& from,
                                         const ModuleVersion& to);
    
    // Find migration path
    std::optional<std::vector<Migration*>> find_migration_path(
        const ModuleVersion& from, const ModuleVersion& to);
};

/**
 * Helper macros for defining migrations
 */
#define DEFINE_MIGRATION(name, from_maj, from_min, from_patch, \
                        to_maj, to_min, to_patch) \
    class name : public spacetimedb::Migration { \
    public: \
        name() : Migration(ModuleVersion{from_maj, from_min, from_patch}, \
                          ModuleVersion{to_maj, to_min, to_patch}) {} \
        void up(spacetimedb::MigrationContext& ctx) override; \
        void down(spacetimedb::MigrationContext& ctx) override; \
        std::string description() const override; \
    };

#define REGISTER_MIGRATION(migration_class) \
    static bool _reg_##migration_class = []() { \
        auto m = std::make_unique<migration_class>(); \
        spacetimedb::MigrationRegistry::instance().register_migration( \
            m->from_version(), m->to_version(), std::move(m)); \
        return true; \
    }();

/**
 * Built-in migration reducers
 */

// Called when module needs migration
SPACETIMEDB_REDUCER(__migrate__, spacetimedb::ReducerContext ctx, 
                   std::string from_version, std::string to_version);

// Get current module version
SPACETIMEDB_REDUCER(__get_module_version__, spacetimedb::ReducerContext ctx);

// List available migrations
SPACETIMEDB_REDUCER(__list_migrations__, spacetimedb::ReducerContext ctx);

} // namespace spacetimedb