#include "spacetimedb/sdk/index_management.h"
#include "spacetimedb/sdk/logging.h"
#include "spacetimedb/bsatn/bsatn.h"
#include <stdexcept>

namespace spacetimedb {

namespace index_utils {
    std::string GenerateIndexName(const std::string& table_name, 
                                 const std::string& column_name) {
        return table_name + "_" + column_name + "_idx_btree";
    }
    
    std::string GenerateMultiColumnIndexName(const std::string& table_name,
                                           const std::string& index_name) {
        return table_name + "_" + index_name + "_idx_btree";
    }
}

// Template specializations will need to be implemented for specific types
// For now, we provide the basic infrastructure

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
    // TODO: Implement row loading from iterator
    // This will require FFI calls to get the next row
    is_end_ = true;  // For now, mark as end
}

template<typename RowType>
IndexIterator<RowType>& IndexIterator<RowType>::operator++() {
    if (!is_end_) {
        LoadNext();
    }
    return *this;
}

template<typename RowType>
const RowType& IndexIterator<RowType>::operator*() const {
    if (is_end_ || !current_row_) {
        throw std::runtime_error("Iterator is at end or invalid");
    }
    return *current_row_;
}

template<typename RowType>
const RowType* IndexIterator<RowType>::operator->() const {
    if (is_end_ || !current_row_) {
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

// Helper function for BSATN serialization of bounds
template<typename T>
std::vector<uint8_t> SerializeBound(const Bound<T>& bound) {
    if (bound.type == BoundType::Unbounded) {
        return {}; // Empty for unbounded
    }
    
    SpacetimeDb::bsatn::Writer writer;
    SpacetimeDb::bsatn::serialize(writer, bound.value);
    return writer.take_buffer();
}

template<typename T>
std::pair<std::vector<uint8_t>, std::vector<uint8_t>> SerializeRange(const Range<T>& range) {
    auto start_bytes = SerializeBound(range.start);
    auto end_bytes = SerializeBound(range.end);
    return {std::move(start_bytes), std::move(end_bytes)};
}

} // namespace spacetimedb