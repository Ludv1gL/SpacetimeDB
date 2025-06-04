#ifndef SPACETIMEDB_BSATN_TYPE_REGISTRY_H
#define SPACETIMEDB_BSATN_TYPE_REGISTRY_H

#include "algebraic_type.h"
#include "traits.h"
#include <map>
#include <vector>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <mutex>

namespace spacetimedb::bsatn {

// Forward declarations
class ITypeRegistrar;
class TypeRegistry;

/**
 * Type registry for managing algebraic types and their IDs.
 * This provides a central place to register and look up types,
 * similar to the C# type system.
 */
class TypeRegistry : public ITypeRegistrar {
private:
    std::vector<AlgebraicType> types_;
    std::map<std::string, uint32_t> named_types_;
    std::map<std::type_index, uint32_t> cpp_type_map_;
    mutable std::mutex mutex_;
    
    // Special type IDs for common types
    static constexpr uint32_t UNIT_TYPE_ID = 0;
    
public:
    TypeRegistry() {
        // Register the unit type at index 0
        types_.push_back(AlgebraicType::make_product(
            std::make_unique<ProductType>(std::vector<ProductTypeElement>{})
        ));
        named_types_["Unit"] = UNIT_TYPE_ID;
    }
    
    // Register a type and return its ID
    uint32_t register_type(AlgebraicType type) override {
        std::lock_guard<std::mutex> lock(mutex_);
        uint32_t id = types_.size();
        types_.push_back(std::move(type));
        return id;
    }
    
    // Get a type by ID
    const AlgebraicType& get_type(uint32_t type_id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type_id >= types_.size()) {
            throw std::out_of_range("Invalid type ID: " + std::to_string(type_id));
        }
        return types_[type_id];
    }
    
    // Register a named type
    uint32_t register_named_type(const std::string& name, AlgebraicType type) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Check if already registered
        auto it = named_types_.find(name);
        if (it != named_types_.end()) {
            return it->second;
        }
        
        uint32_t id = types_.size();
        types_.push_back(std::move(type));
        named_types_[name] = id;
        return id;
    }
    
    // Find type ID by name
    std::optional<uint32_t> find_type(const std::string& name) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = named_types_.find(name);
        if (it != named_types_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    // Register a C++ type and return its ID
    template<typename T>
    uint32_t register_cpp_type() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::type_index type_idx(typeid(T));
        auto it = cpp_type_map_.find(type_idx);
        if (it != cpp_type_map_.end()) {
            return it->second;
        }
        
        // Get the algebraic type for T
        AlgebraicType alg_type = bsatn_traits<T>::algebraic_type();
        uint32_t id = types_.size();
        types_.push_back(std::move(alg_type));
        cpp_type_map_[type_idx] = id;
        
        // Also register with demangled name if possible
        const char* name = typeid(T).name();
        if (name) {
            named_types_[name] = id;
        }
        
        return id;
    }
    
    // Get or register a C++ type
    template<typename T>
    uint32_t get_or_register_type() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::type_index type_idx(typeid(T));
        
        auto it = cpp_type_map_.find(type_idx);
        if (it != cpp_type_map_.end()) {
            return it->second;
        }
        
        // Register new type
        AlgebraicType alg_type = bsatn_traits<T>::algebraic_type();
        uint32_t id = types_.size();
        types_.push_back(std::move(alg_type));
        cpp_type_map_[type_idx] = id;
        
        return id;
    }
    
    // Get the unit type ID
    static uint32_t unit_type_id() { return UNIT_TYPE_ID; }
    
    // Get the global type registry instance
    static TypeRegistry& instance() {
        static TypeRegistry registry;
        return registry;
    }
};

/**
 * RAII helper for building types with a specific registry.
 */
class TypeRegistryContext {
private:
    ITypeRegistrar* previous_;
    static thread_local ITypeRegistrar* current_;
    
public:
    explicit TypeRegistryContext(ITypeRegistrar* registrar) 
        : previous_(current_) {
        current_ = registrar;
    }
    
    ~TypeRegistryContext() {
        current_ = previous_;
    }
    
    static ITypeRegistrar* current() {
        return current_ ? current_ : &TypeRegistry::instance();
    }
};

// Define the thread-local storage
inline thread_local ITypeRegistrar* TypeRegistryContext::current_ = nullptr;

/**
 * Helper to build a ProductType with automatic type registration.
 */
template<typename T>
std::unique_ptr<ProductType> build_product_type() {
    ProductTypeBuilder builder(TypeRegistryContext::current());
    T::register_fields(builder);
    return builder.build();
}

/**
 * Macro to generate field registration for a struct.
 * Usage:
 *   struct MyStruct {
 *       int32_t x;
 *       std::string name;
 *       
 *       SPACETIMEDB_BSATN_REGISTER_TYPE(MyStruct, x, name)
 *   };
 */
#define SPACETIMEDB_BSATN_REGISTER_TYPE(Type, ...) \
    static void register_fields(spacetimedb::bsatn::ProductTypeBuilder& builder) { \
        SPACETIMEDB_BSATN_REGISTER_TYPE_FIELDS(Type, builder, __VA_ARGS__) \
    }

#define SPACETIMEDB_BSATN_REGISTER_TYPE_FIELD(Type, builder, field) \
    builder.with_field<decltype(Type::field)>(#field);

#define SPACETIMEDB_BSATN_REGISTER_TYPE_FIELDS(Type, builder, ...) \
    SPACETIMEDB_FOR_EACH_ARG(SPACETIMEDB_BSATN_REGISTER_TYPE_FIELD, Type, builder, __VA_ARGS__)

} // namespace spacetimedb::bsatn

// Legacy namespace alias
namespace SpacetimeDb::bsatn {
    using namespace ::spacetimedb::bsatn;
}

#endif // SPACETIMEDB_BSATN_TYPE_REGISTRY_H