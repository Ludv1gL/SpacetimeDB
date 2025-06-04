/**
 * SpacetimeDB C++ SDK Test Module
 * 
 * This module provides comprehensive testing of all C++ SDK features,
 * matching the functionality of the Rust and C# sdk-test modules.
 */

#include <spacetimedb/spacetimedb.h>
#include <spacetimedb/spacetimedb_autogen.h>

using namespace spacetimedb;

// =============================================================================
// ENUMS - Testing enum support
// =============================================================================

enum class SimpleEnum : uint32_t {
    Zero = 0,
    One = 1,
    Two = 2
};

// Tagged enum (simulated with struct since C++ doesn't have discriminated unions)
struct EnumWithPayload {
    enum Tag : uint32_t {
        U8, U16, U32, U64, U128, U256,
        I8, I16, I32, I64, I128, I256,
        Bool, F32, F64, Str,
        Identity, ConnectionId, Timestamp,
        Bytes, Ints, Strings, SimpleEnums
    } tag;
    
    // Payload data
    union {
        uint8_t u8_val;
        uint16_t u16_val;
        uint32_t u32_val;
        uint64_t u64_val;
        SpacetimeDb::Types::uint128_t_placeholder u128_val;
        SpacetimeDb::Types::uint256_t_placeholder u256_val;
        int8_t i8_val;
        int16_t i16_val;
        int32_t i32_val;
        int64_t i64_val;
        SpacetimeDb::Types::int128_t_placeholder i128_val;
        SpacetimeDb::Types::int256_t_placeholder i256_val;
        bool bool_val;
        float f32_val;
        double f64_val;
    };
    
    // For complex types, we store pointers
    std::unique_ptr<std::string> str_val;
    std::unique_ptr<SpacetimeDb::sdk::Identity> identity_val;
    std::unique_ptr<SpacetimeDb::sdk::ConnectionId> connection_id_val;
    std::unique_ptr<SpacetimeDb::sdk::Timestamp> timestamp_val;
    std::unique_ptr<std::vector<uint8_t>> bytes_val;
    std::unique_ptr<std::vector<int32_t>> ints_val;
    std::unique_ptr<std::vector<std::string>> strings_val;
    std::unique_ptr<std::vector<SimpleEnum>> simple_enums_val;
};

// =============================================================================
// STRUCTS - Testing all primitive and complex types
// =============================================================================

struct UnitStruct {
    // Empty struct
};

SPACETIMEDB_REGISTER_TYPE(UnitStruct)

struct ByteStruct {
    uint8_t b;
};

SPACETIMEDB_REGISTER_TYPE(ByteStruct,
    SPACETIMEDB_FIELD(ByteStruct, b)
)

struct EveryPrimitiveStruct {
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint64_t d;
    SpacetimeDb::Types::uint128_t_placeholder e;
    SpacetimeDb::Types::uint256_t_placeholder f;
    int8_t g;
    int16_t h;
    int32_t i;
    int64_t j;
    SpacetimeDb::Types::int128_t_placeholder k;
    SpacetimeDb::Types::int256_t_placeholder l;
    bool m;
    float n;
    double o;
    std::string p;
    SpacetimeDb::sdk::Identity q;
    SpacetimeDb::sdk::ConnectionId r;
    SpacetimeDb::sdk::Timestamp s;
    SpacetimeDb::Duration t;
};

SPACETIMEDB_REGISTER_TYPE(EveryPrimitiveStruct,
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, a),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, b),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, c),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, d),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, e),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, f),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, g),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, h),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, i),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, j),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, k),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, l),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, m),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, n),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, o),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, p),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, q),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, r),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, s),
    SPACETIMEDB_FIELD(EveryPrimitiveStruct, t)
)

