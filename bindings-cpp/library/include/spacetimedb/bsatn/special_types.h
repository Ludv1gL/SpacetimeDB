#ifndef SPACETIMEDB_BSATN_SPECIAL_TYPES_H
#define SPACETIMEDB_BSATN_SPECIAL_TYPES_H

#include "algebraic_type.h"
#include <string_view>

namespace spacetimedb::bsatn {

/**
 * Special type tags used by SpacetimeDB to identify built-in types.
 * These match the Rust implementation exactly.
 */
constexpr const char* IDENTITY_TAG = "__identity__";
constexpr const char* CONNECTION_ID_TAG = "__connection_id__";
constexpr const char* TIMESTAMP_TAG = "__timestamp_micros_since_unix_epoch__";
constexpr const char* TIME_DURATION_TAG = "__time_duration_micros__";

/**
 * Enumeration of special types recognized by SpacetimeDB.
 */
enum class SpecialTypeKind {
    None,
    Identity,
    ConnectionId,
    Timestamp,
    TimeDuration
};

/**
 * Check if a ProductType represents a special SpacetimeDB type.
 * Special types are ProductTypes with exactly one field that has a special tag name.
 */
inline bool is_special_type(const ProductType& product) {
    return product.elements.size() == 1 && 
           product.elements[0].name.has_value() && (
               *product.elements[0].name == IDENTITY_TAG ||
               *product.elements[0].name == CONNECTION_ID_TAG ||
               *product.elements[0].name == TIMESTAMP_TAG ||
               *product.elements[0].name == TIME_DURATION_TAG
           );
}

/**
 * Get the kind of special type represented by a ProductType.
 */
inline SpecialTypeKind get_special_type_kind(const ProductType& product) {
    if (product.elements.size() != 1 || !product.elements[0].name.has_value()) {
        return SpecialTypeKind::None;
    }
    
    const std::string& tag = *product.elements[0].name;
    
    if (tag == IDENTITY_TAG) return SpecialTypeKind::Identity;
    if (tag == CONNECTION_ID_TAG) return SpecialTypeKind::ConnectionId;
    if (tag == TIMESTAMP_TAG) return SpecialTypeKind::Timestamp;
    if (tag == TIME_DURATION_TAG) return SpecialTypeKind::TimeDuration;
    
    return SpecialTypeKind::None;
}

/**
 * Create a special type ProductType with the given tag and data type.
 */
inline std::unique_ptr<ProductType> make_special_type(const char* tag, uint32_t data_type_id) {
    std::vector<ProductTypeElement> elements;
    elements.emplace_back(tag, data_type_id);
    return std::make_unique<ProductType>(std::move(elements));
}

/**
 * Helper functions to create AlgebraicTypes for SpacetimeDB special types.
 */
namespace special_types {
    
    inline AlgebraicType identity() {
        // Identity is a ProductType with a single field tagged "__identity__" containing 32 bytes (array of U8)
        auto array_type = std::make_unique<ArrayType>(
            7 // U8 type tag - this should reference the U8 type in the registry
        );
        // TODO: This needs proper type registry integration
        auto product = make_special_type(IDENTITY_TAG, 0); // TODO: fix type index
        return AlgebraicType::make_product(std::move(product));
    }
    
    inline AlgebraicType connection_id() {
        // ConnectionId is a ProductType with a single field tagged "__connection_id__" containing U64
        auto product = make_special_type(CONNECTION_ID_TAG, 13); // TODO: fix type index (U64)
        return AlgebraicType::make_product(std::move(product));
    }
    
    inline AlgebraicType timestamp() {
        // Timestamp is a ProductType with a single field tagged "__timestamp_micros_since_unix_epoch__" containing I64
        auto product = make_special_type(TIMESTAMP_TAG, 12); // TODO: fix type index (I64)
        return AlgebraicType::make_product(std::move(product));
    }
    
    inline AlgebraicType time_duration() {
        // TimeDuration is a ProductType with a single field tagged "__time_duration_micros__" containing I64
        auto product = make_special_type(TIME_DURATION_TAG, 12); // TODO: fix type index (I64)
        return AlgebraicType::make_product(std::move(product));
    }
    
} // namespace special_types

} // namespace spacetimedb::bsatn

// Legacy namespace alias
namespace SpacetimeDb::bsatn {
    using namespace ::spacetimedb::bsatn;
}

#endif // SPACETIMEDB_BSATN_SPECIAL_TYPES_H