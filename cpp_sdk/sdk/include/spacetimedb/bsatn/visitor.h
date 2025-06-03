#ifndef SPACETIMEDB_BSATN_VISITOR_H
#define SPACETIMEDB_BSATN_VISITOR_H

#include "reader.h"
#include "algebraic_type.h"
#include <string>
#include <functional>
#include <optional>
#include <stdexcept>

namespace SpacetimeDb::bsatn {

/**
 * Visitor pattern for deserializing product types (structs).
 * This matches the Rust implementation's visitor pattern.
 */
template<typename Output>
class ProductVisitor {
public:
    using Result = Output;
    
    virtual ~ProductVisitor() = default;
    
    // Visit fields in order (for tuples/unnamed products)
    virtual Result visit_seq(Reader& reader, size_t field_count) = 0;
    
    // Visit named fields (for structs)
    virtual Result visit_named_seq(Reader& reader, 
                                   const std::vector<std::string>& field_names) {
        // Default implementation ignores names
        return visit_seq(reader, field_names.size());
    }
};

/**
 * Visitor for sum types (enums/discriminated unions).
 */
template<typename Output>
class SumVisitor {
public:
    using Result = Output;
    
    virtual ~SumVisitor() = default;
    
    // Visit a specific variant by tag
    virtual Result visit_variant(uint8_t tag, const std::string& name, Reader& reader) = 0;
};

/**
 * Visitor for array types.
 */
template<typename Output>
class ArrayVisitor {
public:
    using Result = Output;
    
    virtual ~ArrayVisitor() = default;
    
    // Visit array elements
    virtual Result visit_array(Reader& reader, size_t length) = 0;
};

/**
 * Access interface for product fields during deserialization.
 */
class ProductAccess {
public:
    virtual ~ProductAccess() = default;
    
    // Get the next field's value
    template<typename T>
    T next_field() {
        return deserialize<T>(*reader_);
    }
    
    // Get the number of fields
    virtual size_t field_count() const = 0;
    
protected:
    Reader* reader_;
    
    explicit ProductAccess(Reader* r) : reader_(r) {}
};

/**
 * Named product access with field names.
 */
class NamedProductAccess : public ProductAccess {
private:
    std::vector<std::string> field_names_;
    size_t current_field_ = 0;
    
public:
    NamedProductAccess(Reader* r, std::vector<std::string> names)
        : ProductAccess(r), field_names_(std::move(names)) {}
    
    // Get the next field's name
    std::optional<std::string> next_field_name() {
        if (current_field_ < field_names_.size()) {
            return field_names_[current_field_++];
        }
        return std::nullopt;
    }
    
    size_t field_count() const override {
        return field_names_.size();
    }
};

/**
 * Error types for BSATN operations (matching Rust's error types).
 */
enum class DecodeErrorKind {
    MissingField,
    UnknownField,
    InvalidTag,
    InvalidUtf8,
    InvalidBool,
    BufferTooSmall,
    Custom
};

class DecodeError : public std::runtime_error {
private:
    DecodeErrorKind kind_;
    std::string context_;
    
public:
    DecodeError(DecodeErrorKind kind, const std::string& message)
        : std::runtime_error(message), kind_(kind) {}
    
    DecodeError(DecodeErrorKind kind, const std::string& message, const std::string& context)
        : std::runtime_error(message + " (context: " + context + ")")
        , kind_(kind)
        , context_(context) {}
    
    DecodeErrorKind kind() const { return kind_; }
    const std::string& context() const { return context_; }
    
    // Factory methods for common errors
    static DecodeError missing_field(const std::string& field_name) {
        return DecodeError(DecodeErrorKind::MissingField, 
                          "Missing required field: " + field_name);
    }
    
    static DecodeError unknown_field(const std::string& field_name) {
        return DecodeError(DecodeErrorKind::UnknownField, 
                          "Unknown field: " + field_name);
    }
    
    static DecodeError invalid_tag(uint8_t expected, uint8_t found) {
        return DecodeError(DecodeErrorKind::InvalidTag,
                          "Invalid tag: expected " + std::to_string(expected) + 
                          ", found " + std::to_string(found));
    }
};

/**
 * Deserialize with seed - allows stateful deserialization.
 */
template<typename T>
class DeserializeSeed {
public:
    using Value = T;
    
    virtual ~DeserializeSeed() = default;
    
    // Deserialize using this seed
    virtual T deserialize(Reader& reader) = 0;
};

/**
 * Field name validator.
 */
class FieldNameValidator {
private:
    std::vector<std::string> expected_fields_;
    std::vector<bool> seen_fields_;
    
public:
    explicit FieldNameValidator(std::vector<std::string> expected)
        : expected_fields_(std::move(expected))
        , seen_fields_(expected_fields_.size(), false) {}
    
    void validate_field(const std::string& field_name) {
        auto it = std::find(expected_fields_.begin(), expected_fields_.end(), field_name);
        if (it == expected_fields_.end()) {
            throw DecodeError::unknown_field(field_name);
        }
        
        size_t index = std::distance(expected_fields_.begin(), it);
        if (seen_fields_[index]) {
            throw DecodeError(DecodeErrorKind::Custom, "Duplicate field: " + field_name);
        }
        seen_fields_[index] = true;
    }
    
    void validate_complete() const {
        for (size_t i = 0; i < expected_fields_.size(); ++i) {
            if (!seen_fields_[i]) {
                throw DecodeError::missing_field(expected_fields_[i]);
            }
        }
    }
};

} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_VISITOR_H