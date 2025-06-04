#ifndef SPACETIMEDB_INTERNAL_ITABLE_H
#define SPACETIMEDB_INTERNAL_ITABLE_H

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "spacetimedb/bsatn/bsatn.h"
#include "spacetimedb/internal/autogen/RawTableDefV9.g.h"
#include "spacetimedb/internal/autogen/RawScheduleDefV9.g.h"
#include "spacetimedb/internal/autogen/RawSequenceDefV9.g.h"
#include "spacetimedb/internal/autogen/RawConstraintDefV9.g.h"
#include "spacetimedb/internal/FFI.h"

namespace SpacetimeDb {
namespace Internal {

// Forward declarations
class ITypeRegistrar;

// Base class for table iterators
template<typename T>
class RawTableIterBase {
public:
    class Iterator {
    private:
        FFI::RowIter handle;
        std::vector<uint8_t> buffer;
        std::vector<uint8_t> current;
        
    public:
        explicit Iterator(FFI::RowIter handle) 
            : handle(handle), buffer(0x20000) {}
        
        ~Iterator() {
            if (handle != 0xFFFFFFFF) {
                FFI::row_iter_bsatn_close(handle);
            }
        }
        
        // Iterator traits
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::vector<uint8_t>;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::vector<uint8_t>*;
        using reference = const std::vector<uint8_t>&;
        
        Iterator& operator++() {
            MoveNext();
            return *this;
        }
        
        bool operator!=(const Iterator& other) const {
            return handle != other.handle;
        }
        
        const std::vector<uint8_t>& operator*() const {
            return current;
        }
        
    private:
        bool MoveNext();
    };
    
protected:
    virtual void IterStart(FFI::RowIter& handle) = 0;
    
public:
    Iterator begin() {
        FFI::RowIter handle;
        IterStart(handle);
        Iterator it(handle);
        ++it; // Move to first element
        return it;
    }
    
    Iterator end() {
        return Iterator(0xFFFFFFFF);
    }
    
    // Parse the raw bytes into T objects
    std::vector<T> Parse();
};

// Table view interface
template<typename View, typename T>
class ITableView {
protected:
    // Table metadata
    static std::string GetTableName() {
        // Extract table name from View type name
        // This is a simple implementation - could be improved
        return typeid(View).name();
    }
    
    static FFI::TableId GetTableId() {
        static FFI::TableId tableId = []() {
            auto name = GetTableName();
            FFI::TableId id;
            FFI::table_id_from_name(
                reinterpret_cast<const uint8_t*>(name.data()), 
                static_cast<uint32_t>(name.size()), 
                &id
            );
            return id;
        }();
        return tableId;
    }
    
public:
    // Required static methods for codegen
    static RawTableDefV9 MakeTableDesc(ITypeRegistrar& registrar);
    static T ReadGenFields(bsatn::Reader& reader, T row);
    
    // Table operations
    virtual uint64_t Count() const = 0;
    virtual std::vector<T> Iter() const = 0;
    virtual T Insert(const T& row) = 0;
    virtual bool Delete(const T& row) = 0;
    
protected:
    // Helper implementations
    static uint64_t DoCount() {
        uint64_t count;
        FFI::datastore_table_row_count(GetTableId(), &count);
        return count;
    }
    
    static std::vector<T> DoIter();
    static T DoInsert(const T& row);
    static bool DoDelete(const T& row);
    
    // Helper to create schedule definition
    static RawScheduleDefV9 MakeSchedule(const std::string& reducerName, uint16_t colIndex) {
        return RawScheduleDefV9{
            GetTableName() + "_sched",  // name
            reducerName,                // reducer_name
            colIndex                    // scheduled_at_column
        };
    }
    
    // Helper to create sequence definition
    static RawSequenceDefV9 MakeSequence(uint16_t colIndex) {
        return RawSequenceDefV9{
            std::nullopt,   // name
            colIndex,       // column
            std::nullopt,   // start
            std::nullopt,   // min_value
            std::nullopt,   // max_value
            1               // increment
        };
    }
    
    // Helper to create unique constraint
    static RawConstraintDefV9 MakeUniqueConstraint(uint16_t colIndex);
};

// Raw table iterator implementation
template<typename T>
class RawTableIter : public RawTableIterBase<T> {
private:
    FFI::TableId tableId;
    
public:
    explicit RawTableIter(FFI::TableId tableId) : tableId(tableId) {}
    
protected:
    void IterStart(FFI::RowIter& handle) override {
        FFI::datastore_table_scan_bsatn(tableId, &handle);
    }
};

} // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_ITABLE_H