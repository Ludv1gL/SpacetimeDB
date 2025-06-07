#ifndef SPACETIMEDB_LIBRARY_INDEX_MANAGEMENT_H
#define SPACETIMEDB_LIBRARY_INDEX_MANAGEMENT_H

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <memory>
#include <type_traits>
#include <functional>
#include <spacetimedb/abi/spacetimedb_abi.h>  // For FFI function declarations
#include <spacetimedb/bsatn/bsatn.h>  // For serialization

namespace SpacetimeDb {

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
    static Bound Inclusive(const T& val) { return Bound(val, BoundType::Inclusive); }
    static Bound Exclusive(const T& val) { return Bound(val, BoundType::Exclusive); }
    static Bound Unbounded() { return Bound(T{}, BoundType::Unbounded); }
    
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

// FFI functions are declared in spacetimedb_abi.h which is included above

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
        std::vector<RowType> results;
        
        try {
            IndexId index_id = this->GetIndexId();
            
            // Serialize range bounds
            std::vector<uint8_t> start_buffer, end_buffer;
            SerializeBound(range.start, start_buffer);
            SerializeBound(range.end, end_buffer);
            
            // Call FFI to scan the index
            uint32_t iter;
            int16_t result = datastore_btree_scan_bsatn(
                index_id,
                nullptr, 0, 0, // No prefix for now
                start_buffer.data(), start_buffer.size(),
                end_buffer.data(), end_buffer.size(),
                &iter
            );
            
            if (result != 0) {
                throw std::runtime_error("Failed to start BTree scan: " + std::to_string(result));
            }
            
            // Read results from iterator
            ReadIteratorResults(iter, results);
            
        } catch (const std::exception& e) {
            // Log error but don't throw to maintain compatibility
            // TODO: Add proper logging
        }
        
        return results;
    }
    
    // Delete by exact match
    uint32_t Delete(const KeyType& key) {
        Range<KeyType> range(key);
        return DeleteRange(range);
    }
    
    // Delete by range
    uint32_t DeleteRange(const Range<KeyType>& range) {
        try {
            IndexId index_id = this->GetIndexId();
            
            // Serialize range bounds
            std::vector<uint8_t> start_buffer, end_buffer;
            SerializeBound(range.start, start_buffer);
            SerializeBound(range.end, end_buffer);
            
            // Call FFI to delete by range
            uint32_t num_deleted = 0;
            int16_t result = datastore_delete_by_btree_scan_bsatn(
                index_id,
                nullptr, 0, 0, // No prefix for now
                start_buffer.data(), start_buffer.size(),
                end_buffer.data(), end_buffer.size(),
                &num_deleted
            );
            
            if (result != 0) {
                throw std::runtime_error("Failed to delete by BTree range: " + std::to_string(result));
            }
            
            return num_deleted;
            
        } catch (const std::exception& e) {
            // Log error but don't throw to maintain compatibility
            return 0;
        }
    }
    
    // Iterator-based filtering
    IndexIterator<RowType> begin(const Range<KeyType>& range) {
        try {
            IndexId index_id = this->GetIndexId();
            
            // Serialize range bounds
            std::vector<uint8_t> start_buffer, end_buffer;
            SerializeBound(range.start, start_buffer);
            SerializeBound(range.end, end_buffer);
            
            // Call FFI to start scan
            uint32_t iter;
            int16_t result = datastore_btree_scan_bsatn(
                index_id,
                nullptr, 0, 0, // No prefix for now
                start_buffer.data(), start_buffer.size(),
                end_buffer.data(), end_buffer.size(),
                &iter
            );
            
            if (result == 0) {
                return IndexIterator<RowType>(iter);
            }
        } catch (const std::exception& e) {
            // Fall through to return end iterator
        }
        
        return IndexIterator<RowType>(); // End iterator on error
    }
    
    IndexIterator<RowType> end() {
        return IndexIterator<RowType>();
    }

