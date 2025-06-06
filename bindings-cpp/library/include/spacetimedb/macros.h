#ifndef SPACETIMEDB_MACROS_H
#define SPACETIMEDB_MACROS_H

#include "spacetimedb/bsatn_all.h"
#include "spacetimedb/reducer_context.h"
#include "spacetimedb/table_ops.h"
#include "spacetimedb/schedule_reducer.h"
#include "spacetimedb/reducer_args.h"
#include "spacetimedb/rls.h"

#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <functional>
#include <type_traits>
#include <unordered_map>

// =============================================================================
// HELPER MACROS
// =============================================================================

// Helper macro to stringify its argument
#define SPACETIMEDB_STRINGIFY_IMPL(x) #x
#define SPACETIMEDB_STRINGIFY(x) SPACETIMEDB_STRINGIFY_IMPL(x)

// Helper macros for token pasting
#define SPACETIMEDB_PASTE_IMPL(a, b) a##b
#define SPACETIMEDB_PASTE(a, b) SPACETIMEDB_PASTE_IMPL(a, b)

// Helper to count macro arguments
#define SPACETIMEDB_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define SPACETIMEDB_NARGS(...) SPACETIMEDB_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// FOR_EACH implementation for variadic macros
#define SPACETIMEDB_FOR_EACH(MACRO, ...) \
    SPACETIMEDB_GET_MACRO(__VA_ARGS__, \
        SPACETIMEDB_FE_10, SPACETIMEDB_FE_9, SPACETIMEDB_FE_8, SPACETIMEDB_FE_7, \
        SPACETIMEDB_FE_6, SPACETIMEDB_FE_5, SPACETIMEDB_FE_4, SPACETIMEDB_FE_3, \
        SPACETIMEDB_FE_2, SPACETIMEDB_FE_1)(MACRO, __VA_ARGS__)

#ifndef SPACETIMEDB_GET_MACRO
#define SPACETIMEDB_GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,NAME,...) NAME
#endif

#ifndef SPACETIMEDB_FE_1
#define SPACETIMEDB_FE_1(MACRO, X) MACRO(X)
#define SPACETIMEDB_FE_2(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_1(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_3(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_2(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_4(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_3(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_5(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_4(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_6(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_5(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_7(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_6(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_8(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_7(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_9(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_8(MACRO, __VA_ARGS__)
#define SPACETIMEDB_FE_10(MACRO, X, ...) MACRO(X) SPACETIMEDB_FE_9(MACRO, __VA_ARGS__)
#endif

// Helper macro for stringifying each argument in a variadic list
#define SPACETIMEDB_STRINGIFY_EACH(...) SPACETIMEDB_STRINGIFY_EACH_IMPL(__VA_ARGS__)
#define SPACETIMEDB_STRINGIFY_EACH_IMPL(...) {SPACETIMEDB_FOR_EACH(SPACETIMEDB_STRINGIFY_ONE, __VA_ARGS__)}
#define SPACETIMEDB_STRINGIFY_ONE(x) SPACETIMEDB_STRINGIFY(x),

// =============================================================================
// INTERNAL UTILITIES
// =============================================================================

