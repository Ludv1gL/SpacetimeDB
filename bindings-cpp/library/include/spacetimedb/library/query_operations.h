#ifndef SPACETIMEDB_LIBRARY_QUERY_OPERATIONS_H
#define SPACETIMEDB_LIBRARY_QUERY_OPERATIONS_H

#include "index_management.h"
#include "spacetimedb/bsatn/bsatn.h"
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace spacetimedb {

// Forward declarations
using TableId = uint32_t;
using RowIter = uint32_t;

/**
 * FFI declarations for advanced query operations
 */
extern "C" {
    // Table operations
    int32_t datastore_table_row_count(TableId table_id, uint64_t* count);
    int32_t datastore_table_scan_bsatn(TableId table_id, RowIter* iter);
    
    // Row operations
    int32_t datastore_delete_all_by_eq_bsatn(TableId table_id, 
                                             const uint8_t* relation, uint32_t relation_len,
                                             uint32_t* deleted_count);
    
    // Iterator operations
    int32_t row_iter_bsatn_advance(RowIter iter, uint8_t* buffer, uint32_t* buffer_len);
    int32_t row_iter_bsatn_close(RowIter iter);
}

/**
 * Advanced table iterator with lazy evaluation and RAII
 */
template<typename RowType>
class TableIterator {
private:
    RowIter handle_;
    std::vector<uint8_t> buffer_;
    std::optional<RowType> current_row_;
    bool is_end_;
    bool is_valid_;
    
    void advance_internal() {
        if (is_end_) return;
        
        uint32_t buffer_len = buffer_.size();
        int32_t result = row_iter_bsatn_advance(handle_, buffer_.data(), &buffer_len);
        
        if (result != 0) {
            is_end_ = true;
            current_row_ = std::nullopt;
            return;
        }
        
        // If buffer was too small, resize and try again
        if (buffer_len > buffer_.size()) {
            buffer_.resize(buffer_len);
            buffer_len = buffer_.size();
            result = row_iter_bsatn_advance(handle_, buffer_.data(), &buffer_len);
            if (result != 0) {
                is_end_ = true;
                current_row_ = std::nullopt;
                return;
            }
        }
        
        // Deserialize the row
        try {
            SpacetimeDb::bsatn::Reader reader(buffer_.data(), buffer_len);
            current_row_ = SpacetimeDb::bsatn::deserialize<RowType>(reader);
        } catch (const std::exception&) {
            is_end_ = true;
            current_row_ = std::nullopt;
        }
    }
    
public:
    explicit TableIterator(RowIter handle) 
        : handle_(handle), buffer_(1024), is_end_(false), is_valid_(true) {
        advance_internal();
    }
    
    // End iterator constructor
    TableIterator() : handle_(0), is_end_(true), is_valid_(false) {}
    
    // RAII cleanup
    ~TableIterator() {
        if (is_valid_) {
            row_iter_bsatn_close(handle_);
        }
    }
    
    // Move constructor
    TableIterator(TableIterator&& other) noexcept
        : handle_(other.handle_), buffer_(std::move(other.buffer_)),
          current_row_(std::move(other.current_row_)), 
          is_end_(other.is_end_), is_valid_(other.is_valid_) {
        other.is_valid_ = false; // Prevent double cleanup
    }
    
    // Move assignment
    TableIterator& operator=(TableIterator&& other) noexcept {
        if (this != &other) {
            if (is_valid_) {
                row_iter_bsatn_close(handle_);
            }
            handle_ = other.handle_;
            buffer_ = std::move(other.buffer_);
            current_row_ = std::move(other.current_row_);
            is_end_ = other.is_end_;
            is_valid_ = other.is_valid_;
            other.is_valid_ = false;
        }
        return *this;
    }
    
    // Iterator interface
    TableIterator& operator++() {
        advance_internal();
        return *this;
    }
    
    const RowType& operator*() const {
        if (!current_row_) {
            throw std::runtime_error("Iterator dereferenced at end or invalid state");
        }
        return *current_row_;
    }
    
    const RowType* operator->() const {
        if (!current_row_) {
            return nullptr;
        }
        return &(*current_row_);
    }
    
    bool operator!=(const TableIterator& other) const {
        return is_end_ != other.is_end_;
    }
    
    bool operator==(const TableIterator& other) const {
        return is_end_ == other.is_end_;
    }
    
    bool at_end() const { return is_end_; }
};

/**
 * Query result container with lazy evaluation
 */
template<typename RowType>
class QueryResult {
private:
    RowIter row_iter_;
    
public:
    explicit QueryResult(RowIter iter) : row_iter_(iter) {}
    
    TableIterator<RowType> begin() {
        return TableIterator<RowType>(row_iter_);
    }
    
    TableIterator<RowType> end() {
        return TableIterator<RowType>();
    }
    
    // Convert to vector (materializes all results)
    std::vector<RowType> to_vector() {
        std::vector<RowType> results;
        for (auto&& row : *this) {
            results.push_back(std::move(row));
        }
        return results;
    }
    
    // Count results without materializing them
    uint64_t count() {
        uint64_t count = 0;
        for (auto it = begin(); it != end(); ++it) {
            ++count;
        }
        return count;
    }
    
    // Find first result (short-circuits iteration)
    std::optional<RowType> first() {
        auto it = begin();
        if (it != end()) {
            return *it;
        }
        return std::nullopt;
    }
    
    // Check if any results exist
    bool any() {
        auto it = begin();
        return it != end();
    }
};

/**
 * Enhanced table operations with advanced query capabilities
 */
template<typename RowType>
class AdvancedTableHandle {
private:
    std::string table_name_;
    mutable std::optional<TableId> cached_table_id_;
    
    TableId get_table_id() const {
        if (!cached_table_id_) {
            // TODO: Implement table_id_from_name FFI call
            throw std::runtime_error("Table ID resolution not implemented");
        }
        return *cached_table_id_;
    }
    
public:
    explicit AdvancedTableHandle(const std::string& name) : table_name_(name) {}
    
    // Basic operations
    void insert(const RowType& row) {
        // TODO: Implement via existing TableHandle
    }
    
    // Row counting operations
    uint64_t count() const {
        uint64_t row_count;
        int32_t result = datastore_table_row_count(get_table_id(), &row_count);
        if (result != 0) {
            throw std::runtime_error("Failed to get row count");
        }
        return row_count;
    }
    
    // Full table scan with lazy iteration
    QueryResult<RowType> scan() const {
        RowIter iter;
        int32_t result = datastore_table_scan_bsatn(get_table_id(), &iter);
        if (result != 0) {
            throw std::runtime_error("Failed to start table scan");
        }
        return QueryResult<RowType>(iter);
    }
    
    // Iterate all rows
    TableIterator<RowType> begin() const {
        return scan().begin();
    }
    
    TableIterator<RowType> end() const {
        return TableIterator<RowType>();
    }
    
    // Filter operations using predicates
    template<typename Predicate>
    std::vector<RowType> filter(Predicate pred) const {
        std::vector<RowType> results;
        for (const auto& row : scan()) {
            if (pred(row)) {
                results.push_back(row);
            }
        }
        return results;
    }
    
    // Update operations with predicate
    template<typename UpdateFunc>
    uint32_t update_where(UpdateFunc update_func) {
        uint32_t updated_count = 0;
        std::vector<RowType> rows_to_update;
        
        // First pass: identify rows to update
        for (const auto& row : scan()) {
            auto updated_row = update_func(row);
            if (updated_row) {
                rows_to_update.push_back(*updated_row);
                ++updated_count;
            }
        }
        
        // Second pass: perform updates
        // TODO: Implement batch update operations
        
        return updated_count;
    }
    
    // Delete operations with predicate
    template<typename Predicate>
    uint32_t delete_where(Predicate pred) {
        uint32_t deleted_count = 0;
        
        // TODO: Implement efficient predicate-based deletion
        // For now, this would require:
        // 1. Scan table to find matching rows
        // 2. Collect row IDs or unique keys
        // 3. Delete via index operations
        
        return deleted_count;
    }
    
    // Find first matching row
    template<typename Predicate>
    std::optional<RowType> find_first(Predicate pred) const {
        for (const auto& row : scan()) {
            if (pred(row)) {
                return row;
            }
        }
        return std::nullopt;
    }
    
    // Check if any row matches predicate
    template<typename Predicate>
    bool exists(Predicate pred) const {
        for (const auto& row : scan()) {
            if (pred(row)) {
                return true;
            }
        }
        return false;
    }
};

/**
 * Compound query builder for complex operations
 */
template<typename RowType>
class QueryBuilder {
private:
    AdvancedTableHandle<RowType>& table_;
    
public:
    explicit QueryBuilder(AdvancedTableHandle<RowType>& table) : table_(table) {}
    
    // Fluent interface for building complex queries
    template<typename Predicate>
    QueryBuilder& where(Predicate pred) {
        // TODO: Build compound predicate
        return *this;
    }
    
    template<typename KeyFunc>
    QueryBuilder& order_by(KeyFunc key_func) {
        // TODO: Add sorting capability
        return *this;
    }
    
    QueryBuilder& limit(uint64_t max_rows) {
        // TODO: Add result limiting
        return *this;
    }
    
    QueryBuilder& offset(uint64_t skip_rows) {
        // TODO: Add result offset
        return *this;
    }
    
    // Execute the query
    std::vector<RowType> execute() {
        // TODO: Execute compound query with optimizations
        return {};
    }
};

/**
 * Utility functions for common query patterns
 */
namespace query_utils {
    // Helper for creating common predicates
    template<typename T, typename FieldType>
    auto equals(FieldType T::*field, const FieldType& value) {
        return [field, value](const T& row) {
            return row.*field == value;
        };
    }
    
    template<typename T, typename FieldType>
    auto greater_than(FieldType T::*field, const FieldType& value) {
        return [field, value](const T& row) {
            return row.*field > value;
        };
    }
    
    template<typename T, typename FieldType>
    auto between(FieldType T::*field, const FieldType& min, const FieldType& max) {
        return [field, min, max](const T& row) {
            const auto& val = row.*field;
            return val >= min && val <= max;
        };
    }
}

} // namespace spacetimedb

#endif // SPACETIMEDB_LIBRARY_QUERY_OPERATIONS_H