#ifndef SPACETIMEDB_VALIDATED_TABLE_H
#define SPACETIMEDB_VALIDATED_TABLE_H

#include "spacetimedb/table_ops.h"
#include "spacetimedb/constraint_validation.h"
#include <memory>
#include <mutex>

namespace spacetimedb {

/**
 * @brief Table wrapper that performs constraint validation on operations.
 * 
 * This class wraps the basic TableOps to add constraint validation
 * before insert/update operations and maintains constraint caches.
 * 
 * @tparam T The row type for this table
 * @ingroup sdk_constraints sdk_tables
 */
template<typename T>
class ValidatedTable : public SpacetimeDb::TableOps<T> {
private:
    std::unique_ptr<TableConstraintManager<T>> constraint_manager_;
    mutable std::mutex cache_mutex_;
    
public:
    using Base = SpacetimeDb::TableOps<T>;
    using typename Base::Row;
    using typename Base::InsertError;
    
    ValidatedTable(uint32_t table_id, const std::string& name)
        : Base(table_id, name), 
          constraint_manager_(std::make_unique<TableConstraintManager<T>>(name)) {}
    
    // Get constraint manager for configuration
    TableConstraintManager<T>& constraints() { return *constraint_manager_; }
    const TableConstraintManager<T>& constraints() const { return *constraint_manager_; }
    
    // Override insert to add validation
    T insert(const T& row) {
        // Validate before insert
        constraint_manager_->validate_or_throw(row);
        
        // Perform the actual insert
        return Base::insert(row);
    }
    
    // Override try_insert to add validation
    std::optional<InsertError> try_insert(T& row) {
        // Validate constraints first
        auto validation_result = constraint_manager_->validate(row);
        if (!validation_result.is_valid()) {
            // Convert validation errors to InsertError
            for (const auto& violation : validation_result.violations()) {
                if (violation.type == ConstraintViolation::UNIQUE ||
                    violation.type == ConstraintViolation::PRIMARY_KEY) {
                    return InsertError{InsertError::UniqueViolation, violation.message};
                }
            }
            // Return generic error for other violations
            return InsertError{InsertError::UniqueViolation, validation_result.to_string()};
        }
        
        // Try the actual insert
        return Base::try_insert(row);
    }
    
    // Batch insert with validation
    std::vector<T> insert_batch(const std::vector<T>& rows) {
        std::vector<T> inserted;
        inserted.reserve(rows.size());
        
        // Validate all rows first
        for (const auto& row : rows) {
            constraint_manager_->validate_or_throw(row);
        }
        
        // Insert all rows
        for (const auto& row : rows) {
            inserted.push_back(Base::insert(row));
        }
        
        return inserted;
    }
    
    // Update operation with validation
    bool update(const T& old_row, const T& new_row) {
        // Validate new row
        constraint_manager_->validate_or_throw(new_row);
        
        // Delete old and insert new
        bool deleted = Base::delete_row(old_row);
        if (deleted) {
            Base::insert(new_row);
        }
        return deleted;
    }
    
    // Upsert operation
    T upsert(const T& row, std::function<bool(const T&, const T&)> match_predicate) {
        // Find existing row
        for (auto existing : Base::iter()) {
            if (match_predicate(existing, row)) {
                // Update existing
                Base::delete_row(existing);
                return insert(row);
            }
        }
        
        // Insert new
        return insert(row);
    }
};

/**
 * @brief Factory for creating validated tables with common constraints.
 * @ingroup sdk_constraints sdk_tables
 */
class ValidatedTableFactory {
public:
    /**
     * @brief Create a validated table with primary key constraint.
     */
    template<typename T, typename KeyType>
    static std::unique_ptr<ValidatedTable<T>> create_with_primary_key(
        uint32_t table_id,
        const std::string& table_name,
        const std::string& key_column,
        std::function<KeyType(const T&)> key_extractor) {
        
        auto table = std::make_unique<ValidatedTable<T>>(table_id, table_name);
        
        // Add primary key constraint (unique + not null)
        table->constraints().add_validator(
            std::make_unique<UniqueValidator<T, KeyType>>(
                "pk_" + key_column,
                std::vector<std::string>{key_column},
                key_extractor
            )
        );
        
        return table;
    }
    
