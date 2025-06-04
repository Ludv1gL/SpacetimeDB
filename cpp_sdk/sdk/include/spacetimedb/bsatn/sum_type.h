#ifndef SPACETIMEDB_BSATN_SUM_TYPE_H
#define SPACETIMEDB_BSATN_SUM_TYPE_H

#include "reader.h"
#include "writer.h"
#include "traits.h"
#include <variant>
#include <type_traits>
#include <utility>

namespace spacetimedb::bsatn {

/**
 * SumType (discriminated union) implementation for BSATN.
 * This provides a type-safe way to represent values that can be one of several types.
 * Follows Rust/C# naming conventions for consistency.
 * 
 * Example usage:
 *   using MySum = SumType<int32_t, std::string, double>;
 *   MySum value = "hello";  // Automatically selects the string variant
 */
template<typename... Ts>
class SumType {
private:
    std::variant<Ts...> value_;
    
    // Helper to deserialize based on tag
    template<size_t... Is>
    static SumType deserialize_impl(Reader& reader, uint8_t tag, std::index_sequence<Is...>) {
        SumType result;
        bool success = ((tag == Is ? (result.value_ = deserialize<Ts>(reader), true) : false) || ...);
        if (!success) {
            throw std::runtime_error("Invalid sum type tag: " + std::to_string(tag));
        }
        return result;
    }
    
public:
    // Constructors
    SumType() = default;
    
    template<typename T>
    SumType(T&& value) : value_(std::forward<T>(value)) {}
    
    // Get the current tag (0-based index)
    uint8_t tag() const {
        return static_cast<uint8_t>(value_.index());
    }
    
    // Check if the sum holds a specific type
    template<typename T>
    bool is() const {
        return std::holds_alternative<T>(value_);
    }
    
    // Get the value of a specific type (throws if wrong type)
    template<typename T>
    const T& get() const {
        return std::get<T>(value_);
    }
    
    template<typename T>
    T& get() {
        return std::get<T>(value_);
    }
    
    // Get pointer to value of specific type (returns nullptr if wrong type)
    template<typename T>
    const T* get_if() const {
        return std::get_if<T>(&value_);
    }
    
    template<typename T>
    T* get_if() {
        return std::get_if<T>(&value_);
    }
    
    // Visit the sum with a visitor
    template<typename Visitor>
    auto visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }
    
    template<typename Visitor>
    auto visit(Visitor&& visitor) {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }
    
    // Equality comparison
    bool operator==(const SumType& other) const {
        return value_ == other.value_;
    }
    
    bool operator!=(const SumType& other) const {
        return !(*this == other);
    }
    
    // BSATN serialization
    void bsatn_serialize(Writer& writer) const {
        writer.write_u8(tag());
        std::visit([&writer](const auto& v) {
            serialize(writer, v);
        }, value_);
    }
    
    static SumType bsatn_deserialize(Reader& reader) {
        uint8_t tag = reader.read_u8();
        if (tag >= sizeof...(Ts)) {
            throw std::runtime_error("Invalid sum type tag: " + std::to_string(tag));
        }
        return deserialize_impl(reader, tag, std::index_sequence_for<Ts...>{});
    }
};

// Specialization of bsatn_traits for SumType
template<typename... Ts>
struct bsatn_traits<SumType<Ts...>> {
    static void serialize(Writer& writer, const SumType<Ts...>& value) {
        value.bsatn_serialize(writer);
    }
    
    static SumType<Ts...> deserialize(Reader& reader) {
        return SumType<Ts...>::bsatn_deserialize(reader);
    }
    
    static AlgebraicType algebraic_type() {
        SumTypeBuilder builder;
        size_t idx = 0;
        ((builder.with_variant<Ts>("Variant" + std::to_string(idx++)), ...));
        return AlgebraicType::make_sum(builder.build());
    }
};

/**
 * Tagged sum type with named variants.
 * This provides a more ergonomic way to define sum types with meaningful names.
 * 
 * Example:
 *   struct Ok { int32_t value; };
 *   struct Err { std::string message; };
 *   using Result = TaggedSum<
 *       TaggedVariant<"Ok", Ok>,
 *       TaggedVariant<"Err", Err>
 *   >;
 */
template<typename Name, typename T>
struct TaggedVariant {
    using name_type = Name;
    using value_type = T;
};

