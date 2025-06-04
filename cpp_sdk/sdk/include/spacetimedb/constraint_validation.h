#ifndef SPACETIMEDB_CONSTRAINT_VALIDATION_H
#define SPACETIMEDB_CONSTRAINT_VALIDATION_H

#include <vector>
#include <map>
#include <set>
#include <functional>
#include <memory>
#include <string>
#include <optional>
#include <variant>
#include <type_traits>
#include "spacetimedb/bsatn/bsatn.h"
#include "spacetimedb/sdk/exceptions.h"
#include "spacetimedb/sdk/schema_management.h"

namespace spacetimedb {

/**
 * @brief Constraint validation result containing error details.
 * @ingroup sdk_constraints
 */
struct ConstraintViolation {
    enum Type {
        NOT_NULL,           ///< NULL value in NOT NULL column
        UNIQUE,             ///< Duplicate value in unique column
        PRIMARY_KEY,        ///< Primary key violation
        CHECK,              ///< Check constraint failed
        FOREIGN_KEY,        ///< Foreign key constraint failed
        AUTO_INCREMENT,     ///< Auto-increment overflow
        DATA_TYPE           ///< Data type validation failed
    };
    
    Type type;
    std::string constraint_name;
    std::string column_name;
    std::string message;
    std::optional<std::string> details;
    
    ConstraintViolation(Type t, const std::string& name, const std::string& col, const std::string& msg)
        : type(t), constraint_name(name), column_name(col), message(msg) {}
};

/**
 * @brief Result of constraint validation.
 * @ingroup sdk_constraints
 */
class ValidationResult {
private:
    std::vector<ConstraintViolation> violations_;
    
public:
    ValidationResult() = default;
    
    bool is_valid() const { return violations_.empty(); }
    
    void add_violation(const ConstraintViolation& violation) {
        violations_.push_back(violation);
    }
    
    void add_violation(ConstraintViolation&& violation) {
        violations_.push_back(std::move(violation));
    }
    
    const std::vector<ConstraintViolation>& violations() const { return violations_; }
    
    std::string to_string() const {
        if (is_valid()) return "Valid";
        
        std::string result = "Constraint violations:\n";
        for (const auto& v : violations_) {
            result += "  - " + v.message + "\n";
            if (v.details) {
                result += "    Details: " + *v.details + "\n";
            }
        }
        return result;
    }
    
    // Throw exception if validation failed
    void throw_if_invalid() const {
        if (!is_valid()) {
            throw SpacetimeDB::UniqueConstraintViolationException(to_string());
        }
    }
};

/**
 * @brief Base interface for constraint validators.
 * @ingroup sdk_constraints
 */
template<typename T>
class IConstraintValidator {
public:
    virtual ~IConstraintValidator() = default;
    virtual ValidationResult validate(const T& row) const = 0;
    virtual std::string name() const = 0;
};

/**
 * @brief NOT NULL constraint validator.
 * @ingroup sdk_constraints
 */
template<typename T, typename FieldType>
class NotNullValidator : public IConstraintValidator<T> {
private:
    std::string column_name_;
    std::function<const FieldType*(const T&)> field_getter_;
    
public:
    NotNullValidator(const std::string& column, std::function<const FieldType*(const T&)> getter)
        : column_name_(column), field_getter_(getter) {}
    
    ValidationResult validate(const T& row) const override {
        ValidationResult result;
        const FieldType* value = field_getter_(row);
        
        if constexpr (std::is_pointer_v<FieldType>) {
            if (value == nullptr || *value == nullptr) {
                result.add_violation(ConstraintViolation(
                    ConstraintViolation::NOT_NULL,
                    "not_null_" + column_name_,
                    column_name_,
                    "Column '" + column_name_ + "' cannot be NULL"
                ));
            }
        } else if constexpr (std::is_same_v<FieldType, std::optional<typename FieldType::value_type>>) {
            if (!value->has_value()) {
                result.add_violation(ConstraintViolation(
                    ConstraintViolation::NOT_NULL,
                    "not_null_" + column_name_,
                    column_name_,
                    "Column '" + column_name_ + "' cannot be NULL"
                ));
            }
        }
        
        return result;
    }
    