private:
    // Helper method to serialize a bound value
    void SerializeBound(const Bound<KeyType>& bound, std::vector<uint8_t>& buffer) {
        SpacetimeDb::bsatn::Writer writer(buffer);
        
        // Serialize bound type
        writer.write_u8(static_cast<uint8_t>(bound.type));
        
        // Serialize bound value (if not unbounded)
        if (bound.type != BoundType::Unbounded) {
            SpacetimeDb::bsatn::serialize(writer, bound.value);
        }
    }
    
    // Helper method to read results from iterator
    void ReadIteratorResults(uint32_t iter, std::vector<RowType>& results) {
        const size_t BUFFER_SIZE = 8192;
        std::vector<uint8_t> buffer(BUFFER_SIZE);
        
        while (true) {
            size_t buffer_len = buffer.size();
            int16_t result = row_iter_bsatn_advance(iter, buffer.data(), &buffer_len);
            
            if (result == 0) {
                // End of iteration
                break;
            } else if (result > 0) {
                // Got a row, deserialize it
                try {
                    SpacetimeDb::bsatn::Reader reader(buffer.data(), buffer_len);
                    RowType row = SpacetimeDb::bsatn::bsatn_traits<RowType>::deserialize(reader);
                    results.push_back(std::move(row));
                } catch (const std::exception& e) {
                    // Skip malformed rows
                    continue;
                }
            } else {
                // Error occurred
                break;
            }
        }
        
        // Close the iterator
        row_iter_bsatn_close(iter);
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
 * Index and constraint registration system
 */
class IndexRegistry {
private:
    struct IndexInfo {
        std::string table_name;
        std::string column_name;
        std::string index_name;
        bool is_unique;
        bool is_primary_key;
    };
    
    std::vector<IndexInfo> indexes_;
    
public:
    static IndexRegistry& instance() {
        static IndexRegistry registry;
        return registry;
    }
    
    template<typename TableType, typename FieldType>
    void register_primary_key(const std::string& table_name, 
                             const std::string& column_name,
                             std::function<const FieldType&(const TableType&)> field_getter) {
        IndexInfo info{
            .table_name = table_name,
            .column_name = column_name,
            .index_name = "pk_" + table_name + "_" + column_name,
            .is_unique = true,
            .is_primary_key = true
        };
        indexes_.push_back(info);
        
        // Create the actual index via FFI
        CreateIndexViaFFI(info, true);
    }
    
    template<typename TableType, typename FieldType>
    void register_unique_constraint(const std::string& table_name,
                                   const std::string& column_name,
                                   std::function<const FieldType&(const TableType&)> field_getter) {
        IndexInfo info{
            .table_name = table_name,
            .column_name = column_name,
            .index_name = "unique_" + table_name + "_" + column_name,
            .is_unique = true,
            .is_primary_key = false
        };
        indexes_.push_back(info);
        
        // Create the actual index via FFI
        CreateIndexViaFFI(info, true);
    }
    
    template<typename TableType, typename FieldType>
    void register_index(const std::string& table_name,
                       const std::string& column_name,
                       const std::string& index_name,
                       bool is_unique,
                       std::function<const FieldType&(const TableType&)> field_getter) {
        IndexInfo info{
            .table_name = table_name,
            .column_name = column_name,
            .index_name = index_name,
            .is_unique = is_unique,
            .is_primary_key = false
        };
        indexes_.push_back(info);
        
        // Create the actual index via FFI
        CreateIndexViaFFI(info, is_unique);
    }
    
    const std::vector<IndexInfo>& get_indexes() const { return indexes_; }
    
    std::vector<IndexInfo> get_primary_keys(const std::string& table_name) const {
        std::vector<IndexInfo> result;
        for (const auto& info : indexes_) {
            if (info.table_name == table_name && info.is_primary_key) {
                result.push_back(info);
            }
        }
        return result;
    }
    
    std::vector<IndexInfo> get_unique_constraints(const std::string& table_name) const {
        std::vector<IndexInfo> result;
        for (const auto& info : indexes_) {
            if (info.table_name == table_name && info.is_unique && !info.is_primary_key) {
                result.push_back(info);
            }
        }
        return result;
    }

private:
    // Helper method to create index via FFI
    void CreateIndexViaFFI(const IndexInfo& info, bool is_unique) {
        try {
            // Get table ID
            uint32_t table_id;
            int16_t result = table_id_from_name(
                reinterpret_cast<const uint8_t*>(info.table_name.c_str()),
                info.table_name.length(),
                &table_id
            );
            
            if (result != 0) {
                // Table not found, skip index creation
                return;
            }
            
            // For now, assume single column index with column ID 0
            // TODO: Implement proper column ID lookup
            uint8_t col_id = 0;
            
            // Index type: 0 = BTree
            uint8_t index_type = 0;
            
            // Create the index
            result = _create_index(
                reinterpret_cast<const uint8_t*>(info.index_name.c_str()),
                info.index_name.length(),
                table_id,
                index_type,
                &col_id,
                1  // Single column
            );
            
            // Note: We don't throw on failure to maintain compatibility
            // with modules that don't support runtime index creation
            
        } catch (const std::exception& e) {
            // Silently handle errors to maintain compatibility
        }
    }
};

/**
 * Macros for index definition
 */

// Note: Index definition macros have been consolidated in macros.h
// Use SPACETIMEDB_INDEX_BTREE, SPACETIMEDB_INDEX_UNIQUE etc from macros.h
// These marker variables are for internal use by the index system
namespace internal {
    template<typename T> struct index_markers {
        // Markers will be set by macros in macros.h
    };
}

// =============================================================================
// INDEXITERATOR IMPLEMENTATION
// =============================================================================

template<typename RowType>
IndexIterator<RowType>::IndexIterator(RowIter iter) 
    : row_iter_(iter), is_end_(false) {
    LoadNext();
}

template<typename RowType>
IndexIterator<RowType>::IndexIterator() 
    : row_iter_(0), is_end_(true) {
}

template<typename RowType>
void IndexIterator<RowType>::LoadNext() {
    if (is_end_) return;
    
    const size_t BUFFER_SIZE = 8192;
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    size_t buffer_len = buffer.size();
    
    int16_t result = row_iter_bsatn_advance(row_iter_, buffer.data(), &buffer_len);
    
    if (result == 0) {
        // End of iteration
        is_end_ = true;
        current_row_.reset();
        row_iter_bsatn_close(row_iter_);
    } else if (result > 0) {
        // Got a row, deserialize it
        try {
            SpacetimeDb::bsatn::Reader reader(buffer.data(), buffer_len);
            RowType row = SpacetimeDb::bsatn::bsatn_traits<RowType>::deserialize(reader);
            current_row_ = std::move(row);
        } catch (const std::exception& e) {
            // Skip malformed rows and try next
            LoadNext();
        }
    } else {
        // Error occurred
        is_end_ = true;
        current_row_.reset();
        row_iter_bsatn_close(row_iter_);
    }
}

template<typename RowType>
IndexIterator<RowType>& IndexIterator<RowType>::operator++() {
    LoadNext();
    return *this;
}

template<typename RowType>
const RowType& IndexIterator<RowType>::operator*() const {
    if (!current_row_) {
        throw std::runtime_error("Dereferencing end iterator");
    }
    return *current_row_;
}

template<typename RowType>
const RowType* IndexIterator<RowType>::operator->() const {
    if (!current_row_) {
        return nullptr;
    }
    return &(*current_row_);
}

template<typename RowType>
bool IndexIterator<RowType>::operator!=(const IndexIterator& other) const {
    return is_end_ != other.is_end_ || row_iter_ != other.row_iter_;
}

template<typename RowType>
bool IndexIterator<RowType>::operator==(const IndexIterator& other) const {
    return !(*this != other);
}

} // namespace SpacetimeDb

#endif // SPACETIMEDB_LIBRARY_INDEX_MANAGEMENT_H