struct EveryVecStruct {
    std::vector<uint8_t> a;
    std::vector<uint16_t> b;
    std::vector<uint32_t> c;
    std::vector<uint64_t> d;
    std::vector<SpacetimeDb::Types::uint128_t_placeholder> e;
    std::vector<SpacetimeDb::Types::uint256_t_placeholder> f;
    std::vector<int8_t> g;
    std::vector<int16_t> h;
    std::vector<int32_t> i;
    std::vector<int64_t> j;
    std::vector<SpacetimeDb::Types::int128_t_placeholder> k;
    std::vector<SpacetimeDb::Types::int256_t_placeholder> l;
    std::vector<bool> m;
    std::vector<float> n;
    std::vector<double> o;
    std::vector<std::string> p;
    std::vector<SimpleEnum> q;
};

SPACETIMEDB_REGISTER_TYPE(EveryVecStruct,
    SPACETIMEDB_FIELD(EveryVecStruct, a),
    SPACETIMEDB_FIELD(EveryVecStruct, b),
    SPACETIMEDB_FIELD(EveryVecStruct, c),
    SPACETIMEDB_FIELD(EveryVecStruct, d),
    SPACETIMEDB_FIELD(EveryVecStruct, e),
    SPACETIMEDB_FIELD(EveryVecStruct, f),
    SPACETIMEDB_FIELD(EveryVecStruct, g),
    SPACETIMEDB_FIELD(EveryVecStruct, h),
    SPACETIMEDB_FIELD(EveryVecStruct, i),
    SPACETIMEDB_FIELD(EveryVecStruct, j),
    SPACETIMEDB_FIELD(EveryVecStruct, k),
    SPACETIMEDB_FIELD(EveryVecStruct, l),
    SPACETIMEDB_FIELD(EveryVecStruct, m),
    SPACETIMEDB_FIELD(EveryVecStruct, n),
    SPACETIMEDB_FIELD(EveryVecStruct, o),
    SPACETIMEDB_FIELD(EveryVecStruct, p),
    SPACETIMEDB_FIELD(EveryVecStruct, q)
)

// Testing field renaming
struct VecWithRenamedFields {
    std::vector<int32_t> integerList;      // Renamed to "int_list"
    std::vector<std::string> stringArray;  // Renamed to "str_array"
};

SPACETIMEDB_REGISTER_TYPE(VecWithRenamedFields,
    SPACETIMEDB_FIELD_RENAMED(VecWithRenamedFields, integerList, "int_list"),
    SPACETIMEDB_FIELD_RENAMED(VecWithRenamedFields, stringArray, "str_array")
)

// =============================================================================
// TABLES - Testing all table features
// =============================================================================

// Basic tables
SPACETIMEDB_TABLE(UnitStruct, "unit_struct", true)
SPACETIMEDB_TABLE(ByteStruct, "byte_struct", true)
SPACETIMEDB_TABLE(EveryPrimitiveStruct, "every_primitive_struct", true)
SPACETIMEDB_TABLE(EveryVecStruct, "every_vec_struct", true)

// Table with constraints
struct ConstrainedData {
    uint32_t id;
    std::string unique_field;
    int32_t positive_only;
    std::string email;
};

SPACETIMEDB_REGISTER_TYPE(ConstrainedData,
    SPACETIMEDB_FIELD(ConstrainedData, id),
    SPACETIMEDB_FIELD(ConstrainedData, unique_field),
    SPACETIMEDB_FIELD(ConstrainedData, positive_only),
    SPACETIMEDB_FIELD(ConstrainedData, email)
)

SPACETIMEDB_TABLE(ConstrainedData, "constrained_data", true)
SPACETIMEDB_PRIMARY_KEY(ConstrainedData, id)
SPACETIMEDB_UNIQUE_CONSTRAINT(ConstrainedData, unique_field)
SPACETIMEDB_CHECK_CONSTRAINT(ConstrainedData, positive_check, "positive_only > 0")

// Table with indexes
struct IndexedData {
    uint64_t id;
    std::string name;
    int32_t score;
    SpacetimeDb::sdk::Timestamp created_at;
};