    std::string name() const override { return "NOT NULL on " + column_name_; }
};

/**
 * @brief Check constraint validator.
 * @ingroup sdk_constraints
 */
template<typename T>
class CheckValidator : public IConstraintValidator<T> {
private:
    std::string constraint_name_;
    std::function<bool(const T&)> predicate_;
    std::string expression_;
    
public:
    CheckValidator(const std::string& name, std::function<bool(const T&)> pred, const std::string& expr)
        : constraint_name_(name), predicate_(pred), expression_(expr) {}
    
    ValidationResult validate(const T& row) const override {
        ValidationResult result;
        
        if (!predicate_(row)) {
            result.add_violation(ConstraintViolation(
                ConstraintViolation::CHECK,
                constraint_name_,
                "",  // Check constraints may involve multiple columns
                "Check constraint '" + constraint_name_ + "' failed: " + expression_
            ));
        }
        
        return result;
    }
    
    std::string name() const override { return "CHECK " + constraint_name_; }
};

/**
 * @brief Data type constraint validator.
 * @ingroup sdk_constraints
 */
template<typename T, typename FieldType>
class DataTypeValidator : public IConstraintValidator<T> {
private:
    std::string column_name_;
    std::function<const FieldType*(const T&)> field_getter_;
    std::function<bool(const FieldType&)> type_checker_;
    std::string type_description_;
    
public:
    DataTypeValidator(const std::string& column, 
                     std::function<const FieldType*(const T&)> getter,
                     std::function<bool(const FieldType&)> checker,
                     const std::string& type_desc)
        : column_name_(column), field_getter_(getter), 
          type_checker_(checker), type_description_(type_desc) {}
    
    ValidationResult validate(const T& row) const override {
        ValidationResult result;
        const FieldType* value = field_getter_(row);
        
        if (value && !type_checker_(*value)) {
            result.add_violation(ConstraintViolation(
                ConstraintViolation::DATA_TYPE,
                "type_" + column_name_,
                column_name_,
                "Column '" + column_name_ + "' must be " + type_description_
            ));
        }
        
        return result;
    }
    
    std::string name() const override { return "TYPE " + column_name_ + " " + type_description_; }
};

/**
 * @brief Unique constraint cache for runtime validation.
 * @ingroup sdk_constraints
 */
template<typename KeyType>
class UniqueConstraintCache {
private:
    mutable std::set<KeyType> unique_values_;
    mutable bool cache_valid_ = false;
    
public:
    void invalidate() { 
        cache_valid_ = false; 
        unique_values_.clear();
    }
    
    bool contains(const KeyType& key) const {
        return unique_values_.find(key) != unique_values_.end();
    }
    
    void add(const KeyType& key) {
        unique_values_.insert(key);
    }
    
    void remove(const KeyType& key) {
        unique_values_.erase(key);
    }
    
    bool is_valid() const { return cache_valid_; }
    void mark_valid() { cache_valid_ = true; }
};

/**
 * @brief Unique constraint validator with caching.
 * @ingroup sdk_constraints
 */
template<typename T, typename KeyType>
class UniqueValidator : public IConstraintValidator<T> {
private:
    std::string constraint_name_;
    std::vector<std::string> column_names_;
    std::function<KeyType(const T&)> key_extractor_;
    std::shared_ptr<UniqueConstraintCache<KeyType>> cache_;
    
public:
    UniqueValidator(const std::string& name,
                   const std::vector<std::string>& columns,
                   std::function<KeyType(const T&)> extractor,
                   std::shared_ptr<UniqueConstraintCache<KeyType>> cache = nullptr)
        : constraint_name_(name), column_names_(columns), 
          key_extractor_(extractor), cache_(cache) {
        if (!cache_) {
            cache_ = std::make_shared<UniqueConstraintCache<KeyType>>();
        }
    }
    
    ValidationResult validate(const T& row) const override {
        ValidationResult result;
        KeyType key = key_extractor_(row);
        
        // Check cache if available
        if (cache_->is_valid() && cache_->contains(key)) {
            std::string columns_str;
            for (size_t i = 0; i < column_names_.size(); ++i) {
                if (i > 0) columns_str += ", ";
                columns_str += column_names_[i];
            }
            
            result.add_violation(ConstraintViolation(
                ConstraintViolation::UNIQUE,
                constraint_name_,
                columns_str,
                "Duplicate value in unique constraint '" + constraint_name_ + "' on columns (" + columns_str + ")"
            ));
        }
        
        return result;
    }
    
