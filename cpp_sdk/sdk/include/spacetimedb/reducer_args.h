#pragma once

#include "bsatn/reader.h"
#include "bsatn/writer.h"
#include "bsatn/traits.h"
#include <tuple>
#include <utility>

namespace spacetimedb {

// Helper to deserialize multiple arguments from BSATN
template<typename... Args>
class ReducerArgumentDeserializer {
public:
    static std::tuple<Args...> deserialize(const uint8_t* data, size_t len) {
        spacetimedb::bsatn::Reader reader(data, len);
        return deserialize_tuple<Args...>(reader, std::index_sequence_for<Args...>{});
    }
    
private:
    template<typename... T, size_t... Is>
    static std::tuple<T...> deserialize_tuple(spacetimedb::bsatn::Reader& reader, std::index_sequence<Is...>) {
        return std::make_tuple(deserialize_arg<T>(reader, Is)...);
    }
    
    template<typename T>
    static T deserialize_arg(spacetimedb::bsatn::Reader& reader, size_t index) {
        // Each argument is wrapped in a ProductTypeElement
        // Skip the field name
        auto name_opt = reader.read_u8();
        if (name_opt == 0) { // Some
            reader.read_string(); // Skip the field name
        }
        
        // Deserialize the actual value
        return spacetimedb::bsatn::bsatn_traits<T>::deserialize(reader);
    }
};

// Helper to serialize multiple arguments to BSATN
template<typename... Args>
class ReducerArgumentSerializer {
public:
    static std::vector<uint8_t> serialize(const Args&... args) {
        std::vector<uint8_t> buffer;
        spacetimedb::bsatn::Writer writer(buffer);
        
        // Write as a product type
        writer.write_u8(2); // AlgebraicType::Product
        writer.write_vec_len(sizeof...(Args));
        
        size_t index = 0;
        (serialize_arg(writer, args, index++), ...);
        
        return buffer;
    }
    
private:
    template<typename T>
    static void serialize_arg(spacetimedb::bsatn::Writer& writer, const T& arg, size_t index) {
        // Write ProductTypeElement
        writer.write_u8(0); // Some
        writer.write_string("arg" + std::to_string(index));
        
        // Write the type (simplified - in reality this should use type registry)
        spacetimedb::bsatn::algebraic_type_of<T>::get().write_bsatn(writer);
        
        // Write the value
        spacetimedb::bsatn::bsatn_traits<T>::serialize(writer, arg);
    }
};

// Helper to generate parameter type info for registration
template<typename... Args>
class ReducerParameterInfo {
public:
    static std::vector<std::string> get_param_types() {
        std::vector<std::string> types;
        (add_type<Args>(types), ...);
        return types;
    }
    
    static std::vector<uint32_t> get_param_type_refs() {
        std::vector<uint32_t> refs;
        (add_type_ref<Args>(refs), ...);
        return refs;
    }
    
private:
    template<typename T>
    static void add_type(std::vector<std::string>& types) {
        // Map C++ types to type names
        if constexpr (std::is_same_v<T, int32_t>) {
            types.push_back("i32");
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            types.push_back("u32");
        } else if constexpr (std::is_same_v<T, int64_t>) {
            types.push_back("i64");
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            types.push_back("u64");
        } else if constexpr (std::is_same_v<T, std::string>) {
            types.push_back("string");
        } else if constexpr (std::is_same_v<T, bool>) {
            types.push_back("bool");
        } else {
            types.push_back("custom");
        }
    }
    
    template<typename T>
    static void add_type_ref(std::vector<uint32_t>& refs) {
        // Get type reference from registry
        auto ref = spacetimedb::bsatn::TypeRegistry::instance().get_or_register_type<T>();
        refs.push_back(ref);
    }
};

} // namespace spacetimedb