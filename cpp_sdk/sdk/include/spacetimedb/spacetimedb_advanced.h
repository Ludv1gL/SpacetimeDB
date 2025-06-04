#ifndef SPACETIMEDB_ADVANCED_H
#define SPACETIMEDB_ADVANCED_H

/**
 * SpacetimeDB C++ SDK - Advanced Features
 * 
 * This header extends the basic SDK with advanced features:
 * - Advanced query operations
 * - Index management
 * - Schema management
 * 
 * Include this header AFTER spacetimedb.h for advanced functionality.
 */

#ifndef SPACETIMEDB_H
#error "spacetimedb_advanced.h must be included after spacetimedb.h"
#endif

// Include advanced features
#include "sdk/query_operations.h"   // Advanced query operations
#include "sdk/index_management.h"   // Index management  
#include "sdk/schema_management.h"  // Schema management

namespace spacetimedb {

// -----------------------------------------------------------------------------
// Enhanced TableHandle Integration
// -----------------------------------------------------------------------------

/**
 * EnhancedTableHandle - Bridges basic TableHandle with advanced features
 * 
 * This class wraps the basic TableHandle and adds advanced query, index,
 * and schema capabilities from Features 4-6.
 */
template<typename T>
class EnhancedTableHandle : public TableHandle<T> {
public:
    using TableHandle<T>::TableHandle;
    
    // Advanced Query Operations (from query_operations.h)
    QueryResult<T> filter(std::function<bool(const T&)> predicate) {
        AdvancedTableHandle<T> advanced(this->table_name_);
        return advanced.filter(predicate);
    }
    
    size_t update_where(std::function<bool(const T&)> predicate, 
                       std::function<void(T&)> updater) {
        AdvancedTableHandle<T> advanced(this->table_name_);
        return advanced.update_where(predicate, updater);
    }
    
    size_t delete_where(std::function<bool(const T&)> predicate) {
        AdvancedTableHandle<T> advanced(this->table_name_);
        return advanced.delete_where(predicate);
    }
    
    std::optional<T> find_first(std::function<bool(const T&)> predicate) {
        AdvancedTableHandle<T> advanced(this->table_name_);
        return advanced.find_first(predicate);
    }
    
    bool exists(std::function<bool(const T&)> predicate) {
        AdvancedTableHandle<T> advanced(this->table_name_);
        return advanced.exists(predicate);
    }
    
    // Index Operations (from index_management.h)
    template<typename... KeyTypes>
    BTreeIndex<T, KeyTypes...> btree_index(const std::string& name) {
        return BTreeIndex<T, KeyTypes...>(this->table_name_, name);
    }
    
    template<typename... KeyTypes>
    UniqueIndex<T, KeyTypes...> unique_index(const std::string& name) {
        return UniqueIndex<T, KeyTypes...>(this->table_name_, name);
    }
    
    // Query Builder
    QueryBuilder<T> query() {
        return QueryBuilder<T>(this->table_name_);
    }
};

// -----------------------------------------------------------------------------
// Schema Builder Integration
// -----------------------------------------------------------------------------

/**
 * Enhanced table registration with schema management
 */
template<typename T>
class TableRegistration {
    SchemaBuilder builder_;
    
public:
    TableRegistration(const std::string& name) {
        builder_.table<T>(name);
    }
    
    TableRegistration& primary_key(const std::string& column) {
        builder_.column(column, ColumnAttrs::PrimaryKey);
        return *this;
    }
    
    TableRegistration& unique(const std::string& column) {
        builder_.column(column, ColumnAttrs::Unique);
        return *this;
    }
    
    TableRegistration& auto_increment(const std::string& column) {
        builder_.column(column, ColumnAttrs::AutoInc);
        return *this;
    }
    
    TableRegistration& indexed(const std::string& column) {
        builder_.column(column, ColumnAttrs::Indexed);
        return *this;
    }
    
    void register_table() {
        // Integration with module registration would go here
        // For now, this serves as a schema definition helper
    }
};

// -----------------------------------------------------------------------------
// Convenience Functions
// -----------------------------------------------------------------------------

template<typename T>
EnhancedTableHandle<T> enhanced_table(const std::string& name) {
    return EnhancedTableHandle<T>(name);
}

template<typename T>
TableRegistration<T> define_table(const std::string& name) {
    return TableRegistration<T>(name);
}

} // namespace spacetimedb

#endif // SPACETIMEDB_ADVANCED_H