#ifndef SPACETIMEDB_BSATN_ITYPEREGISTRAR_H
#define SPACETIMEDB_BSATN_ITYPEREGISTRAR_H

#include <cstdint>
#include <string>
#include <optional>

namespace SpacetimeDb::bsatn {

// Forward declarations
struct AlgebraicType;

/**
 * Interface for type registration.
 * This allows different components to register types without
 * depending on the full TypeRegistry implementation.
 */
class ITypeRegistrar {
public:
    virtual ~ITypeRegistrar() = default;
    
    // Register a type and return its ID
    virtual uint32_t register_type(AlgebraicType type) = 0;
    
    // Get a type by its ID
    virtual const AlgebraicType& get_type(uint32_t type_id) const = 0;
    
    // Register a named type
    virtual uint32_t register_named_type(const std::string& name, AlgebraicType type) = 0;
    
    // Find a type by name
    virtual std::optional<uint32_t> find_type(const std::string& name) const = 0;
};

} // namespace SpacetimeDb::bsatn

#endif // SPACETIMEDB_BSATN_ITYPEREGISTRAR_H