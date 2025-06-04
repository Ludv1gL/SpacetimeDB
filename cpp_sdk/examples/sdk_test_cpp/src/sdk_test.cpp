#include "../../../sdk/include/spacetimedb/spacetimedb_core.h"
#include <cstdio>
#include <cstring>
#include <type_traits>

// =============================================================================
// ENUMS
// =============================================================================

enum class SimpleEnum : uint8_t {
    Zero = 0,
    One = 1,
    Two = 2
};

// Note: C++ doesn't support enums with payloads like Rust, so we use a tagged union
struct EnumWithPayload {
    enum Tag : uint8_t {
        U8, U16, U32, U64, U128, U256,
        I8, I16, I32, I64, I128, I256,
        Bool, F32, F64, Str,
        Identity, ConnectionId, Timestamp,
        Bytes, Ints, Strings, SimpleEnums
    } tag;
    
    union {
        uint8_t u8_val;
        uint16_t u16_val;
        uint32_t u32_val;
        uint64_t u64_val;
        // Note: u128, u256, i128, i256 would need custom types
        int8_t i8_val;
        int16_t i16_val;
        int32_t i32_val;
        int64_t i64_val;
        bool bool_val;
        float f32_val;
        double f64_val;
        // String, vectors, and complex types would need proper memory management
    };
    
    // For simplicity, we'll focus on basic types in this test
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const EnumWithPayload& value) {
        buffer.push_back(static_cast<uint8_t>(value.tag));
        switch (value.tag) {
            case U8: buffer.push_back(value.u8_val); break;
            case U16: 
                buffer.push_back(value.u16_val & 0xFF);
                buffer.push_back((value.u16_val >> 8) & 0xFF);
                break;
            case U32:
                buffer.push_back(value.u32_val & 0xFF);
                buffer.push_back((value.u32_val >> 8) & 0xFF);
                buffer.push_back((value.u32_val >> 16) & 0xFF);
                buffer.push_back((value.u32_val >> 24) & 0xFF);
                break;
            // Add more cases as needed
            default: break;
        }
    }
};

// =============================================================================
// STRUCTS
// =============================================================================

struct UnitStruct {
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const UnitStruct&) {
        // Unit struct has no fields
    }
};

struct ByteStruct {
    uint8_t b;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const ByteStruct& value) {
        buffer.push_back(value.b);
    }
};

struct EveryPrimitiveStruct {
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint64_t d;
    // u128, u256 would need custom types
    int8_t g;
    int16_t h;
    int32_t i;
    int64_t j;
    // i128, i256 would need custom types
    bool m;
    float n;
    double o;
    std::string p;
    spacetimedb::Identity q;
    spacetimedb::ConnectionId r;
    spacetimedb::Timestamp s;
    // TimeDuration would need to be defined
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const EveryPrimitiveStruct& value) {
        // Serialize each field
        buffer.push_back(value.a);
        
        buffer.push_back(value.b & 0xFF);
        buffer.push_back((value.b >> 8) & 0xFF);
        
        buffer.push_back(value.c & 0xFF);
        buffer.push_back((value.c >> 8) & 0xFF);
        buffer.push_back((value.c >> 16) & 0xFF);
        buffer.push_back((value.c >> 24) & 0xFF);
        
        // Continue for other fields...
        // For brevity, not implementing all fields
    }
};

// =============================================================================
// TABLES - Single value tables
// =============================================================================

// Helper template for serialization
template<typename T>
void serialize_field(std::vector<uint8_t>& buffer, const T& value) {
    if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t> || std::is_same_v<T, bool>) {
        buffer.push_back(static_cast<uint8_t>(value));
    } else if constexpr (std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t>) {
        buffer.push_back(value & 0xFF);
        buffer.push_back((value >> 8) & 0xFF);
    } else if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t>) {
        buffer.push_back(value & 0xFF);
        buffer.push_back((value >> 8) & 0xFF);
        buffer.push_back((value >> 16) & 0xFF);
        buffer.push_back((value >> 24) & 0xFF);
    } else if constexpr (std::is_same_v<T, uint64_t> || std::is_same_v<T, int64_t>) {
        for (int i = 0; i < 8; ++i) {
            buffer.push_back((value >> (i * 8)) & 0xFF);
        }
    } else if constexpr (std::is_same_v<T, float>) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        buffer.insert(buffer.end(), bytes, bytes + sizeof(float));
    } else if constexpr (std::is_same_v<T, double>) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        buffer.insert(buffer.end(), bytes, bytes + sizeof(double));
    } else if constexpr (std::is_same_v<T, std::string>) {
        uint32_t len = value.length();
        buffer.push_back(len & 0xFF);
        buffer.push_back((len >> 8) & 0xFF);
        buffer.push_back((len >> 16) & 0xFF);
        buffer.push_back((len >> 24) & 0xFF);
        buffer.insert(buffer.end(), value.begin(), value.end());
    }
}

