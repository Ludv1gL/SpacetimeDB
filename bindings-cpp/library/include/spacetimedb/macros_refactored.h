#ifndef SPACETIMEDB_MACROS_REFACTORED_H
#define SPACETIMEDB_MACROS_REFACTORED_H

#include "spacetimedb/bsatn_all.h"
#include "spacetimedb/reducer_context.h"
#include "spacetimedb/table_ops.h"
#include "spacetimedb/internal/Module.h"
#include "spacetimedb/internal/FFI.h"
#include <string>
#include <vector>
#include <cstring>

// =============================================================================
// HELPER MACROS
// =============================================================================

#define SPACETIMEDB_STRINGIFY_IMPL(x) #x
#define SPACETIMEDB_STRINGIFY(x) SPACETIMEDB_STRINGIFY_IMPL(x)
#define SPACETIMEDB_PASTE_IMPL(a, b) a##b
#define SPACETIMEDB_PASTE(a, b) SPACETIMEDB_PASTE_IMPL(a, b)

// =============================================================================
// TYPE SERIALIZATION HELPERS
// =============================================================================

// Generate BSATN serialization for a struct with fields
template<typename T>
void spacetimedb_serialize_struct_field(SpacetimeDb::bsatn::Writer& writer, const char* name, const T& value);

// Specializations for basic types
template<>
inline void spacetimedb_serialize_struct_field<uint8_t>(SpacetimeDb::bsatn::Writer& writer, const char* name, const uint8_t& value) {
    // Write field name
    writer.write_u8(0); // Some
    writer.write_u32_le(strlen(name));
    writer.write_bytes((const uint8_t*)name, strlen(name));
    writer.write_u8(7); // U8 = 7
}

template<>
inline void spacetimedb_serialize_struct_field<uint32_t>(SpacetimeDb::bsatn::Writer& writer, const char* name, const uint32_t& value) {
    writer.write_u8(0); // Some
    writer.write_u32_le(strlen(name));
    writer.write_bytes((const uint8_t*)name, strlen(name));
    writer.write_u8(11); // U32 = 11
}

template<>
inline void spacetimedb_serialize_struct_field<std::string>(SpacetimeDb::bsatn::Writer& writer, const char* name, const std::string& value) {
    writer.write_u8(0); // Some
    writer.write_u32_le(strlen(name));
    writer.write_bytes((const uint8_t*)name, strlen(name));
    writer.write_u8(4); // String = 4
}

// =============================================================================
// TABLE REGISTRATION MACRO
// =============================================================================

// Register a table - this needs to be called in global scope
#define SPACETIMEDB_TABLE(StructType) \
    namespace { \
        struct Register_##StructType##_Table { \
            Register_##StructType##_Table() { \
                auto& module = SpacetimeDb::Internal::Module::Instance(); \
                /* Create table definition */ \
                SpacetimeDb::Internal::RawTableDefV9 tableDef; \
                tableDef.name = #StructType; \
                tableDef.table_access = SpacetimeDb::Internal::TableAccess::Public; \
                tableDef.table_type = SpacetimeDb::Internal::TableType::User; \
                /* Generate type for the table */ \
                auto typeRef = module.RegisterTypeGeneric(#StructType, \
                    [](SpacetimeDb::Internal::AlgebraicTypeRef) -> std::vector<uint8_t> { \
                        SpacetimeDb::bsatn::Writer writer; \
                        /* Write ProductType */ \
                        writer.write_u8(2); /* Product type */ \
                        /* Write field count and fields */ \
                        StructType##_write_type_definition(writer); \
                        return writer.take_buffer(); \
                    } \
                ); \
                tableDef.product_type_ref = typeRef.idx; \
                /* Register the table */ \
                module.RegisterTableImpl(tableDef); \
            } \
        }; \
        static Register_##StructType##_Table register_##StructType##_table_instance; \
    } \
    /* Table operations handle */ \
    struct StructType##_Table { \
        static uint32_t get_table_id() { \
            static uint32_t id = 0; \
            if (id == 0) { \
                const char* name = #StructType; \
                SpacetimeDb::Internal::FFI::table_id_from_name( \
                    (const uint8_t*)name, strlen(name), &id \
                ); \
            } \
            return id; \
        } \
        static void insert(const StructType& row) { \
            SpacetimeDb::bsatn::Writer writer; \
            row.bsatn_serialize(writer); \
            auto buffer = writer.take_buffer(); \
            SpacetimeDb::Internal::FFI::datastore_insert_bsatn( \
                get_table_id(), buffer.data(), buffer.size() \
            ); \
        } \
    };

// =============================================================================
// STRUCT FIELD REGISTRATION MACROS
// =============================================================================

// Define how to write type definition for a struct
#define SPACETIMEDB_STRUCT_2(StructType, field_count) \
    void StructType##_write_type_definition(SpacetimeDb::bsatn::Writer& writer) { \
        writer.write_u32_le(field_count); \
    }

