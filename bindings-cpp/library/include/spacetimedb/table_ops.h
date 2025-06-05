#ifndef SPACETIMEDB_TABLE_OPS_H
#define SPACETIMEDB_TABLE_OPS_H

#include "spacetimedb/bsatn/bsatn.h"
#include "spacetimedb/internal/FFI.h"
#include <vector>
#include <stdexcept>
#include <optional>

namespace SpacetimeDb {

// Forward declarations
template<typename T> class TableIterator;

// Rust-like table operations with direct FFI calls
// This provides the core table functionality without virtual overhead
template<typename T>
class TableOps {
private:
    uint32_t table_id_;
    std::string table_name_;
    
public:
    using Row = T;
    
    TableOps(uint32_t table_id, const std::string& name) 
        : table_id_(table_id), table_name_(name) {}
    
    // Count rows - direct FFI call like Rust
    uint64_t count() const {
        uint64_t result = 0;
        auto err = Internal::FFI::datastore_table_row_count(table_id_, &result);
        if (err != Internal::FFI::Errno::OK) {
            throw std::runtime_error("Failed to get row count for table " + table_name_);
        }
        return result;
    }
    
    // Iterate over all rows
    TableIterator<T> iter() const {
        return TableIterator<T>(table_id_);
    }
    
    // Insert with auto-increment support
    T insert(const T& row) {
        bsatn::Writer writer;
        row.bsatn_serialize(writer);
        auto bytes = writer.take_buffer();
        uint32_t bytes_len = static_cast<uint32_t>(bytes.size());
        
        auto err = Internal::FFI::datastore_insert_bsatn(table_id_, bytes.data(), &bytes_len);
        if (err != Internal::FFI::Errno::OK) {
            throw std::runtime_error("Failed to insert into table " + table_name_);
        }
        
        // Read back the row with any auto-generated fields
        bsatn::Reader reader(bytes.data(), bytes_len);
        T result;
        result.bsatn_deserialize(reader);
        return result;
    }
    
    // Try insert with error handling
    struct InsertError {
        enum Type { UniqueViolation, AutoIncOverflow } type;
        std::string message;
    };
    
    std::optional<InsertError> try_insert(T& row) {
        try {
            row = insert(row);
            return std::nullopt;
        } catch (const std::exception& e) {
            // Parse error to determine type
            return InsertError{InsertError::UniqueViolation, e.what()};
        }
    }
    
    // Delete - returns true if row was deleted
    bool delete_row(const T& row) {
        bsatn::Writer writer;
        writer.write_u32_le(1); // Array with one element
        row.bsatn_serialize(writer);
        
        auto bytes = writer.take_buffer();
        uint32_t count = 0;
        
        auto err = Internal::FFI::datastore_delete_all_by_eq_bsatn(
            table_id_,
            bytes.data(),
            static_cast<uint32_t>(bytes.size()),
            &count
        );
        
        if (err != Internal::FFI::Errno::OK) {
            throw std::runtime_error("Failed to delete from table " + table_name_);
        }
        
        return count > 0;
    }
};

// Iterator for table rows - similar to Rust's TableIter
// Implementation moved to avoid duplicate definition with library/table.h
template<typename T>
class TableIteratorOps {
private:
    Internal::FFI::RowIter handle_ = 0xFFFFFFFF;
    std::vector<uint8_t> buffer_;
    std::vector<T> current_batch_;
    size_t current_index_ = 0;
    bool done_ = false;
    
    void FetchNextBatch() {
        if (done_) return;
        
        buffer_.resize(0x20000); // 128KB buffer
        uint32_t buf_len = static_cast<uint32_t>(buffer_.size());
        
        auto ret = Internal::FFI::row_iter_bsatn_advance(handle_, buffer_.data(), &buf_len);
        
        if (ret == Internal::FFI::Errno::EXHAUSTED) {
            done_ = true;
            return;
        }
        
        if (ret != Internal::FFI::Errno::OK) {
            throw std::runtime_error("Iterator error");
        }
        
        // Parse the batch
        buffer_.resize(buf_len);
        bsatn::Reader reader(buffer_);
        current_batch_.clear();
        
        while (!reader.is_eos()) {
            T row;
            row.bsatn_deserialize(reader);
            current_batch_.push_back(std::move(row));
        }
        
        current_index_ = 0;
    }
    
public:
    explicit TableIteratorOps(uint32_t table_id) {
        Internal::FFI::datastore_table_scan_bsatn(table_id, &handle_);
        FetchNextBatch();
    }
    
    ~TableIteratorOps() {
        if (handle_ != 0xFFFFFFFF) {
            Internal::FFI::row_iter_bsatn_close(handle_);
        }
    }
    
    // Iterator interface
    bool has_next() const {
        return !done_ || current_index_ < current_batch_.size();
    }
    
    T next() {
        if (current_index_ >= current_batch_.size()) {
            FetchNextBatch();
        }
        
        if (done_ && current_index_ >= current_batch_.size()) {
            throw std::runtime_error("Iterator exhausted");
        }
        
        return current_batch_[current_index_++];
    }
    
    // Range-for support
    class iterator {
        TableIteratorOps* parent_;
        std::optional<T> current_;
        
    public:
        explicit iterator(TableIteratorOps* parent) : parent_(parent) {
            if (parent && parent->has_next()) {
                current_ = parent->next();
            }
        }
        
        iterator& operator++() {
            if (parent_ && parent_->has_next()) {
                current_ = parent_->next();
            } else {
                current_.reset();
            }
            return *this;
        }
        
        bool operator!=(const iterator& other) const {
            return current_.has_value() != other.current_.has_value();
        }
        
        const T& operator*() const { return *current_; }
    };
    
    iterator begin() { return iterator(this); }
    iterator end() { return iterator(nullptr); }
};

} // namespace SpacetimeDb

#endif // SPACETIMEDB_TABLE_OPS_H