namespace SpacetimeDb {
namespace internal {

// Helper to get table ID from name - cached like Rust SDK
inline uint32_t get_table_id(const std::string& name) {
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

} // namespace internal
} // namespace SpacetimeDb

// =============================================================================
// TYPE GENERATION AND REGISTRATION
// =============================================================================

// Helper function to generate AlgebraicType for a type
// This is specialized for user types and handles primitives by default
template<typename T>
std::vector<uint8_t> spacetimedb_generate_type() {
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
    } else if constexpr (std::is_same_v<T, SpacetimeDb::U128>) {
        writer.write_u8(16); // U128
    } else if constexpr (std::is_same_v<T, SpacetimeDb::U256>) {
        writer.write_u8(17); // U256
    } else if constexpr (std::is_same_v<T, int8_t>) {
        writer.write_u8(6);  // I8
    } else if constexpr (std::is_same_v<T, int16_t>) {
        writer.write_u8(7);  // I16
    } else if constexpr (std::is_same_v<T, int32_t>) {
        writer.write_u8(8);  // I32
    } else if constexpr (std::is_same_v<T, int64_t>) {
        writer.write_u8(9);  // I64
    } else if constexpr (std::is_same_v<T, SpacetimeDb::I128>) {
        writer.write_u8(10); // I128
    } else if constexpr (std::is_same_v<T, SpacetimeDb::I256>) {
        writer.write_u8(11); // I256
    } else if constexpr (std::is_same_v<T, float>) {
        writer.write_u8(18); // F32
    } else if constexpr (std::is_same_v<T, double>) {
        writer.write_u8(19); // F64
    } else if constexpr (std::is_same_v<T, std::string>) {
        writer.write_u8(4);  // String
    } else if constexpr (std::is_same_v<T, SpacetimeDb::Identity>) {
        writer.write_u8(21); // Identity
    } else if constexpr (std::is_same_v<T, SpacetimeDb::Address>) {
        writer.write_u8(22); // Address
    } else {
        // For user types, try to use their registered algebraic type
        if constexpr (requires { SpacetimeDb::bsatn::bsatn_traits<T>::algebraic_type(); }) {
            auto alg_type = SpacetimeDb::bsatn::bsatn_traits<T>::algebraic_type();
            // TODO: Write the algebraic type properly
            writer.write_u8(2);  // Product type placeholder
            writer.write_u32_le(0); // Field count placeholder
        } else {
            // Fallback for types without BSATN traits
            writer.write_u8(2);  // Product type
            writer.write_u32_le(0); // Field count placeholder
        }
    }
    
    return writer.get_buffer();
}

// =============================================================================
// TABLE MACROS - C# [SpacetimeDB.Table] Equivalent
// =============================================================================

// Enhanced table macro with optional scheduling support
// Usage: SPACETIMEDB_TABLE(MyStruct, "table_name", true)
//    or: SPACETIMEDB_TABLE(MyStruct, "table_name", true, "scheduled_reducer", "scheduled_at")
#define SPACETIMEDB_TABLE(...) \
    SPACETIMEDB_PASTE(SPACETIMEDB_TABLE_, SPACETIMEDB_NARGS(__VA_ARGS__))(__VA_ARGS__)

// 3-argument version: Basic table
#define SPACETIMEDB_TABLE_3(RowType, table_name, is_public) \
    SPACETIMEDB_TABLE_IMPL(RowType, table_name, is_public, nullptr, nullptr)

// 5-argument version: Table with scheduling
#define SPACETIMEDB_TABLE_5(RowType, table_name, is_public, scheduled_reducer, scheduled_at) \
    SPACETIMEDB_TABLE_IMPL(RowType, table_name, is_public, scheduled_reducer, scheduled_at)

// Implementation macro
#define SPACETIMEDB_TABLE_IMPL(RowType, table_name, is_public, scheduled_reducer, scheduled_at) \
    struct RowType##__TableHandle { \
        SpacetimeDb::TableOps<RowType> ops; \
        \
        RowType##__TableHandle() : ops(SpacetimeDb::internal::get_table_id(table_name), table_name) {} \
        \
        uint64_t count() const { return ops.count(); } \
        SpacetimeDb::TableIterator<RowType> iter() const { return ops.iter(); } \
        RowType insert(const RowType& row) { return ops.insert(row); } \
        bool delete_row(const RowType& row) { return ops.delete_row(row); } \
    }; \
    \
    inline RowType##__TableHandle get_##RowType##_table() { \
        return RowType##__TableHandle{}; \
    } \
    \
    namespace { \
        struct Register_##RowType##_Table { \
            Register_##RowType##_Table() { \
                SpacetimeDb::Internal::Module::RegisterTable( \
                    table_name, \
                    is_public ? SpacetimeDb::Internal::TableAccess::Public : SpacetimeDb::Internal::TableAccess::Private, \
                    scheduled_reducer, \
                    scheduled_at, \
                    []() -> std::vector<uint8_t> { \
                        return spacetimedb_generate_type<RowType>(); \
                    } \
                ); \
            } \
        }; \
        static Register_##RowType##_Table register_##RowType##_table_instance; \
    }

