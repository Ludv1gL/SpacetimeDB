#include "spacetimedb/index_management.h"
#include "spacetimedb/logger.h"
#include "spacetimedb/bsatn/bsatn.h"
#include <stdexcept>

namespace SpacetimeDb {

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

// Template specializations for specific types can be added here if needed

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

} // namespace SpacetimeDb