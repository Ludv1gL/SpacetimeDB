#ifndef SPACETIMEDB_SDK_SCHEMA_MANAGEMENT_H
#define SPACETIMEDB_SDK_SCHEMA_MANAGEMENT_H

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <memory>
#include "spacetimedb/bsatn/bsatn.h"

namespace spacetimedb {

// Forward declaration for index definitions
struct RawIndexDefV9 {
    std::optional<std::string> name;
    std::string accessor_name;
    // TODO: Add algorithm field when RawIndexAlgorithm is available
    std::vector<uint16_t> columns;  // Simple column list for now
    
    RawIndexDefV9(std::optional<std::string> index_name, std::string accessor, std::vector<uint16_t> cols)
        : name(std::move(index_name)), accessor_name(std::move(accessor)), columns(std::move(cols)) {}
};

/**
 * Column attribute flags (matches C# ColumnAttrs)
 */
enum class ColumnAttrs : uint8_t {
    UnSet = 0b0000,
    Indexed = 0b0001,
    AutoInc = 0b0010,
    Unique = Indexed | 0b0100,
    Identity = Unique | AutoInc,      // Unique + AutoInc
    PrimaryKey = Unique | 0b1000,
    PrimaryKeyAuto = PrimaryKey | AutoInc  // PrimaryKey + AutoInc
};

// Bitwise operations for ColumnAttrs
constexpr ColumnAttrs operator|(ColumnAttrs a, ColumnAttrs b) {
    return static_cast<ColumnAttrs>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr ColumnAttrs operator&(ColumnAttrs a, ColumnAttrs b) {
    return static_cast<ColumnAttrs>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

constexpr bool has_attr(ColumnAttrs attrs, ColumnAttrs check) {
    return (attrs & check) == check;
}

/**
 * Table access types
 */
enum class TableAccess : uint8_t {
    Public = 0,
    Private = 1
};

/**
 * Table types
 */
enum class TableType : uint8_t {
    System = 0,
    User = 1
};

/**
 * Unique constraint data for composite constraints
 */
struct RawUniqueConstraintDataV9 {
    std::vector<uint16_t> columns;  // Column indices
    
    explicit RawUniqueConstraintDataV9(std::vector<uint16_t> cols) 
        : columns(std::move(cols)) {}
};

/**
 * Constraint data variant (extensible for future constraint types)
 */
enum class RawConstraintDataV9Tag : uint8_t {
    Unique = 0
};

struct RawConstraintDataV9 {
    RawConstraintDataV9Tag tag;
    std::unique_ptr<RawUniqueConstraintDataV9> unique_data;
    
    explicit RawConstraintDataV9(RawUniqueConstraintDataV9 unique_constraint) 
        : tag(RawConstraintDataV9Tag::Unique), 
          unique_data(std::make_unique<RawUniqueConstraintDataV9>(std::move(unique_constraint))) {}
    
    // Move constructor
    RawConstraintDataV9(RawConstraintDataV9&& other) noexcept
        : tag(other.tag), unique_data(std::move(other.unique_data)) {}
    
    // Move assignment
    RawConstraintDataV9& operator=(RawConstraintDataV9&& other) noexcept {
        if (this != &other) {
            tag = other.tag;
            unique_data = std::move(other.unique_data);
        }
        return *this;
    }
    
    // Deleted copy operations
    RawConstraintDataV9(const RawConstraintDataV9&) = delete;
    RawConstraintDataV9& operator=(const RawConstraintDataV9&) = delete;
};

/**
 * Constraint definition
 */
struct RawConstraintDefV9 {
    std::optional<std::string> name;
    RawConstraintDataV9 data;
    
    RawConstraintDefV9(std::optional<std::string> constraint_name, RawConstraintDataV9 constraint_data)
        : name(std::move(constraint_name)), data(std::move(constraint_data)) {}
};

/**
 * Sequence definition for auto-increment fields
 */
struct RawSequenceDefV9 {
    std::optional<std::string> name;
    uint16_t column;                        // Column index
    std::optional<int64_t> start;           // Starting value (using int64_t for now)
    std::optional<int64_t> min_value;       // Minimum allowed value
    std::optional<int64_t> max_value;       // Maximum allowed value
    int64_t increment;                      // Increment step (default: 1)
    
    explicit RawSequenceDefV9(uint16_t col, int64_t inc = 1)
        : column(col), increment(inc) {}
    
    RawSequenceDefV9& with_name(const std::string& seq_name) {
        name = seq_name;
        return *this;
    }
    
    RawSequenceDefV9& with_start(int64_t start_val) {
        start = start_val;
        return *this;
    }
    
    RawSequenceDefV9& with_range(int64_t min_val, int64_t max_val) {
        min_value = min_val;
        max_value = max_val;
        return *this;
    }
};

/**
 * Schedule definition for scheduled tables
 */
struct RawScheduleDefV9 {
    std::optional<std::string> name;
    std::string reducer_name;               // Reducer to invoke
    uint16_t scheduled_at_column;           // Column index for ScheduleAt field
    
    RawScheduleDefV9(std::string reducer, uint16_t col)
        : reducer_name(std::move(reducer)), scheduled_at_column(col) {}
        
    RawScheduleDefV9& with_name(const std::string& schedule_name) {
        name = schedule_name;
        return *this;
    }
};

/**
 * Row-level security definition
 */
struct RawRowLevelSecurityDefV9 {
    std::string sql;    // SQL filter expression
    
    explicit RawRowLevelSecurityDefV9(std::string sql_query)
        : sql(std::move(sql_query)) {}
};

/**
 * Complete table definition structure
 */
struct RawTableDefV9 {
    std::string name;
    uint32_t product_type_ref;                          // Type reference
    std::vector<uint16_t> primary_key;                  // Primary key column indices
    std::vector<RawIndexDefV9> indexes;                 // Index definitions
    std::vector<RawConstraintDefV9> constraints;        // Constraint definitions
    std::vector<RawSequenceDefV9> sequences;            // Sequence definitions
    std::optional<RawScheduleDefV9> schedule;           // Schedule definition
    TableType table_type;                               // System or User
    TableAccess table_access;                           // Public or Private
    
    RawTableDefV9(const std::string& table_name, uint32_t type_ref, TableAccess access)
        : name(table_name), product_type_ref(type_ref), table_type(TableType::User), table_access(access) {}
};

/**
 * Type validation helpers for schema attributes
 */
template<typename T>
constexpr bool is_integer_type() {
    return std::is_integral_v<T>;
    // TODO: Add support for int128_t, uint128_t when available
}

template<typename T>
constexpr bool is_equatable_type() {
    return is_integer_type<T>() || 
           std::is_same_v<T, std::string> ||
           std::is_same_v<T, bool>;
    // TODO: Add support for Identity, ConnectionId types
}

template<typename T>
constexpr bool is_scheduleat_type() {
    // TODO: Add support for ScheduleAt type when available
    return false;
}

/**
 * Column attribute validation
 */
template<typename T>
constexpr bool validate_autoinc_field() {
    static_assert(is_integer_type<T>(), "AutoInc fields must be integer types");
    return true;
}

template<typename T>
constexpr bool validate_unique_field() {
    static_assert(is_equatable_type<T>(), "Unique fields must be equatable types");
    return true;
}

template<typename T>
constexpr bool validate_primary_key_field() {
    static_assert(is_equatable_type<T>(), "Primary key fields must be equatable types");
    return true;
}

/**
 * Schema builder helper class
 */
class SchemaBuilder {
private:
    RawTableDefV9 table_def_;
    uint16_t column_counter_;
    
public:
    explicit SchemaBuilder(const std::string& table_name, uint32_t type_ref, TableAccess access)
        : table_def_(table_name, type_ref, access), column_counter_(0) {}
    
    // Add column attributes
    SchemaBuilder& add_primary_key_column(uint16_t column_index) {
        table_def_.primary_key.push_back(column_index);
        return *this;
    }
    
    SchemaBuilder& add_unique_constraint(const std::vector<uint16_t>& columns, 
                                       const std::optional<std::string>& name = std::nullopt) {
        auto unique_data = RawUniqueConstraintDataV9(columns);
        auto constraint_data = RawConstraintDataV9(std::move(unique_data));
        table_def_.constraints.emplace_back(name, std::move(constraint_data));
        return *this;
    }
    
    SchemaBuilder& add_sequence(uint16_t column, int64_t increment = 1) {
        table_def_.sequences.emplace_back(column, increment);
        return *this;
    }
    
    SchemaBuilder& add_schedule(const std::string& reducer_name, uint16_t scheduled_at_column) {
        table_def_.schedule = RawScheduleDefV9(reducer_name, scheduled_at_column);
        return *this;
    }
    
    SchemaBuilder& add_index(const RawIndexDefV9& index) {
        table_def_.indexes.push_back(index);
        return *this;
    }
    
    RawTableDefV9 build() {
        return std::move(table_def_);
    }
};

/**
 * Macros for column attributes
 */

// Mark field as auto-increment
#define SPACETIMEDB_AUTOINC(field_type, field_name) \
    static_assert(spacetimedb::validate_autoinc_field<field_type>(), \
                  "AutoInc field " #field_name " must be integer type"); \
    static constexpr auto _autoinc_##field_name = true;

// Mark field as unique
#define SPACETIMEDB_UNIQUE(field_type, field_name) \
    static_assert(spacetimedb::validate_unique_field<field_type>(), \
                  "Unique field " #field_name " must be equatable type"); \
    static constexpr auto _unique_##field_name = true;

// Mark field as primary key
#define SPACETIMEDB_PRIMARY_KEY(field_type, field_name) \
    static_assert(spacetimedb::validate_primary_key_field<field_type>(), \
                  "Primary key field " #field_name " must be equatable type"); \
    static constexpr auto _primary_key_##field_name = true;

// Mark field as identity (unique + auto-increment)
#define SPACETIMEDB_IDENTITY(field_type, field_name) \
    SPACETIMEDB_UNIQUE(field_type, field_name) \
    SPACETIMEDB_AUTOINC(field_type, field_name) \
    static constexpr auto _identity_##field_name = true;

// Mark field as primary key with auto-increment
#define SPACETIMEDB_PRIMARY_KEY_AUTO(field_type, field_name) \
    SPACETIMEDB_PRIMARY_KEY(field_type, field_name) \
    SPACETIMEDB_AUTOINC(field_type, field_name) \
    static constexpr auto _primary_key_auto_##field_name = true;

/**
 * Table-level schema macros
 */

// Define scheduled table
#define SPACETIMEDB_SCHEDULED_TABLE(table_type, table_name, reducer_name, scheduled_at_field, is_public) \
    SPACETIMEDB_TABLE(table_type, table_name, is_public) \
    static constexpr auto _scheduled_table_##table_name = true; \
    static constexpr auto _scheduled_reducer_##table_name = #reducer_name; \
    static constexpr auto _scheduled_field_##table_name = #scheduled_at_field;

// Define client visibility filter  
#define SPACETIMEDB_CLIENT_VISIBILITY_FILTER(name, sql_query) \
    static const char* __rls_filter_##name = sql_query; \
    static constexpr auto _rls_filter_##name = true;

// Define composite unique constraint
#define SPACETIMEDB_UNIQUE_CONSTRAINT(table_type, constraint_name, ...) \
    static constexpr auto _unique_constraint_##constraint_name = true; \
    static constexpr auto _unique_columns_##constraint_name = std::make_tuple(__VA_ARGS__);

/**
 * Schema metadata generation helpers
 */
namespace schema_utils {
    // Generate column attributes from field annotations
    template<typename T>
    ColumnAttrs get_column_attributes() {
        ColumnAttrs attrs = ColumnAttrs::UnSet;
        
        // TODO: Use reflection/template magic to detect field attributes
        // For now, return basic attributes
        
        return attrs;
    }
    
    // Generate table metadata from type information
    template<typename T>
    RawTableDefV9 generate_table_metadata(const std::string& name, bool is_public) {
        uint32_t type_ref = 0; // TODO: Get from type registry
        TableAccess access = is_public ? TableAccess::Public : TableAccess::Private;
        
        SchemaBuilder builder(name, type_ref, access);
        
        // TODO: Use template reflection to:
        // 1. Find fields with attributes
        // 2. Generate constraints
        // 3. Generate sequences
        // 4. Generate indexes
        
        return builder.build();
    }
    
    // Validate schema consistency
    template<typename T>
    bool validate_schema() {
        // TODO: Implement schema validation rules:
        // - Unique column names
        // - Valid attribute combinations
        // - Proper sequence field types
        // - Scheduled table requirements
        
        return true;
    }
}

} // namespace spacetimedb

#endif // SPACETIMEDB_SDK_SCHEMA_MANAGEMENT_H