SPACETIMEDB_REGISTER_TYPE(IndexedData,
    SPACETIMEDB_FIELD(IndexedData, id),
    SPACETIMEDB_FIELD(IndexedData, name),
    SPACETIMEDB_FIELD(IndexedData, score),
    SPACETIMEDB_FIELD(IndexedData, created_at)
)

SPACETIMEDB_TABLE(IndexedData, "indexed_data", true)
SPACETIMEDB_INDEX(IndexedData, idx_name, name)
SPACETIMEDB_INDEX(IndexedData, idx_score, score)
SPACETIMEDB_COMPOSITE_INDEX(IndexedData, idx_score_created, score, created_at)

// Private table with RLS
struct PrivateData {
    SpacetimeDb::sdk::Identity owner;
    std::string secret_data;
    bool is_public;
};

SPACETIMEDB_REGISTER_TYPE(PrivateData,
    SPACETIMEDB_FIELD(PrivateData, owner),
    SPACETIMEDB_FIELD(PrivateData, secret_data),
    SPACETIMEDB_FIELD(PrivateData, is_public)
)

SPACETIMEDB_TABLE(PrivateData, "private_data", false)  // Private table

// RLS policies
SPACETIMEDB_RLS_SELECT(private_data, view_own_or_public,
    rls::or_conditions({
        rls::user_owns("owner"),
        "is_public = true"
    })
)

SPACETIMEDB_RLS_INSERT(private_data, insert_own_only,
    rls::user_owns("owner")
)

SPACETIMEDB_RLS_UPDATE(private_data, update_own_only,
    rls::user_owns("owner")
)

SPACETIMEDB_RLS_DELETE(private_data, delete_own_only,
    rls::user_owns("owner")
)

// =============================================================================
// REDUCERS - Testing all reducer features
// =============================================================================

// No-argument reducer
SPACETIMEDB_REDUCER(test_empty_reducer, UserDefined, ctx)
{
    LOG_INFO("Empty reducer called");
}

// Single primitive argument
SPACETIMEDB_REDUCER(test_single_primitive, UserDefined, ctx, uint32_t value)
{
    LOG_INFO("Single primitive: " + std::to_string(value));
}

// Multiple arguments of different types
SPACETIMEDB_REDUCER(test_multiple_args, UserDefined, ctx,
    std::string name,
    uint32_t age,
    bool is_active,
    double balance)
{
    LOG_INFO("Multiple args - Name: " + name + ", Age: " + std::to_string(age));
}

// Complex type arguments
SPACETIMEDB_REDUCER(test_complex_args, UserDefined, ctx,
    SpacetimeDb::sdk::Identity identity,
    std::vector<std::string> tags,
    std::optional<uint32_t> maybe_id)
{
    LOG_INFO("Complex args - Identity: " + identity.to_string());
    LOG_INFO("Tags count: " + std::to_string(tags.size()));
    if (maybe_id.has_value()) {
        LOG_INFO("Optional ID: " + std::to_string(maybe_id.value()));
    }
}

// Testing struct arguments
SPACETIMEDB_REDUCER(test_struct_arg, UserDefined, ctx, ByteStruct byte_data)
{
    LOG_INFO("Struct arg - byte value: " + std::to_string(byte_data.b));
}

// Insert operations
SPACETIMEDB_REDUCER(insert_primitive, UserDefined, ctx, EveryPrimitiveStruct data)
{
    auto table = get_EveryPrimitiveStruct_table();
    table.insert(data);
    LOG_INFO("Inserted primitive struct");
}

SPACETIMEDB_REDUCER(insert_vec, UserDefined, ctx, EveryVecStruct data)
{
    auto table = get_EveryVecStruct_table();
    table.insert(data);
    LOG_INFO("Inserted vec struct");
}