// Register fields for a struct (call after struct definition)
#define SPACETIMEDB_REGISTER_FIELD(StructType, field_name, field_type) \
    void StructType##_write_field_##field_name(SpacetimeDb::bsatn::Writer& writer) { \
        spacetimedb_serialize_struct_field<field_type>(writer, #field_name, field_type{}); \
    }

// =============================================================================
// REDUCER REGISTRATION MACRO
// =============================================================================

#define SPACETIMEDB_REDUCER(name) \
    void name##_impl(SpacetimeDb::ReducerContext& ctx, const uint8_t* args, size_t args_len); \
    namespace { \
        struct Register_##name##_Reducer { \
            Register_##name##_Reducer() { \
                auto& module = SpacetimeDb::Internal::Module::Instance(); \
                /* Create reducer definition */ \
                SpacetimeDb::Internal::RawReducerDefV9 reducerDef; \
                reducerDef.name = #name; \
                /* Generate parameter type */ \
                reducerDef.params = SpacetimeDb::Internal::ProductType{}; \
                name##_write_params(reducerDef.params); \
                reducerDef.lifecycle = std::nullopt; \
                /* Add to module */ \
                module.moduleDef.reducers.push_back(reducerDef); \
                /* Store reducer function */ \
                module.reducerFns.push_back( \
                    [](SpacetimeDb::ReducerContext ctx, const uint8_t* args, size_t args_len) -> SpacetimeDb::Internal::FFI::Errno { \
                        try { \
                            name##_impl(ctx, args, args_len); \
                            return SpacetimeDb::Internal::FFI::Errno::OK; \
                        } catch (...) { \
                            return SpacetimeDb::Internal::FFI::Errno::HOST_CALL_FAILURE; \
                        } \
                    } \
                ); \
                module.reducerNames.push_back(#name); \
            } \
        }; \
        static Register_##name##_Reducer register_##name##_reducer_instance; \
    } \
    void name##_impl(SpacetimeDb::ReducerContext& ctx, const uint8_t* args, size_t args_len)

// Declare reducer parameter structure
#define SPACETIMEDB_REDUCER_PARAMS_0(name) \
    void name##_write_params(SpacetimeDb::Internal::ProductType& params) { \
        /* No parameters */ \
    }

#define SPACETIMEDB_REDUCER_PARAMS_3(name, type1, name1) \
    void name##_write_params(SpacetimeDb::Internal::ProductType& params) { \
        SpacetimeDb::Internal::ProductTypeElement elem; \
        elem.name = #name1; \
        /* Set algebraic type based on type1 */ \
        name##_set_param_type_##name1(elem.algebraic_type); \
        params.elements.push_back(elem); \
    } \
    void name##_set_param_type_##name1(SpacetimeDb::Internal::AlgebraicType& type);

// =============================================================================
// BSATN SERIALIZATION SUPPORT
// =============================================================================

// Macro to implement BSATN serialization for a struct
#define SPACETIMEDB_BSATN_IMPL_3(StructType, field1_type, field1_name) \
    void StructType##_write_type_definition(SpacetimeDb::bsatn::Writer& writer) { \
        writer.write_u32_le(1); /* 1 field */ \
        spacetimedb_serialize_struct_field<field1_type>(writer, #field1_name, field1_type{}); \
    } \
    namespace { \
        /* Extension to add bsatn_serialize method */ \
        void bsatn_serialize_##StructType(const StructType& obj, SpacetimeDb::bsatn::Writer& writer) { \
            obj.field1_name.bsatn_serialize ? obj.field1_name.bsatn_serialize(writer) : writer.write<field1_type>(obj.field1_name); \
        } \
    }

// For 3-field structs
#define SPACETIMEDB_BSATN_IMPL_7(StructType, field1_type, field1_name, field2_type, field2_name, field3_type, field3_name) \
    void StructType##_write_type_definition(SpacetimeDb::bsatn::Writer& writer) { \
        writer.write_u32_le(3); /* 3 fields */ \
        spacetimedb_serialize_struct_field<field1_type>(writer, #field1_name, field1_type{}); \
        spacetimedb_serialize_struct_field<field2_type>(writer, #field2_name, field2_type{}); \
        spacetimedb_serialize_struct_field<field3_type>(writer, #field3_name, field3_type{}); \
    } \
    /* Add bsatn_serialize method to struct */ \
    inline void StructType##_bsatn_serialize(const StructType& obj, SpacetimeDb::bsatn::Writer& writer) { \
        writer.write<field1_type>(obj.field1_name); \
        writer.write<field2_type>(obj.field2_name); \
        writer.write<field3_type>(obj.field3_name); \
    }

// Extension method to add serialization
#define SPACETIMEDB_SERIALIZE_IMPL(StructType) \
    namespace SpacetimeDb { \
        template<> \
        inline void bsatn::Writer::write<StructType>(const StructType& value) { \
            StructType##_bsatn_serialize(value, *this); \
        } \
    } \
    /* Add member function */ \
    inline void bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const { \
        StructType##_bsatn_serialize(*this, writer); \
    }

#endif // SPACETIMEDB_MACROS_REFACTORED_H