    std::string name() const override { return "UNIQUE " + constraint_name_; }
    
    // Cache management
    void update_cache(const KeyType& key, bool insert) {
        if (insert) {
            cache_->add(key);
        } else {
            cache_->remove(key);
        }
    }
    
    void invalidate_cache() { cache_->invalidate(); }
};

/**
 * @brief Composite constraint validator that combines multiple validators.
 * @ingroup sdk_constraints
 */
template<typename T>
class CompositeValidator : public IConstraintValidator<T> {
private:
    std::vector<std::unique_ptr<IConstraintValidator<T>>> validators_;
    
public:
    CompositeValidator() = default;
    
    void add_validator(std::unique_ptr<IConstraintValidator<T>> validator) {
        validators_.push_back(std::move(validator));
    }
    
    ValidationResult validate(const T& row) const override {
        ValidationResult result;
        
        for (const auto& validator : validators_) {
            auto sub_result = validator->validate(row);
            for (const auto& violation : sub_result.violations()) {
                result.add_violation(violation);
            }
        }
        
        return result;
    }
    
    std::string name() const override { return "Composite validator"; }
    
    size_t validator_count() const { return validators_.size(); }
};

/**
 * @brief Builder for creating constraint validators.
 * @ingroup sdk_constraints
 */
template<typename T>
class ConstraintBuilder {
private:
    CompositeValidator<T> validator_;
    
public:
    ConstraintBuilder& not_null(const std::string& column, 
                               std::function<const auto*(const T&)> getter) {
        using FieldType = std::remove_pointer_t<decltype(getter(std::declval<T>()))>;
        validator_.add_validator(
            std::make_unique<NotNullValidator<T, FieldType>>(column, getter)
        );
        return *this;
    }
    
    ConstraintBuilder& check(const std::string& name, 
                            std::function<bool(const T&)> predicate,
                            const std::string& expression) {
        validator_.add_validator(
            std::make_unique<CheckValidator<T>>(name, predicate, expression)
        );
        return *this;
    }
    
    template<typename KeyType>
    ConstraintBuilder& unique(const std::string& name,
                             const std::vector<std::string>& columns,
                             std::function<KeyType(const T&)> key_extractor) {
        validator_.add_validator(
            std::make_unique<UniqueValidator<T, KeyType>>(name, columns, key_extractor)
        );
        return *this;
    }
    
    ConstraintBuilder& data_type(const std::string& column,
                                std::function<const auto*(const T&)> getter,
                                std::function<bool(const auto&)> type_checker,
                                const std::string& type_description) {
        using FieldType = std::remove_pointer_t<decltype(getter(std::declval<T>()))>;
        validator_.add_validator(
            std::make_unique<DataTypeValidator<T, FieldType>>(
                column, getter, type_checker, type_description
            )
        );
        return *this;
    }
    
    CompositeValidator<T> build() {
        return std::move(validator_);
    }
};

/**
 * @brief Table constraint manager for managing all constraints on a table.
 * @ingroup sdk_constraints
 */
template<typename T>
class TableConstraintManager {
private:
    std::string table_name_;
    CompositeValidator<T> validator_;
    bool validation_enabled_ = true;
    
public:
    explicit TableConstraintManager(const std::string& table_name) 
        : table_name_(table_name) {}
    
    // Enable/disable validation
    void set_validation_enabled(bool enabled) { validation_enabled_ = enabled; }
    bool is_validation_enabled() const { return validation_enabled_; }
    
    // Add validator
    void add_validator(std::unique_ptr<IConstraintValidator<T>> validator) {
        validator_.add_validator(std::move(validator));
    }
    
    // Validate a row
    ValidationResult validate(const T& row) const {
        if (!validation_enabled_) {
            return ValidationResult();  // Return valid result if disabled
        }
        return validator_.validate(row);
    }
    
    // Validate with exception throwing
    void validate_or_throw(const T& row) const {
        if (validation_enabled_) {
            auto result = validate(row);
            result.throw_if_invalid();
        }
    }
    
    // Get constraint count
    size_t constraint_count() const { return validator_.validator_count(); }
};

} // namespace spacetimedb

#endif // SPACETIMEDB_CONSTRAINT_VALIDATION_H