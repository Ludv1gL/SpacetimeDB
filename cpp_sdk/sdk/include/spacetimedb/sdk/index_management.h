#ifndef SPACETIMEDB_SDK_INDEX_MANAGEMENT_H
#define SPACETIMEDB_SDK_INDEX_MANAGEMENT_H

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <memory>
#include <type_traits>

namespace spacetimedb {

// Forward declarations
using IndexId = uint32_t;
using ColId = uint32_t;
using RowIter = uint32_t;
using TableId = uint32_t;

/**
 * Index algorithm types supported by SpacetimeDB
 */
enum class IndexAlgorithmTag : uint8_t {
    BTree = 0,
    Hash = 1    // Future expansion
};

/**
 * Represents a B-tree index algorithm with column specifications
 */
struct BTreeAlgorithm {
    std::vector<ColId> columns;  // Column indices for the index
    
    explicit BTreeAlgorithm(std::vector<ColId> cols) : columns(std::move(cols)) {}
    
    // Helper for single column
    explicit BTreeAlgorithm(ColId col) : columns({col}) {}
};

/**
 * Range bound types for index queries
 */
enum class BoundType : uint8_t {
    Inclusive = 0,
    Exclusive = 1,
    Unbounded = 2
};

/**
 * Represents a bound value for range queries
 */
template<typename T>
struct Bound {
    T value;
    BoundType type;
    
    // Factory methods for ergonomics
    static Bound Inclusive(const T& val) { return {val, BoundType::Inclusive}; }
    static Bound Exclusive(const T& val) { return {val, BoundType::Exclusive}; }
    static Bound Unbounded() { return {T{}, BoundType::Unbounded}; }
    
    explicit Bound(const T& val, BoundType t = BoundType::Inclusive) : value(val), type(t) {}
};

/**
 * Range specification for index queries
 */
template<typename T>
struct Range {
    Bound<T> start;
    Bound<T> end;
    
    // Exact match (single value)
    explicit Range(const T& value) 
        : start(Bound<T>::Inclusive(value)), end(Bound<T>::Inclusive(value)) {}
    
    // Range match
    Range(const Bound<T>& start_bound, const Bound<T>& end_bound) 
        : start(start_bound), end(end_bound) {}
};

/**
 * FFI declarations for index operations
 */
extern "C" {
    int32_t index_id_from_name(const uint8_t* name, uint32_t name_len, IndexId* out);
    
    int32_t datastore_index_scan_range_bsatn(
        IndexId index_id, 
        const uint8_t* prefix, uint32_t prefix_len, ColId prefix_elems,
        const uint8_t* rstart, uint32_t rstart_len, 
        const uint8_t* rend, uint32_t rend_len, 
        RowIter* out);
    
    int32_t datastore_delete_by_index_scan_range_bsatn(
        IndexId index_id,
        const uint8_t* prefix, uint32_t prefix_len, ColId prefix_elems,
        const uint8_t* rstart, uint32_t rstart_len,
        const uint8_t* rend, uint32_t rend_len,
        uint32_t* deleted_count);
        
    int32_t datastore_update_bsatn(
        TableId table_id, IndexId index_id,
        uint8_t* row, uint32_t* row_len);
}

/**
 * Base class for all index types
 */
template<typename RowType>
class IndexBase {
protected:
    std::string index_name_;
    mutable std::optional<IndexId> cached_index_id_;
    
    // Lazy index ID resolution
    IndexId GetIndexId() const {
        if (!cached_index_id_) {
            IndexId id;
            int32_t result = index_id_from_name(
                reinterpret_cast<const uint8_t*>(index_name_.c_str()),
                index_name_.length(),
                &id
            );
            if (result == 0) {
                cached_index_id_ = id;
            } else {
                throw std::runtime_error("Failed to resolve index ID for: " + index_name_);
            }
        }
        return *cached_index_id_;
    }
    
public:
    explicit IndexBase(const std::string& name) : index_name_(name) {}
    