// Macro to define single-value tables
#define DEFINE_SINGLE_VALUE_TABLE(TypeName, FieldType, field_name) \
struct TypeName { \
    FieldType field_name; \
    \
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const TypeName& value) { \
        serialize_field(buffer, value.field_name); \
    } \
}; \
SPACETIMEDB_TABLE(TypeName, TypeName##_table, true)

// Define all single-value tables
DEFINE_SINGLE_VALUE_TABLE(OneU8, uint8_t, n)
DEFINE_SINGLE_VALUE_TABLE(OneU16, uint16_t, n)
DEFINE_SINGLE_VALUE_TABLE(OneU32, uint32_t, n)
DEFINE_SINGLE_VALUE_TABLE(OneU64, uint64_t, n)

DEFINE_SINGLE_VALUE_TABLE(OneI8, int8_t, n)
DEFINE_SINGLE_VALUE_TABLE(OneI16, int16_t, n)
DEFINE_SINGLE_VALUE_TABLE(OneI32, int32_t, n)
DEFINE_SINGLE_VALUE_TABLE(OneI64, int64_t, n)

DEFINE_SINGLE_VALUE_TABLE(OneBool, bool, b)
DEFINE_SINGLE_VALUE_TABLE(OneF32, float, f)
DEFINE_SINGLE_VALUE_TABLE(OneF64, double, f)
DEFINE_SINGLE_VALUE_TABLE(OneString, std::string, s)

// Special types need custom handling
struct OneIdentity {
    spacetimedb::Identity i;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneIdentity& value) {
        buffer.insert(buffer.end(), value.i.data, value.i.data + 32);
    }
};
SPACETIMEDB_TABLE(OneIdentity, OneIdentity_table, true)

struct OneConnectionId {
    spacetimedb::ConnectionId a;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneConnectionId& value) {
        for (int i = 0; i < 8; ++i) {
            buffer.push_back((value.a.high >> (i * 8)) & 0xFF);
        }
        for (int i = 0; i < 8; ++i) {
            buffer.push_back((value.a.low >> (i * 8)) & 0xFF);
        }
    }
};
SPACETIMEDB_TABLE(OneConnectionId, OneConnectionId_table, true)

struct OneTimestamp {
    spacetimedb::Timestamp t;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneTimestamp& value) {
        for (int i = 0; i < 8; ++i) {
            buffer.push_back((value.t >> (i * 8)) & 0xFF);
        }
    }
};
SPACETIMEDB_TABLE(OneTimestamp, OneTimestamp_table, true)

// =============================================================================
// TABLES - Unique constraint tables
// =============================================================================

