// Comprehensive bridge providing ALL missing SpacetimeDB SDK functionality
#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <utility>

// Mock all the SpacetimeDB namespaces and types that the original sdk_test.h expects

namespace SpacetimeDb {
    namespace Types {
        struct uint128_t_placeholder {
            uint64_t low = 0;
            uint64_t high = 0;
            uint128_t_placeholder() = default;
            uint128_t_placeholder(uint64_t val) : low(val), high(0) {}
        };
        
        struct int128_t_placeholder {
            uint64_t low = 0;
            int64_t high = 0;
            int128_t_placeholder() = default;
            int128_t_placeholder(int64_t val) : low(val), high(val < 0 ? -1 : 0) {}
        };
    }
    
    namespace sdk {
        struct u256_placeholder {
            std::array<uint64_t, 4> data = {0, 0, 0, 0};
            u256_placeholder() = default;
            u256_placeholder(uint64_t val) { data[0] = val; }
        };
        
        struct i256_placeholder {
            std::array<uint64_t, 4> data = {0, 0, 0, 0};
            i256_placeholder() = default;
            i256_placeholder(int64_t val) { 
                data[0] = val; 
                if (val < 0) data[1] = data[2] = data[3] = 0xFFFFFFFFFFFFFFFFULL;
            }
        };
        
        struct Identity {
            std::array<uint8_t, 32> bytes = {};
            Identity() = default;
        };
        
        struct ConnectionId {
            uint64_t id = 0;
            ConnectionId() = default;
            ConnectionId(uint64_t val) : id(val) {}
        };
        
        struct Timestamp {
            uint64_t microseconds = 0;
            Timestamp() = default;
            Timestamp(uint64_t us) : microseconds(us) {}
        };
        
        struct TimeDuration {
            uint64_t microseconds = 0;
            TimeDuration() = default;
            TimeDuration(uint64_t us) : microseconds(us) {}
        };
        
        struct ScheduleAt {
            uint64_t timestamp = 0;
            ScheduleAt() = default;
            ScheduleAt(uint64_t ts) : timestamp(ts) {}
        };
    }
    
    enum class CoreType : uint8_t {
        Bool = 0, I8 = 1, U8 = 2, I16 = 3, U16 = 4, I32 = 5, U32 = 6,
        I64 = 7, U64 = 8, I128 = 9, U128 = 10, I256 = 11, U256 = 12,
        F32 = 13, F64 = 14, String = 15, Bytes = 16, UserDefined = 17
    };
    
    struct TypeIdentifier {
        CoreType core_type;
        const char* user_defined_name = nullptr;
    };
    
    struct FieldDefinition {
        const char* name;
        TypeIdentifier type;
        bool is_optional = false;
        bool is_unique = false;
        bool is_auto_increment = false;
    };
    
    struct EnumVariantDefinition {
        const char* name;
    };
    
    struct ReducerParameterDefinition {
        const char* name;
        TypeIdentifier type;
    };
    
    struct IndexDefinition {
        const char* index_name;
        std::vector<std::string> column_field_names;
    };
    
    enum class ReducerKind {
        Init,
        None,
        ClientConnected,
        ClientDisconnected,
        Scheduled
    };
    
    namespace bsatn {
        class Writer {
        public:
            void write_u8(uint8_t) {}
            void write_u16_le(uint16_t) {}
            void write_u32_le(uint32_t) {}
            void write_u64_le(uint64_t) {}
            void write_i8(int8_t) {}
            void write_i16_le(int16_t) {}
            void write_i32_le(int32_t) {}
            void write_i64_le(int64_t) {}
            void write_bool(bool) {}
            void write_f32_le(float) {}
            void write_f64_le(double) {}
            void write_string(const std::string&) {}
            template<typename T> void write_vector(const std::vector<T>&) {}
            template<typename T> void write_optional(const std::optional<T>&) {}
        };
        
        class Reader {
        public:
            uint8_t read_u8() { return 0; }
            uint16_t read_u16_le() { return 0; }
            uint32_t read_u32_le() { return 0; }
            uint64_t read_u64_le() { return 0; }
            int8_t read_i8() { return 0; }
            int16_t read_i16_le() { return 0; }
            int32_t read_i32_le() { return 0; }
            int64_t read_i64_le() { return 0; }
            bool read_bool() { return false; }
            float read_f32_le() { return 0.0f; }
            double read_f64_le() { return 0.0; }
            std::string read_string() { return ""; }
            
            Types::uint128_t_placeholder read_u128_le() { return Types::uint128_t_placeholder{}; }
            Types::int128_t_placeholder read_i128_le() { return Types::int128_t_placeholder{}; }
            sdk::u256_placeholder read_u256_le() { return sdk::u256_placeholder{}; }
            sdk::i256_placeholder read_i256_le() { return sdk::i256_placeholder{}; }
            
            template<typename T> std::vector<T> read_vector() { return std::vector<T>{}; }
            template<typename T> std::optional<T> read_optional() { return std::optional<T>{}; }
        };
        
        // Global serialization functions
        template<typename T>
        void serialize(Writer& writer, const T& value) {
            // Default empty implementation
        }
        
        template<typename T>
        T deserialize(Reader& reader) {
            return T{};
        }
        