    const std::string& GetName() const { return index_name_; }
};

/**
 * Iterator for index query results
 */
template<typename RowType>
class IndexIterator {
private:
    RowIter row_iter_;
    std::optional<RowType> current_row_;
    bool is_end_;
    
    void LoadNext();
    
public:
    explicit IndexIterator(RowIter iter);
    IndexIterator(); // End iterator
    
    IndexIterator& operator++();
    const RowType& operator*() const;
    const RowType* operator->() const;
    bool operator!=(const IndexIterator& other) const;
    bool operator==(const IndexIterator& other) const;
};

/**
 * B-tree index implementation
 */
template<typename RowType, typename KeyType>
class BTreeIndex : public IndexBase<RowType> {
public:
    explicit BTreeIndex(const std::string& name) : IndexBase<RowType>(name) {}
    
    // Exact match filter
    std::vector<RowType> Filter(const KeyType& key) {
        Range<KeyType> range(key);
        return FilterRange(range);
    }
    
    // Range filter
    std::vector<RowType> FilterRange(const Range<KeyType>& range) {
        // Implementation will serialize bounds and call FFI
        std::vector<RowType> results;
        // TODO: Implement BSATN serialization and FFI call
        return results;
    }
    
    // Delete by exact match
    uint32_t Delete(const KeyType& key) {
        Range<KeyType> range(key);
        return DeleteRange(range);
    }
    
    // Delete by range
    uint32_t DeleteRange(const Range<KeyType>& range) {
        // TODO: Implement BSATN serialization and FFI call
        return 0;
    }
    
    // Iterator-based filtering
    IndexIterator<RowType> begin(const Range<KeyType>& range) {
        // TODO: Implement
        return IndexIterator<RowType>();
    }
    
    IndexIterator<RowType> end() {
        return IndexIterator<RowType>();
    }
};

/**
 * Unique index implementation
 */
template<typename RowType, typename KeyType>
class UniqueIndex : public BTreeIndex<RowType, KeyType> {
public:
    explicit UniqueIndex(const std::string& name) : BTreeIndex<RowType, KeyType>(name) {}
    
    // Find single row by unique key
    std::optional<RowType> Find(const KeyType& key) {
        auto results = this->Filter(key);
        if (results.empty()) {
            return std::nullopt;
        }
        return results[0]; // Should only be one result for unique index
    }
    
    // Update row (for unique indexes)
    RowType Update(const RowType& row) {
        // TODO: Implement via FFI
        return row;
    }
    
    // Delete by unique key (returns true if found and deleted)
    bool Delete(const KeyType& key) {
        return this->BTreeIndex<RowType, KeyType>::Delete(key) > 0;
    }
};

/**
 * Index name generation utilities
 */
namespace index_utils {
    std::string GenerateIndexName(const std::string& table_name, 
                                 const std::string& column_name);
    
    std::string GenerateMultiColumnIndexName(const std::string& table_name,
                                           const std::string& index_name);
}

/**
 * Macros for index definition
 */

// Single column B-tree index
#define SPACETIMEDB_INDEX_BTREE(table_type, column_name) \
    static constexpr auto _index_btree_##column_name = true;

// Multi-column B-tree index with custom name
#define SPACETIMEDB_INDEX_BTREE_MULTI(table_type, index_name, ...) \
    static constexpr auto _index_btree_multi_##index_name = true;

// Unique constraint
#define SPACETIMEDB_INDEX_UNIQUE(table_type, column_name) \
    static constexpr auto _index_unique_##column_name = true;

// Primary key (unique + primary constraint)
#define SPACETIMEDB_INDEX_PRIMARY_KEY(table_type, column_name) \
    static constexpr auto _index_primary_##column_name = true;

} // namespace spacetimedb

#endif // SPACETIMEDB_SDK_INDEX_MANAGEMENT_H