// Scheduled table shorthand
#ifndef SPACETIMEDB_SCHEDULED_TABLE
#define SPACETIMEDB_SCHEDULED_TABLE(RowType, table_name, is_public, reducer_name) \
    SPACETIMEDB_TABLE_5(RowType, table_name, is_public, SPACETIMEDB_STRINGIFY(reducer_name), nullptr)
#endif

// =============================================================================
// REDUCER MACROS - C# [SpacetimeDB.Reducer] Equivalent
// =============================================================================

// Reducer kinds equivalent to C# ReducerKind enum
namespace SpacetimeDb {
    enum class ReducerKind {
        UserDefined = 0,
        Init = 1,
        ClientConnected = 2,
        ClientDisconnected = 3,
        Scheduled = 4
    };
}

// Helper to extract just types from parameter list
#define SPACETIMEDB_REDUCER_ARG_TYPE(type, name) type

// Reducer call helpers for different argument counts
#define SPACETIMEDB_REDUCER_CALL_0(name, ctx, args, args_len) \
    name(ctx);

#define SPACETIMEDB_REDUCER_CALL_2(name, ctx, args, args_len, type1, name1) \
    { \
        auto arg_tuple = SpacetimeDb::ReducerArgumentDeserializer<type1>::deserialize(args, args_len); \
        name(ctx, std::get<0>(arg_tuple)); \
    }

#define SPACETIMEDB_REDUCER_CALL_4(name, ctx, args, args_len, type1, name1, type2, name2) \
    { \
        auto arg_tuple = SpacetimeDb::ReducerArgumentDeserializer<type1, type2>::deserialize(args, args_len); \
        name(ctx, std::get<0>(arg_tuple), std::get<1>(arg_tuple)); \
    }

#define SPACETIMEDB_REDUCER_CALL_6(name, ctx, args, args_len, type1, name1, type2, name2, type3, name3) \
    { \
        auto arg_tuple = SpacetimeDb::ReducerArgumentDeserializer<type1, type2, type3>::deserialize(args, args_len); \
        name(ctx, std::get<0>(arg_tuple), std::get<1>(arg_tuple), std::get<2>(arg_tuple)); \
    }

#define SPACETIMEDB_REDUCER_CALL_8(name, ctx, args, args_len, type1, name1, type2, name2, type3, name3, type4, name4) \
    { \
        auto arg_tuple = SpacetimeDb::ReducerArgumentDeserializer<type1, type2, type3, type4>::deserialize(args, args_len); \
        name(ctx, std::get<0>(arg_tuple), std::get<1>(arg_tuple), std::get<2>(arg_tuple), std::get<3>(arg_tuple)); \
    }