    /**
     * @brief Create a validated table with composite primary key.
     */
    template<typename T, typename KeyType>
    static std::unique_ptr<ValidatedTable<T>> create_with_composite_key(
        uint32_t table_id,
        const std::string& table_name,
        const std::vector<std::string>& key_columns,
        std::function<KeyType(const T&)> key_extractor) {
        
        auto table = std::make_unique<ValidatedTable<T>>(table_id, table_name);
        
        // Add composite key constraint
        table->constraints().add_validator(
            std::make_unique<UniqueValidator<T, KeyType>>(
                "pk_composite",
                key_columns,
                key_extractor
            )
        );
        
        return table;
    }
};

/**
 * @brief RAII transaction wrapper for constraint validation.
 * 
 * Temporarily disables constraint validation within a scope,
 * useful for bulk operations or migrations.
 * 
 * @ingroup sdk_constraints
 */
template<typename T>
class ValidationTransaction {
private:
    ValidatedTable<T>& table_;
    bool previous_state_;
    
public:
    explicit ValidationTransaction(ValidatedTable<T>& table, bool enable_validation = false)
        : table_(table), previous_state_(table.constraints().is_validation_enabled()) {
        table_.constraints().set_validation_enabled(enable_validation);
    }
    
    ~ValidationTransaction() {
        table_.constraints().set_validation_enabled(previous_state_);
    }
    
    // Non-copyable, non-movable
    ValidationTransaction(const ValidationTransaction&) = delete;
    ValidationTransaction& operator=(const ValidationTransaction&) = delete;
    ValidationTransaction(ValidationTransaction&&) = delete;
    ValidationTransaction& operator=(ValidationTransaction&&) = delete;
};

/**
 * @brief Helper macros for defining validated tables with constraints.
 */

// Define a validated table with primary key
#define SPACETIMEDB_VALIDATED_TABLE_WITH_PK(Type, table_name, key_field, is_public) \
    SPACETIMEDB_TABLE(Type, table_name, is_public) \
    inline auto create_validated_##table_name(uint32_t table_id) { \
        return ValidatedTableFactory::create_with_primary_key<Type, decltype(std::declval<Type>().key_field)>( \
            table_id, #table_name, #key_field, \
            [](const Type& row) { return row.key_field; } \
        ); \
    }

// Define a validated table with custom constraints
#define SPACETIMEDB_VALIDATED_TABLE(Type, table_name, is_public) \
    SPACETIMEDB_TABLE(Type, table_name, is_public) \
    inline auto create_validated_##table_name(uint32_t table_id) { \
        return std::make_unique<ValidatedTable<Type>>(table_id, #table_name); \
    }

/**
 * @brief Constraint definition helpers for use with ConstraintBuilder.
 */
namespace constraints {

// Common check constraint predicates
template<typename T, typename FieldType>
auto range_check(FieldType T::*field, FieldType min_val, FieldType max_val) {
    return [field, min_val, max_val](const T& row) {
        return row.*field >= min_val && row.*field <= max_val;
    };
}

template<typename T>
auto string_length_check(std::string T::*field, size_t max_length) {
    return [field, max_length](const T& row) {
        return (row.*field).length() <= max_length;
    };
}

template<typename T>
auto string_pattern_check(std::string T::*field, const std::string& pattern) {
    return [field, pattern](const T& row) {
        // Simple pattern matching - in production, use regex
        return (row.*field).find(pattern) != std::string::npos;
    };
}

// Key extractors for common cases
template<typename T, typename FieldType>
auto single_field_key(FieldType T::*field) {
    return [field](const T& row) { return row.*field; };
}

template<typename T, typename Field1, typename Field2>
auto composite_key(Field1 T::*field1, Field2 T::*field2) {
    return [field1, field2](const T& row) {
        return std::make_pair(row.*field1, row.*field2);
    };
}

} // namespace constraints

} // namespace spacetimedb

#endif // SPACETIMEDB_VALIDATED_TABLE_H