#ifndef SPACETIMEDB_MACROS_H
#define SPACETIMEDB_MACROS_H

#include "spacetimedb/internal/Module.h"
#include "spacetimedb/bsatn_all.h"
#include "spacetimedb/sdk/reducer_context.h"
#include "spacetimedb/table_ops.h"

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits> // For std::is_same_v
#include <unordered_map>

// Helper macro to stringify its argument
#define SPACETIMEDB_STRINGIFY_IMPL(x) #x
#define SPACETIMEDB_STRINGIFY(x) SPACETIMEDB_STRINGIFY_IMPL(x)

// Helper macros for token pasting
#define SPACETIMEDB_PASTE_IMPL(a, b) a##b
#define SPACETIMEDB_PASTE(a, b) SPACETIMEDB_PASTE_IMPL(a, b)

// --- New Table and Reducer Definition Macros ---

// Helper to get table ID from name - cached like Rust
static uint32_t get_table_id(const std::string& name) {
    static std::unordered_map<std::string, uint32_t> cache;
    auto it = cache.find(name);
    if (it != cache.end()) {
        return it->second;
    }
    
    uint32_t id;
    auto err = SpacetimeDb::Internal::FFI::table_id_from_name(
        reinterpret_cast<const uint8_t*>(name.data()),
        static_cast<uint32_t>(name.size()),
        &id
    );
    
    if (err != SpacetimeDb::Internal::FFI::Errno::OK) {
        throw std::runtime_error("Table not found: " + name);
    }
    
    cache[name] = id;
    return id;
}

// Simplified table macro - combines Rust's directness with C++ type safety
#define SPACETIMEDB_TABLE(RowType, table_name, is_public) \
    /* Table handle type */ \
    struct table_name##__TableHandle { \
        SpacetimeDb::TableOps<RowType> ops; \
        \
        table_name##__TableHandle() : ops(get_table_id(SPACETIMEDB_STRINGIFY(table_name)), SPACETIMEDB_STRINGIFY(table_name)) {} \
        \
        /* Delegate to ops */ \
        uint64_t count() const { return ops.count(); } \
        SpacetimeDb::TableIterator<RowType> iter() const { return ops.iter(); } \
        RowType insert(const RowType& row) { return ops.insert(row); } \
        bool delete_row(const RowType& row) { return ops.delete_row(row); } \
    }; \
    \
    /* Global function to get table handle */ \
    inline table_name##__TableHandle get_##table_name##_table() { \
        return table_name##__TableHandle{}; \
    } \
    \
    /* Registration */ \
    namespace { \
        struct Register_##table_name##_Table { \
            Register_##table_name##_Table() { \
                SpacetimeDb::Internal::Module::RegisterTableDirect( \
                    SPACETIMEDB_STRINGIFY(table_name), \
                    is_public ? SpacetimeDb::Internal::TableAccess::Public : SpacetimeDb::Internal::TableAccess::Private, \
                    []() -> std::vector<uint8_t> { \
                        /* Generate AlgebraicType for RowType */ \
                        return spacetimedb_generate_type<RowType>(); \
                    } \
                ); \
            } \
        }; \
        static Register_##table_name##_Table register_##table_name##_table_instance; \
    }