// Enhanced reducer macro with ReducerKind support
// Usage: SPACETIMEDB_REDUCER_EX(my_reducer, UserDefined, ctx, int arg1, std::string arg2)
#define SPACETIMEDB_REDUCER_EX(name, kind, ctx_param, ...) \
    void name(SpacetimeDb::ReducerContext& ctx_param, ##__VA_ARGS__); \
    namespace { \
        SpacetimeDb::Internal::FFI::Errno name##_wrapper( \
            SpacetimeDb::ReducerContext ctx, \
            const uint8_t* args, size_t args_len \
        ) { \
            try { \
                SpacetimeDb::ReducerContext sctx(ctx); \
                SPACETIMEDB_REDUCER_CALL_##SPACETIMEDB_NARGS(__VA_ARGS__)(name, sctx, args, args_len, __VA_ARGS__) \
                return SpacetimeDb::Internal::FFI::Errno::OK; \
            } catch (const std::exception& e) { \
                return SpacetimeDb::Internal::FFI::Errno::HOST_CALL_FAILURE; \
            } \
        } \
        \
        struct Register_##name##_Reducer { \
            Register_##name##_Reducer() { \
                SpacetimeDb::Internal::Module::RegisterReducer( \
                    SPACETIMEDB_STRINGIFY(name), \
                    name##_wrapper, \
                    static_cast<int>(SpacetimeDb::ReducerKind::kind) \
                ); \
            } \
        }; \
        static Register_##name##_Reducer register_##name##_reducer_instance; \
    } \
    void name(SpacetimeDb::ReducerContext& ctx_param, ##__VA_ARGS__)

// Lifecycle reducer shortcuts - C# Attribute Equivalents
#define SPACETIMEDB_INIT(name, ctx_param, ...) \
    SPACETIMEDB_REDUCER_EX(name, Init, ctx_param, ##__VA_ARGS__)

#define SPACETIMEDB_CLIENT_CONNECTED(name, ctx_param, ...) \
    SPACETIMEDB_REDUCER_EX(name, ClientConnected, ctx_param, ##__VA_ARGS__)

#define SPACETIMEDB_CLIENT_DISCONNECTED(name, ctx_param, ...) \
    SPACETIMEDB_REDUCER_EX(name, ClientDisconnected, ctx_param, ##__VA_ARGS__)

// Required versions (future enhancement)
#define SPACETIMEDB_CLIENT_CONNECTED_REQUIRED(name, ctx_param, ...) \
    SPACETIMEDB_CLIENT_CONNECTED(name, ctx_param, ##__VA_ARGS__)

#define SPACETIMEDB_CLIENT_DISCONNECTED_REQUIRED(name, ctx_param, ...) \
    SPACETIMEDB_CLIENT_DISCONNECTED(name, ctx_param, ##__VA_ARGS__)

// Scheduled reducer with duration
#define SPACETIMEDB_SCHEDULED(name, duration, ctx_param, ...) \
    SPACETIMEDB_REDUCER_EX(name, Scheduled, ctx_param, ##__VA_ARGS__) \
    namespace { \
        struct Register_##name##_Schedule { \
            Register_##name##_Schedule() { \
                SpacetimeDb::ScheduleReducer::register_scheduled(SPACETIMEDB_STRINGIFY(name), duration); \
            } \
        }; \
        static Register_##name##_Schedule register_##name##_schedule_instance; \
    }

// Scheduled at specific time
#define SPACETIMEDB_SCHEDULED_AT(name, ctx_param, ...) \
    SPACETIMEDB_REDUCER_EX(name, Scheduled, ctx_param, ##__VA_ARGS__) \
    namespace { \
        struct Register_##name##_ScheduleAt { \
            Register_##name##_ScheduleAt() { \
                SpacetimeDb::ScheduleReducer::register_scheduled_at(SPACETIMEDB_STRINGIFY(name)); \
            } \
        }; \
        static Register_##name##_ScheduleAt register_##name##_schedule_at_instance; \
    }

// Convenience macros for common scheduled intervals
#define SPACETIMEDB_SCHEDULED_EVERY_SECOND(name, ctx_param, ...) \
    SPACETIMEDB_SCHEDULED(name, SpacetimeDb::Duration::from_seconds(1), ctx_param, ##__VA_ARGS__)

#define SPACETIMEDB_SCHEDULED_EVERY_MINUTE(name, ctx_param, ...) \
    SPACETIMEDB_SCHEDULED(name, SpacetimeDb::Duration::from_minutes(1), ctx_param, ##__VA_ARGS__)

#define SPACETIMEDB_SCHEDULED_EVERY_HOUR(name, ctx_param, ...) \
    SPACETIMEDB_SCHEDULED(name, SpacetimeDb::Duration::from_hours(1), ctx_param, ##__VA_ARGS__)

// =============================================================================
// INDEX MACROS - C# Index Attribute Equivalents
// =============================================================================

// Single column B-tree index
#define SPACETIMEDB_INDEX_BTREE(table_type, column_name) \
    namespace { \
        struct Register_##table_type##_##column_name##_BTreeIndex { \
            Register_##table_type##_##column_name##_BTreeIndex() { \
                SpacetimeDb::Internal::Module::RegisterBTreeIndex( \
                    SPACETIMEDB_STRINGIFY(table_type##_##column_name##_idx), \
                    SPACETIMEDB_STRINGIFY(table_type), \
                    {SPACETIMEDB_STRINGIFY(column_name)} \
                ); \
            } \
        }; \
        static Register_##table_type##_##column_name##_BTreeIndex register_##table_type##_##column_name##_btree_idx; \
    }

// Multi-column B-tree index
#define SPACETIMEDB_INDEX_BTREE_MULTI(table_type, index_name, ...) \
    namespace { \
        struct Register_##index_name##_Index { \
            Register_##index_name##_Index() { \
                std::vector<std::string> cols = SPACETIMEDB_STRINGIFY_EACH(__VA_ARGS__); \
                cols.pop_back(); /* Remove trailing comma */ \
                SpacetimeDb::Internal::Module::RegisterBTreeIndex( \
                    SPACETIMEDB_STRINGIFY(index_name), \
                    SPACETIMEDB_STRINGIFY(table_type), \
                    cols \
                ); \
            } \
        }; \
        static Register_##index_name##_Index register_##index_name##_index_instance; \
    }

// Unique constraint (implemented as unique index)
#define SPACETIMEDB_INDEX_UNIQUE(table_type, column_name) \
    namespace { \
        struct Register_##table_type##_##column_name##_UniqueIndex { \
            Register_##table_type##_##column_name##_UniqueIndex() { \
                SpacetimeDb::Internal::Module::RegisterUniqueIndex( \
                    SPACETIMEDB_STRINGIFY(table_type##_##column_name##_unique), \
                    SPACETIMEDB_STRINGIFY(table_type), \
                    SPACETIMEDB_STRINGIFY(column_name) \
                ); \
            } \
        }; \
        static Register_##table_type##_##column_name##_UniqueIndex register_##table_type##_##column_name##_unique_idx; \
    }

// Primary key index
#define SPACETIMEDB_INDEX_PRIMARY_KEY(table_type, column_name) \
    namespace { \
        struct Register_##table_type##_##column_name##_PrimaryKey { \
            Register_##table_type##_##column_name##_PrimaryKey() { \
                SpacetimeDb::Internal::Module::RegisterPrimaryKeyIndex( \
                    SPACETIMEDB_STRINGIFY(table_type), \
                    SPACETIMEDB_STRINGIFY(column_name) \
                ); \
            } \
        }; \
        static Register_##table_type##_##column_name##_PrimaryKey register_##table_type##_##column_name##_pk; \
    }

// Hash index
#define SPACETIMEDB_INDEX_HASH(table_type, column_name) \
    namespace { \
        struct Register_##table_type##_##column_name##_HashIndex { \
            Register_##table_type##_##column_name##_HashIndex() { \
                SpacetimeDb::Internal::Module::RegisterHashIndex( \
                    SPACETIMEDB_STRINGIFY(table_type##_##column_name##_hash), \
                    SPACETIMEDB_STRINGIFY(table_type), \
                    SPACETIMEDB_STRINGIFY(column_name) \
                ); \
            } \
        }; \
        static Register_##table_type##_##column_name##_HashIndex register_##table_type##_##column_name##_hash_idx; \
    }