// Query operations
SPACETIMEDB_REDUCER(query_all_primitives, UserDefined, ctx)
{
    auto table = get_EveryPrimitiveStruct_table();
    uint64_t count = 0;
    for (const auto& row : table.iter()) {
        count++;
    }
    LOG_INFO("Found " + std::to_string(count) + " primitive structs");
}

// Update operations
SPACETIMEDB_REDUCER(update_score, UserDefined, ctx, uint64_t id, int32_t new_score)
{
    auto table = get_IndexedData_table();
    for (auto& row : table.iter()) {
        if (row.id == id) {
            auto old_row = row;
            row.score = new_score;
            table.update(old_row, row);
            LOG_INFO("Updated score for ID: " + std::to_string(id));
            return;
        }
    }
    LOG_WARN("ID not found: " + std::to_string(id));
}

// Delete operations
SPACETIMEDB_REDUCER(delete_by_name, UserDefined, ctx, std::string name)
{
    auto table = get_IndexedData_table();
    uint32_t deleted = 0;
    
    // Collect rows to delete
    std::vector<IndexedData> to_delete;
    for (const auto& row : table.iter()) {
        if (row.name == name) {
            to_delete.push_back(row);
        }
    }
    
    // Delete them
    for (const auto& row : to_delete) {
        if (table.delete_row(row)) {
            deleted++;
        }
    }
    
    LOG_INFO("Deleted " + std::to_string(deleted) + " rows");
}

// =============================================================================
// LIFECYCLE REDUCERS
// =============================================================================

SPACETIMEDB_INIT(init, ctx)
{
    LOG_INFO("SDK test module initialized");
    
    // Insert initial test data
    ByteStruct byte_data{42};
    auto byte_table = get_ByteStruct_table();
    byte_table.insert(byte_data);
    
    // Create indexed data
    IndexedData indexed{
        .id = 1,
        .name = "Test Entry",
        .score = 100,
        .created_at = SpacetimeDb::sdk::Timestamp::now()
    };
    auto indexed_table = get_IndexedData_table();
    indexed_table.insert(indexed);
}

SPACETIMEDB_CLIENT_CONNECTED(on_connect, ctx)
{
    LOG_INFO("Client connected: " + ctx.sender.to_string());
}

SPACETIMEDB_CLIENT_DISCONNECTED(on_disconnect, ctx)
{
    LOG_INFO("Client disconnected: " + ctx.sender.to_string());
}

// =============================================================================
// SCHEDULED REDUCER
// =============================================================================

struct ScheduledData {
    uint64_t id;
    SpacetimeDb::sdk::Timestamp last_run;
    uint32_t run_count;
};

SPACETIMEDB_REGISTER_TYPE(ScheduledData,
    SPACETIMEDB_FIELD(ScheduledData, id),
    SPACETIMEDB_FIELD(ScheduledData, last_run),
    SPACETIMEDB_FIELD(ScheduledData, run_count)
)

SPACETIMEDB_SCHEDULED_TABLE(ScheduledData, "scheduled_data", true, scheduled_task)

SPACETIMEDB_SCHEDULED(scheduled_task, Duration::from_seconds(10), ctx)
{
    LOG_INFO("Scheduled task running");
    
    auto table = get_ScheduledData_table();
    ScheduledData data{
        .id = 0,
        .last_run = SpacetimeDb::sdk::Timestamp::now(),
        .run_count = 0
    };
    
    // Update or insert
    bool found = false;
    for (auto& row : table.iter()) {
        if (row.id == 0) {
            auto old_row = row;
            row.last_run = data.last_run;
            row.run_count++;
            table.update(old_row, row);
            found = true;
            break;
        }
    }
    
    if (!found) {
        table.insert(data);
    }
}

// =============================================================================
// MODULE METADATA
// =============================================================================

SPACETIMEDB_MODULE_METADATA(
    "sdk-test-cpp",
    "SpacetimeDB",
    "Comprehensive C++ SDK test module",
    "Apache-2.0"
)

SPACETIMEDB_MODULE_VERSION(1, 0, 0)