// Helper to count macro arguments
#define SPACETIMEDB_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define SPACETIMEDB_NARGS(...) SPACETIMEDB_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Simplified reducer macro - Rust-like with C++ conveniences
#define SPACETIMEDB_REDUCER(name, ctx_param, ...) \
    void name(spacetimedb::ReducerContext& ctx_param, ##__VA_ARGS__); \
    namespace { \
        SpacetimeDb::Internal::FFI::Errno name##_wrapper( \
            SpacetimeDb::sdk::ReducerContext ctx, \
            const uint8_t* args, size_t args_len \
        ) { \
            try { \
                /* TODO: Deserialize arguments */ \
                spacetimedb::ReducerContext sctx; \
                name(sctx); \
                return SpacetimeDb::Internal::FFI::Errno::OK; \
            } catch (const std::exception& e) { \
                /* Log error */ \
                return SpacetimeDb::Internal::FFI::Errno::HOST_CALL_FAILURE; \
            } \
        } \
        \
        struct Register_##name##_Reducer { \
            Register_##name##_Reducer() { \
                SpacetimeDb::Internal::Module::RegisterReducerDirect( \
                    SPACETIMEDB_STRINGIFY(name), \
                    name##_wrapper \
                ); \
            } \
        }; \
        static Register_##name##_Reducer register_##name##_reducer_instance; \
    } \
    void name(spacetimedb::ReducerContext& ctx_param, ##__VA_ARGS__)

// Helper function to generate AlgebraicType for a type
// This should be specialized for user types
template<typename T>
std::vector<uint8_t> spacetimedb_generate_type() {
    // Default implementation for primitive types
    SpacetimeDb::bsatn::Writer writer;
    
    // Determine type tag based on T
    if constexpr (std::is_same_v<T, bool>) {
        writer.write_u8(5);  // Bool
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        writer.write_u8(12); // U8
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        writer.write_u8(13); // U16
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        writer.write_u8(14); // U32
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        writer.write_u8(15); // U64
    } else if constexpr (std::is_same_v<T, int8_t>) {
        writer.write_u8(6);  // I8
    } else if constexpr (std::is_same_v<T, int16_t>) {
        writer.write_u8(7);  // I16
    } else if constexpr (std::is_same_v<T, int32_t>) {
        writer.write_u8(8);  // I32
    } else if constexpr (std::is_same_v<T, int64_t>) {
        writer.write_u8(9);  // I64
    } else if constexpr (std::is_same_v<T, float>) {
        writer.write_u8(18); // F32
    } else if constexpr (std::is_same_v<T, double>) {
        writer.write_u8(19); // F64
    } else if constexpr (std::is_same_v<T, std::string>) {
        writer.write_u8(4);  // String
    } else {
        // For user types, generate a product type
        writer.write_u8(2);  // Product
        writer.write_u32_le(0); // TODO: Add field count
    }
    
    return writer.take_buffer();
}

// Macro to specialize type generation for user structs
#define SPACETIMEDB_TYPE(Type, ...) \
    template<> \
    std::vector<uint8_t> spacetimedb_generate_type<Type>() { \
        SpacetimeDb::bsatn::Writer writer; \
        writer.write_u8(2); /* Product type */ \
        /* TODO: Generate field information */ \
        writer.write_u32_le(0); /* Field count */ \
        return writer.take_buffer(); \
    }

// Index support - TODO: implement when index API is available
#define SPACETIMEDB_INDEX(table_name, index_name, ...) \
    /* Index registration will be implemented later */

// Built-in reducers - TODO: implement special handling
#define SPACETIMEDB_INIT() \
    /* Init reducer will be implemented later */

#define SPACETIMEDB_CLIENT_CONNECTED() \
    /* Client connected reducer will be implemented later */

#define SPACETIMEDB_CLIENT_DISCONNECTED() \
    /* Client disconnected reducer will be implemented later */
// Field registration helper - simplified for now
#define SPACETIMEDB_REGISTER_FIELDS(Type, ...) \
    /* Field registration will be implemented with BSATN serialization */

// Helper to simplify reducer argument deserialization - TODO: implement
template<typename... Args>
std::tuple<Args...> deserialize_reducer_args(const uint8_t* data, size_t len) {
    // This will be implemented when we have proper BSATN deserialization
    return std::make_tuple(Args{}...);
}

// =============================================================================
// TYPE REGISTRATION MACROS
// =============================================================================

// Generic type registration macro for autogenerated types
// This registers a type with the SpacetimeDB type system for BSATN serialization
// and module definition generation. Used by autogenerated .g.h files.
#define SPACETIMEDB_REGISTER_TYPE(TypeName) \
    namespace spacetimedb { \
    namespace detail { \
    template<> \
    struct TypeRegistrar<SpacetimeDb::Internal::TypeName> { \
        static AlgebraicTypeRef register_type(TypeContext& ctx) { \
            /* Type registration implementation will be provided by */ \
            /* the autogenerated code when the type system is complete */ \
            return ctx.add_placeholder(#TypeName); \
        } \
    }; \
    } /* namespace detail */ \
    } /* namespace spacetimedb */

#endif // SPACETIMEDB_MACROS_H
