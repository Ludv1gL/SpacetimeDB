#ifndef SPACETIMEDB_BSATN_COMPAT_H
#define SPACETIMEDB_BSATN_COMPAT_H

/**
 * Compatibility header to bridge namespace and type differences
 * between the BSATN library and the rest of the SDK.
 */

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

// Namespace aliases to handle inconsistencies
namespace spacetimedb {
    namespace bsatn = ::SpacetimeDb::bsatn;
}

namespace SpacetimeDb::bsatn {

    // Type conversion utilities between std::byte and uint8_t
    
    /**
     * Convert uint8_t span to byte span for Reader construction
     */
    inline std::span<const std::byte> as_bytes(std::span<const uint8_t> data) {
        return std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(data.data()), 
            data.size()
        );
    }
    
    /**
     * Convert uint8_t pointer and size to byte span
     */
    inline std::span<const std::byte> as_bytes(const uint8_t* data, size_t size) {
        return std::span<const std::byte>(
            reinterpret_cast<const std::byte*>(data), 
            size
        );
    }
    
    /**
     * Convert vector<byte> to vector<uint8_t>
     */
    inline std::vector<uint8_t> to_uint8_vector(std::vector<std::byte>&& bytes) {
        std::vector<uint8_t> result;
        result.reserve(bytes.size());
        for (auto b : bytes) {
            result.push_back(static_cast<uint8_t>(b));
        }
        return result;
    }
    
    /**
     * Convert vector<byte> to vector<uint8_t> (copy version)
     */
    inline std::vector<uint8_t> to_uint8_vector(const std::vector<std::byte>& bytes) {
        std::vector<uint8_t> result;
        result.reserve(bytes.size());
        for (auto b : bytes) {
            result.push_back(static_cast<uint8_t>(b));
        }
        return result;
    }
    
    /**
     * Helper class that wraps Reader to accept uint8_t data
     */
    class ReaderCompat : public Reader {
    public:
        ReaderCompat(const uint8_t* data, size_t size) 
            : Reader(as_bytes(data, size)) {}
            
        ReaderCompat(std::span<const uint8_t> data) 
            : Reader(as_bytes(data)) {}
            
        ReaderCompat(const std::vector<uint8_t>& data)
            : Reader(as_bytes(std::span<const uint8_t>(data))) {}
    };
    
    /**
     * Helper class that wraps Writer to provide uint8_t buffer access
     */
    class WriterCompat : public Writer {
    public:
        using Writer::Writer;
        
        std::vector<uint8_t> take_uint8_buffer() {
            return to_uint8_vector(take_buffer());
        }
        
        std::vector<uint8_t> get_uint8_buffer() const {
            auto bytes = get_buffer();
            return to_uint8_vector(bytes);
        }
    };
    
    // Forward declaration
    template<typename T>
    struct bsatn_traits;
    
    // ADL helper for custom type deserialization
    // This template will be found via ADL when deserializing custom types
    template<typename T>
    inline T spacetimedb_deserialize(Reader& reader, T*) {
        // Always use bsatn_traits which is the standard interface
        return bsatn_traits<T>::deserialize(reader);
    }
    
    // Helper to make Option work with string literals
    template<size_t N>
    Option<std::string> make_option(const char (&str)[N]) {
        return Option<std::string>(std::string(str));
    }
    
    inline Option<std::string> make_option(const char* str) {
        return str ? Option<std::string>(std::string(str)) : Option<std::string>();
    }
    
    inline Option<std::string> make_option(std::string str) {
        return Option<std::string>(std::move(str));
    }
    
    inline Option<std::string> make_option(std::nullopt_t) {
        return Option<std::string>();
    }

} // namespace SpacetimeDb::bsatn

// Make common types available in both namespaces
namespace spacetimedb {
    using SpacetimeDb::bsatn::ReaderCompat;
    using SpacetimeDb::bsatn::WriterCompat;
    using SpacetimeDb::bsatn::Option;
    using SpacetimeDb::bsatn::Sum;
    using SpacetimeDb::bsatn::AlgebraicType;
    // TypeRegistry was renamed to TypeRegistrar
    // using SpacetimeDb::bsatn::TypeRegistry;
    using SpacetimeDb::bsatn::make_option;
}

#endif // SPACETIMEDB_BSATN_COMPAT_H