        // Specializations for our bridge types
        template<>
        inline void serialize<Types::uint128_t_placeholder>(Writer& w, const Types::uint128_t_placeholder& v) {}
        
        template<>
        inline void serialize<sdk::Identity>(Writer& w, const sdk::Identity& v) {}
        
        template<>
        inline sdk::Identity deserialize<sdk::Identity>(Reader& r) { return sdk::Identity{}; }
        
        template<>
        inline Types::uint128_t_placeholder deserialize<Types::uint128_t_placeholder>(Reader& r) { 
            return Types::uint128_t_placeholder{}; 
        }
        
        // Add more specializations as needed...
    }
    
    // Mock module schema system
    class ModuleSchema {
    public:
        static ModuleSchema& instance() {
            static ModuleSchema inst;
            return inst;
        }
        
        void register_struct_type(const char*, const char*, const std::vector<FieldDefinition>&) {}
        void register_enum_type(const char*, const char*, const std::vector<EnumVariantDefinition>&) {}
        void register_table(const char*, const char*, bool, const char*) {}
        void set_primary_key(const char*, const char*) {}
        void add_index(const char*, const IndexDefinition&) {}
        void register_reducer(const char*, const char*, const std::vector<ReducerParameterDefinition>&, 
                             std::function<void(bsatn::Reader&)>, ReducerKind) {}
        void register_filter(const char*, const char*) {}
    };
    
    namespace ModuleRegistration {
        // Empty namespace for registration structs
    }
}

// Mock the macros that sdk_test.h uses
#define SPACETIMEDB_STRINGIFY_IMPL(x) #x
#define SPACETIMEDB_STRINGIFY(x) SPACETIMEDB_STRINGIFY_IMPL(x)

#define SPACETIMEDB_PASTE_IMPL(a, b) a##b
#define SPACETIMEDB_PASTE(a, b) SPACETIMEDB_PASTE_IMPL(a, b)

#define SPACETIMEDB_PASTE_PREFIX_IMPL(prefix, type) prefix##type
#define SPACETIMEDB_PASTE_PREFIX(prefix, type) SPACETIMEDB_PASTE_PREFIX_IMPL(prefix, type)

inline ::SpacetimeDb::FieldDefinition SPACETIMEDB_FIELD_INTERNAL(const char* name, ::SpacetimeDb::CoreType core_type, const char* user_defined_name, bool is_optional, bool is_unique_field, bool is_auto_inc_field) {
    ::SpacetimeDb::TypeIdentifier type_id;
    type_id.core_type = core_type;
    if (user_defined_name) {
        type_id.user_defined_name = user_defined_name;
    }
    ::SpacetimeDb::FieldDefinition field_def;
    field_def.name = name;
    field_def.type = type_id;
    field_def.is_optional = is_optional;
    field_def.is_unique = is_unique_field;
    field_def.is_auto_increment = is_auto_inc_field;
    return field_def;
}

#define SPACETIMEDB_FIELD(FieldNameStr, FieldCoreType, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, false, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_OPTIONAL(FieldNameStr, FieldCoreType, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, FieldCoreType, nullptr, true, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_CUSTOM(FieldNameStr, UserDefinedTypeNameStr, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, ::SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr, false, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_FIELD_CUSTOM_OPTIONAL(FieldNameStr, UserDefinedTypeNameStr, IsUniqueBool, IsAutoIncBool) \
    ::SPACETIMEDB_FIELD_INTERNAL(FieldNameStr, ::SpacetimeDb::CoreType::UserDefined, UserDefinedTypeNameStr, true, IsUniqueBool, IsAutoIncBool)

#define SPACETIMEDB_ENUM_VARIANT(VariantNameStr) \
    ::SpacetimeDb::EnumVariantDefinition{VariantNameStr}

// Mock the complex macros as no-ops but with proper syntax
#define SPACETIMEDB_TYPE_ENUM(CppTypeName, SanitizedCppTypeName, SpacetimeDbEnumNameStr, VariantsInitializerList) \
    static_assert(true, "Mock enum registration")

#define SPACETIMEDB_TYPE_STRUCT_WITH_FIELDS(CppTypeName, SanitizedCppTypeName, SpacetimeDbNameStr, FIELDS_MACRO, RegFieldsInitializerList) \
    static_assert(true, "Mock struct registration")

#define SPACETIMEDB_TABLE(CppRowTypeName, SpacetimeDbTableNameStr, IsPublicBool, ScheduledReducerNameStr) \
    static_assert(true, "Mock table registration")

#define SPACETIMEDB_PRIMARY_KEY(SpacetimeDbTableNameStr, FieldNameStr) \
    static_assert(true, "Mock primary key registration")

#define SPACETIMEDB_INDEX(SpacetimeDbTableNameStr, IndexNameStr, ColumnFieldNamesInitializerList) \
    static_assert(true, "Mock index registration")

// For any other macros that might be used
#define SPACETIMEDB_REDUCER_PARAM(ParamNameStr, ParamCoreType) \
    ::SpacetimeDb::ReducerParameterDefinition{}

#define SPACETIMEDB_REDUCER_PARAM_CUSTOM(ParamNameStr, UserDefinedTypeNameStr) \
    ::SpacetimeDb::ReducerParameterDefinition{}