// =============================================================================
// CONSTRAINT MACROS
// =============================================================================

// Foreign key constraint
#define SPACETIMEDB_FOREIGN_KEY(table, field, ref_table, ref_field) \
    static_assert(std::is_same_v< \
        decltype(std::declval<table>().field), \
        decltype(std::declval<ref_table>().ref_field) \
    >, "Foreign key field types must match"); \
    namespace { \
        struct Register_##table##_##field##_ForeignKey { \
            Register_##table##_##field##_ForeignKey() { \
                SpacetimeDb::ConstraintValidation::register_foreign_key( \
                    SPACETIMEDB_STRINGIFY(table), \
                    SPACETIMEDB_STRINGIFY(field), \
                    SPACETIMEDB_STRINGIFY(ref_table), \
                    SPACETIMEDB_STRINGIFY(ref_field) \
                ); \
            } \
        }; \
        static Register_##table##_##field##_ForeignKey register_##table##_##field##_fk; \
    }

// Check constraint
#define SPACETIMEDB_CHECK_CONSTRAINT(table, constraint_sql) \
    namespace { \
        struct Register_##table##_CheckConstraint { \
            Register_##table##_CheckConstraint() { \
                SpacetimeDb::ConstraintValidation::register_check_constraint( \
                    SPACETIMEDB_STRINGIFY(table), \
                    constraint_sql \
                ); \
            } \
        }; \
        static Register_##table##_CheckConstraint register_##table##_check_constraint; \
    }

