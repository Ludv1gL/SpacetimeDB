// THIS FILE IS TEMPORARILY CREATED FOR COMPILATION
// TODO: Generate proper AlgebraicType from Rust definitions

#pragma once

#include <cstdint>
#include <vector>
#include "spacetimedb/bsatn/bsatn.h"

namespace SpacetimeDb::Internal {

// Placeholder for AlgebraicType
// In the actual implementation, this would be a complex sum type
struct AlgebraicType {
    std::vector<uint8_t> data;  // Raw BSATN representation
    
    AlgebraicType() = default;
    
    explicit AlgebraicType(std::vector<uint8_t> data) : data(std::move(data)) {}
    
    void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
        writer.write_bytes(data);
    }
    
    void bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
        data = reader.read_bytes();
    }
    
    static AlgebraicType from_bsatn(SpacetimeDb::bsatn::Reader& reader) {
        AlgebraicType result;
        result.bsatn_deserialize(reader);
        return result;
    }
};

} // namespace SpacetimeDb::Internal