// Macro for unique constraint tables
#define DEFINE_UNIQUE_TABLE(TypeName, FieldType, field_name) \
struct TypeName { \
    FieldType field_name; \
    int32_t data; \
    \
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const TypeName& value) { \
        /* Serialize the unique field */ \
        serialize_field(buffer, value.field_name); \
        /* Serialize data field */ \
        serialize_field(buffer, value.data); \
    } \
}; \
SPACETIMEDB_TABLE(TypeName, TypeName##_table, true)

DEFINE_UNIQUE_TABLE(UniqueU8, uint8_t, n)
DEFINE_UNIQUE_TABLE(UniqueU16, uint16_t, n)
DEFINE_UNIQUE_TABLE(UniqueU32, uint32_t, n)
DEFINE_UNIQUE_TABLE(UniqueU64, uint64_t, n)

DEFINE_UNIQUE_TABLE(UniqueI8, int8_t, n)
DEFINE_UNIQUE_TABLE(UniqueI16, int16_t, n)
DEFINE_UNIQUE_TABLE(UniqueI32, int32_t, n)
DEFINE_UNIQUE_TABLE(UniqueI64, int64_t, n)

DEFINE_UNIQUE_TABLE(UniqueBool, bool, b)
DEFINE_UNIQUE_TABLE(UniqueString, std::string, s)

// =============================================================================
// TABLES - Primary key tables
// =============================================================================

// Use same macro pattern for primary key tables
#define DEFINE_PK_TABLE(TypeName, FieldType, field_name) \
    DEFINE_UNIQUE_TABLE(TypeName, FieldType, field_name)

DEFINE_PK_TABLE(PkU8, uint8_t, n)
DEFINE_PK_TABLE(PkU16, uint16_t, n)
DEFINE_PK_TABLE(PkU32, uint32_t, n)
DEFINE_PK_TABLE(PkU64, uint64_t, n)

DEFINE_PK_TABLE(PkI8, int8_t, n)
DEFINE_PK_TABLE(PkI16, int16_t, n)
DEFINE_PK_TABLE(PkI32, int32_t, n)
DEFINE_PK_TABLE(PkI64, int64_t, n)

DEFINE_PK_TABLE(PkBool, bool, b)
DEFINE_PK_TABLE(PkString, std::string, s)

// =============================================================================
// REDUCERS - Insert operations
// =============================================================================

// Helper to get appropriate test value for a type
template<typename T>
T get_test_value() {
    if constexpr (std::is_same_v<T, uint8_t>) return 42;
    else if constexpr (std::is_same_v<T, uint16_t>) return 1234;
    else if constexpr (std::is_same_v<T, uint32_t>) return 123456;
    else if constexpr (std::is_same_v<T, uint64_t>) return 1234567890;
    else if constexpr (std::is_same_v<T, int8_t>) return -42;
    else if constexpr (std::is_same_v<T, int16_t>) return -1234;
    else if constexpr (std::is_same_v<T, int32_t>) return -123456;
    else if constexpr (std::is_same_v<T, int64_t>) return -1234567890;
    else if constexpr (std::is_same_v<T, bool>) return true;
    else if constexpr (std::is_same_v<T, float>) return 3.14f;
    else if constexpr (std::is_same_v<T, double>) return 3.14159;
    else if constexpr (std::is_same_v<T, std::string>) return std::string("test string");
    else return T{};
}

// Macro to define insert reducers
#define DEFINE_INSERT_REDUCER(table_type, reducer_name, field_type, field_name) \
SPACETIMEDB_REDUCER(reducer_name, spacetimedb::ReducerContext ctx) { \
    /* For now, use test values since we don't have arg parsing yet */ \
    table_type row; \
    row.field_name = get_test_value<field_type>(); \
    \
    auto table_id = ctx.db->table<table_type>(#table_type "_table").get_table_id(); \
    if (table_id != 0) { \
        std::vector<uint8_t> buffer; \
        table_type::spacetimedb_serialize(buffer, row); \
        size_t buffer_size = buffer.size(); \
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size); \
        if (result == 0) { \
            LOG_INFO("Successfully inserted into " #table_type); \
        } else { \
            LOG_ERROR("Failed to insert into " #table_type); \
        } \
    } \
}

// Define insert reducers for all single-value tables
DEFINE_INSERT_REDUCER(OneU8, insert_one_u8, uint8_t, n)
DEFINE_INSERT_REDUCER(OneU16, insert_one_u16, uint16_t, n)
DEFINE_INSERT_REDUCER(OneU32, insert_one_u32, uint32_t, n)
DEFINE_INSERT_REDUCER(OneU64, insert_one_u64, uint64_t, n)

DEFINE_INSERT_REDUCER(OneI8, insert_one_i8, int8_t, n)
DEFINE_INSERT_REDUCER(OneI16, insert_one_i16, int16_t, n)
DEFINE_INSERT_REDUCER(OneI32, insert_one_i32, int32_t, n)
DEFINE_INSERT_REDUCER(OneI64, insert_one_i64, int64_t, n)

DEFINE_INSERT_REDUCER(OneBool, insert_one_bool, bool, b)
DEFINE_INSERT_REDUCER(OneF32, insert_one_f32, float, f)
DEFINE_INSERT_REDUCER(OneF64, insert_one_f64, double, f)
DEFINE_INSERT_REDUCER(OneString, insert_one_string, std::string, s)

// Special reducers for Identity and ConnectionId
SPACETIMEDB_REDUCER(insert_caller_one_identity, spacetimedb::ReducerContext ctx) {
    OneIdentity row{ctx.sender};
    
    auto table_id = ctx.db->table<OneIdentity>("OneIdentity_table").get_table_id();
    if (table_id != 0) {
        std::vector<uint8_t> buffer;
        OneIdentity::spacetimedb_serialize(buffer, row);
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
        if (result == 0) {
            LOG_INFO("Successfully inserted caller identity");
        } else {
            LOG_ERROR("Failed to insert caller identity");
        }
    }
}

SPACETIMEDB_REDUCER(insert_caller_one_connection_id, spacetimedb::ReducerContext ctx) {
    if (ctx.connection_id.has_value()) {
        OneConnectionId row{ctx.connection_id.value()};
        
        auto table_id = ctx.db->table<OneConnectionId>("OneConnectionId_table").get_table_id();
        if (table_id != 0) {
            std::vector<uint8_t> buffer;
            OneConnectionId::spacetimedb_serialize(buffer, row);
            size_t buffer_size = buffer.size();
            uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
            if (result == 0) {
                LOG_INFO("Successfully inserted caller connection ID");
            } else {
                LOG_ERROR("Failed to insert caller connection ID");
            }
        }
    } else {
        LOG_ERROR("No connection ID in reducer context");
    }
}

SPACETIMEDB_REDUCER(insert_call_timestamp, spacetimedb::ReducerContext ctx) {
    OneTimestamp row{ctx.timestamp};
    
    auto table_id = ctx.db->table<OneTimestamp>("OneTimestamp_table").get_table_id();
    if (table_id != 0) {
        std::vector<uint8_t> buffer;
        OneTimestamp::spacetimedb_serialize(buffer, row);
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
        if (result == 0) {
            LOG_INFO("Successfully inserted timestamp");
        } else {
            LOG_ERROR("Failed to insert timestamp");
        }
    }
}

// No-op reducer for testing
SPACETIMEDB_REDUCER(no_op_succeeds, spacetimedb::ReducerContext ctx) {
    LOG_INFO("No-op reducer succeeded");
}

// Init reducer
SPACETIMEDB_INIT(init) {
    LOG_INFO("C++ SDK test module initialized");
    
    // Log module identity
    auto module_id = ctx.identity();
    char id_buf[128];
    snprintf(id_buf, sizeof(id_buf), "Module identity starts with: %02x%02x%02x%02x",
             module_id.data[0], module_id.data[1], module_id.data[2], module_id.data[3]);
    LOG_INFO(id_buf);
}

// =============================================================================
// MODULE EXPORTS
// =============================================================================

// Build the complete module definition with all types and tables
std::vector<uint8_t> build_module_definition() {
    std::vector<uint8_t> module_bytes;
    spacetimedb::BsatnWriter writer(module_bytes);
    
    // RawModuleDef::V9 tag
    writer.write_u8(1);
    
    // This would be extremely long to write out manually for all tables
    // For now, we'll create a minimal version
    
    // 1. typespace: Empty for simplicity
    writer.write_vec_len(0); // types
    writer.write_vec_len(0); // names
    
    // 2. tables: Empty for now (would need to add all table definitions)
    writer.write_vec_len(0);
    
    // 3. reducers: Add a few key reducers
    writer.write_vec_len(5); // 5 reducers for testing
    
    // Reducer 0: init
    writer.write_string("init");
    writer.write_vec_len(0); // params
    writer.write_u8(0); // Some(Init)
    writer.write_u8(0); // Init lifecycle
    
    // Reducer 1: no_op_succeeds
    writer.write_string("no_op_succeeds");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // Reducer 2: insert_one_u8
    writer.write_string("insert_one_u8");
    writer.write_vec_len(0); // params (would need arg in real impl)
    writer.write_u8(1); // None lifecycle
    
    // Reducer 3: insert_caller_one_identity
    writer.write_string("insert_caller_one_identity");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // Reducer 4: insert_call_timestamp
    writer.write_string("insert_call_timestamp");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // 4. types: Empty
    writer.write_vec_len(0);
    
    // 5. misc_exports: Empty
    writer.write_vec_len(0);
    
    // 6. row_level_security: Empty
    writer.write_vec_len(0);
    
    return module_bytes;
}

extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    auto module_def = build_module_definition();
    
    size_t total_size = module_def.size();
    size_t written = 0;
    
    while (written < total_size) {
        size_t chunk_size = std::min(size_t(1024), total_size - written);
        size_t write_size = chunk_size;
        
        uint16_t result = bytes_sink_write(description, 
                                          module_def.data() + written, 
                                          &write_size);
        
        if (result != 0) {
            break;
        }
        
        written += write_size;
    }
}

extern "C" __attribute__((export_name("__call_reducer__"))) 
int16_t __call_reducer__(uint32_t id, 
                        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                        uint64_t conn_id_0, uint64_t conn_id_1, 
                        uint64_t timestamp,
                        uint32_t args, uint32_t error) {
    try {
        // Construct identity from parameters
        auto sender_identity = identity_from_params(sender_0, sender_1, sender_2, sender_3);
        
        // Construct connection ID if valid
        std::optional<spacetimedb::ConnectionId> conn_id;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            conn_id = spacetimedb::ConnectionId(conn_id_0, conn_id_1);
        }
        
        // Create reducer context
        spacetimedb::ReducerContext ctx(spacetimedb::get_module_db(), 
                                       sender_identity, 
                                       timestamp, 
                                       conn_id);
        
        // Dispatch to reducers
        if (!spacetimedb::ReducerDispatcher::instance().call_reducer(id, ctx, args)) {
            return 1; // Unknown reducer
        }
        
        return 0; // Success
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error in __call_reducer__: ") + e.what());
        return 1;
    }
}