// =============================================================================
// TYPE REGISTRATION MACROS - C# [SpacetimeDB.Type] Equivalent
// =============================================================================

// Register a type for BSATN serialization
#define SPACETIMEDB_TYPE(TypeName) \
    template<> \
    inline std::vector<uint8_t> spacetimedb_generate_type<TypeName>() { \
        SpacetimeDb::bsatn::Writer writer; \
        if constexpr (requires { SpacetimeDb::bsatn::bsatn_traits<TypeName>::algebraic_type(); }) { \
            auto alg_type = SpacetimeDb::bsatn::bsatn_traits<TypeName>::algebraic_type(); \
            /* TODO: Properly serialize the algebraic type */ \
            writer.write_u8(2); /* Product type */ \
            writer.write_u32_le(0); /* Field count - to be implemented */ \
        } else { \
            writer.write_u8(2); /* Product type */ \
            writer.write_u32_le(0); /* Field count placeholder */ \
        } \
        return writer.get_buffer(); \
    }

// Tagged enum support - C# [SpacetimeDB.Type] with tagged enum
#define SPACETIMEDB_TAGGED_ENUM(EnumType, ...) \
    namespace { \
        struct Register_##EnumType##_TaggedEnum { \
            Register_##EnumType##_TaggedEnum() { \
                /* TODO: Generate sum type information for enum variants */ \
            } \
        }; \
        static Register_##EnumType##_TaggedEnum register_##EnumType##_tagged_enum; \
    }

// =============================================================================
// FIELD ATTRIBUTE MACROS - C# Attribute Equivalents
// =============================================================================

// Field attribute flags equivalent to C# ColumnAttrs
namespace SpacetimeDb {
#ifndef SPACETIMEDB_COLUMN_ATTRS_DEFINED
#define SPACETIMEDB_COLUMN_ATTRS_DEFINED
    enum class ColumnAttrs : uint8_t {
        UnSet = 0b0000,
        Indexed = 0b0001,
        AutoInc = 0b0010,
        Unique = 0b0101,     // Indexed | 0b0100
        Identity = 0b0111,   // Unique | AutoInc
        PrimaryKey = 0b1101, // Unique | 0b1000
        PrimaryKeyAuto = 0b1111 // PrimaryKey | AutoInc
    };
#endif // SPACETIMEDB_COLUMN_ATTRS_DEFINED
}

// C++ attributes for field decoration
#define SPACETIMEDB_PRIMARY_KEY [[SpacetimeDb::primary_key]]
#define SPACETIMEDB_UNIQUE [[SpacetimeDb::unique]]
#define SPACETIMEDB_AUTO_INC [[SpacetimeDb::auto_inc]]
#define SPACETIMEDB_PRIMARY_KEY_AUTO [[SpacetimeDb::primary_key, SpacetimeDb::auto_inc]]
#define SPACETIMEDB_INDEX [[SpacetimeDb::index]]