// Helper to extract string from string literal type
template<size_t N>
struct StringLiteral {
    char value[N];
    
    constexpr StringLiteral(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            value[i] = str[i];
        }
    }
    
    constexpr operator const char*() const { return value; }
    constexpr const char* c_str() const { return value; }
};

template<typename... Variants>
class TaggedSumType : public SumType<typename Variants::value_type...> {
private:
    using Base = SumType<typename Variants::value_type...>;
    
    template<typename T, size_t I = 0>
    static constexpr size_t variant_index() {
        if constexpr (I >= sizeof...(Variants)) {
            return size_t(-1);
        } else if constexpr (std::is_same_v<T, typename std::tuple_element_t<I, std::tuple<Variants...>>::value_type>) {
            return I;
        } else {
            return variant_index<T, I + 1>();
        }
    }
    
public:
    using Base::Base;
    
    // Get variant name by index
    static constexpr const char* variant_name(size_t index) {
        const char* names[] = { Variants::name_type::c_str()... };
        return index < sizeof...(Variants) ? names[index] : nullptr;
    }
    
    // Get current variant name
    const char* current_variant_name() const {
        return variant_name(this->tag());
    }
    
    // Create from specific variant type
    template<typename T>
    static TaggedSumType make(T&& value) {
        return TaggedSumType(std::forward<T>(value));
    }
};

/**
 * Option type (similar to Rust's Option or C#'s nullable reference types).
 * Specialized sum type for optional values.
 */
template<typename T>
class Option : public SumType<std::monostate, T> {
private:
    using Base = SumType<std::monostate, T>;
    
public:
    Option() : Base(std::monostate{}) {}  // None by default
    Option(std::nullopt_t) : Base(std::monostate{}) {}
    Option(const T& value) : Base(value) {}
    Option(T&& value) : Base(std::move(value)) {}
    
    bool has_value() const { return this->tag() == 1; }
    explicit operator bool() const { return has_value(); }
    
    const T& value() const {
        if (!has_value()) {
            throw std::runtime_error("Option is None");
        }
        return this->template get<T>();
    }
    
    T& value() {
        if (!has_value()) {
            throw std::runtime_error("Option is None");
        }
        return this->template get<T>();
    }
    
    const T& value_or(const T& default_value) const {
        return has_value() ? this->template get<T>() : default_value;
    }
    
    T* operator->() { return has_value() ? &this->template get<T>() : nullptr; }
    const T* operator->() const { return has_value() ? &this->template get<T>() : nullptr; }
    
    T& operator*() { return value(); }
    const T& operator*() const { return value(); }
    
    // Equality comparison
    bool operator==(const Option& other) const {
        if (has_value() != other.has_value()) return false;
        if (!has_value()) return true;  // Both are None
        return value() == other.value();
    }
    
    bool operator!=(const Option& other) const {
        return !(*this == other);
    }
};

// Specialization for Option to match SpacetimeDB's encoding
template<typename T>
struct bsatn_traits<Option<T>> {
    static void serialize(Writer& writer, const Option<T>& opt) {
        if (opt.has_value()) {
            writer.write_u8(1);  // Some
            spacetimedb::bsatn::serialize(writer, opt.value());
        } else {
            writer.write_u8(0);  // None
        }
    }
    
    static Option<T> deserialize(Reader& reader) {
        uint8_t tag = reader.read_u8();
        if (tag == 0) {
            return Option<T>();  // None
        } else if (tag == 1) {
            return Option<T>(spacetimedb::bsatn::deserialize<T>(reader));  // Some
        } else {
            throw std::runtime_error("Invalid Option tag: " + std::to_string(tag));
        }
    }
    
    static AlgebraicType algebraic_type() {
        SumTypeBuilder builder;
        builder.with_unit_variant("None");
        builder.with_variant<T>("Some");
        return AlgebraicType::make_sum(builder.build());
    }
};

// Backward compatibility aliases
template<typename... Ts>
using Sum = SumType<Ts...>;

template<typename... Variants>
using TaggedSum = TaggedSumType<Variants...>;

} // namespace spacetimedb::bsatn

// Legacy namespace alias for compatibility
namespace SpacetimeDb::bsatn {
    using namespace ::spacetimedb::bsatn;
}

#endif // SPACETIMEDB_BSATN_SUM_TYPE_H