// Type attributes
#define SPACETIMEDB_DATA_CONTRACT [[SpacetimeDb::data_contract]]
#define SPACETIMEDB_DATA_MEMBER(name) [[SpacetimeDb::data_member(#name)]]

// Field registration with attributes
#define SPACETIMEDB_FIELD(struct_name, field_name, attrs) \
    namespace { \
        struct Register_##struct_name##_##field_name##_Field { \
            Register_##struct_name##_##field_name##_Field() { \
                /* TODO: Register field with BSATN and attributes */ \
            } \
        }; \
        static Register_##struct_name##_##field_name##_Field register_##struct_name##_##field_name##_field; \
    }

// Rename attribute
#define SPACETIMEDB_RENAME(new_name) [[SpacetimeDb::rename(new_name)]]

// =============================================================================
// VISIBILITY FILTER MACRO
// =============================================================================

#define SPACETIMEDB_CLIENT_VISIBILITY_FILTER(filter_name, sql_query) \
    namespace { \
        struct Register_##filter_name##_Filter { \
            Register_##filter_name##_Filter() { \
                SpacetimeDb::Internal::Module::RegisterClientVisibilityFilter( \
                    #filter_name, \
                    sql_query \
                ); \
            } \
        }; \
        static Register_##filter_name##_Filter register_##filter_name##_filter_instance; \
    }

// =============================================================================
// MODULE METADATA MACROS
// =============================================================================

#define SPACETIMEDB_MODULE_VERSION(major, minor, patch) \
    namespace { \
        struct ModuleVersionRegistration { \
            ModuleVersionRegistration() { \
                SpacetimeDb::Internal::Module::SetVersion(major, minor, patch); \
            } \
        }; \
        static ModuleVersionRegistration module_version_registration; \
    }

#define SPACETIMEDB_MODULE_METADATA(name, author, desc, license) \
    namespace { \
        struct ModuleMetadataRegistration { \
            ModuleMetadataRegistration() { \
                SpacetimeDb::Internal::Module::SetMetadata(name, author, desc, license); \
            } \
        }; \
        static ModuleMetadataRegistration module_metadata_registration; \
    }

// =============================================================================
// ROW LEVEL SECURITY (RLS) MACROS - Future Implementation
// =============================================================================

// RLS policy registration macro
#define SPACETIMEDB_RLS_POLICY(table_name, policy_name, operation, condition) \
    namespace { \
        struct Register_##table_name##_##policy_name##_RLS { \
            Register_##table_name##_##policy_name##_RLS() { \
                if (!SpacetimeDb::validate_sql_condition(condition)) { \
                    static_assert(false, "Invalid SQL condition in RLS policy"); \
                } \
                SpacetimeDb::RlsPolicyRegistry::instance().register_policy( \
                    #table_name, \
                    #policy_name, \
                    SpacetimeDb::RlsOperation::operation, \
                    condition \
                ); \
            } \
        }; \
        static Register_##table_name##_##policy_name##_RLS register_##table_name##_##policy_name##_rls_instance; \
    }

#define SPACETIMEDB_RLS_SELECT(table_name, policy_name, condition) \
    SPACETIMEDB_RLS_POLICY(table_name, policy_name, Select, condition)

#define SPACETIMEDB_RLS_INSERT(table_name, policy_name, condition) \
    SPACETIMEDB_RLS_POLICY(table_name, policy_name, Insert, condition)

#define SPACETIMEDB_RLS_UPDATE(table_name, policy_name, condition) \
    SPACETIMEDB_RLS_POLICY(table_name, policy_name, Update, condition)

#define SPACETIMEDB_RLS_DELETE(table_name, policy_name, condition) \
    SPACETIMEDB_RLS_POLICY(table_name, policy_name, Delete, condition)

#endif // SPACETIMEDB_MACROS_H