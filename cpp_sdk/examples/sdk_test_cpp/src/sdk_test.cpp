#include "sdk_test.h"
#include <variant> // Required for std::visit

// BSATN serialization/deserialization for EnumWithPayload
namespace sdk_test_cpp {

void EnumWithPayload::bsatn_serialize(SpacetimeDB::bsatn::Writer& writer) const {
    // Write the tag first
    SpacetimeDB::bsatn::serialize(writer, tag); // Assumes EnumWithPayloadTag has a serialize function

    // Then write the active variant's value
    std::visit([&writer](const auto& arg) {
        // We need to ensure that 'arg' can be serialized.
        // This relies on SpacetimeDB::bsatn::serialize being overloaded or templated
        // for all types within the std::variant.
        SpacetimeDB::bsatn::serialize(writer, arg);
    }, value);
}

void EnumWithPayload::bsatn_deserialize(SpacetimeDB::bsatn::Reader& reader) {
    // Read the tag first
    tag = SpacetimeDB::bsatn::deserialize<EnumWithPayloadTag>(reader); // Assumes EnumWithPayloadTag has a deserialize function

    // Then read into the appropriate variant member based on the tag
    switch (tag) {
        case EnumWithPayloadTag::TagU8:
            value.emplace<uint8_t>(SpacetimeDB::bsatn::deserialize<uint8_t>(reader));
            break;
        case EnumWithPayloadTag::TagU16:
            value.emplace<uint16_t>(SpacetimeDB::bsatn::deserialize<uint16_t>(reader));
            break;
        case EnumWithPayloadTag::TagU32:
            value.emplace<uint32_t>(SpacetimeDB::bsatn::deserialize<uint32_t>(reader));
            break;
        case EnumWithPayloadTag::TagU64:
            value.emplace<uint64_t>(SpacetimeDB::bsatn::deserialize<uint64_t>(reader));
            break;
        case EnumWithPayloadTag::TagU128:
            value.emplace<spacetimedb::sdk::u128>(SpacetimeDB::bsatn::deserialize<spacetimedb::sdk::u128>(reader));
            break;
        case EnumWithPayloadTag::TagU256:
            value.emplace<spacetimedb::sdk::u256>(SpacetimeDB::bsatn::deserialize<spacetimedb::sdk::u256>(reader));
            break;
        case EnumWithPayloadTag::TagI8:
            value.emplace<int8_t>(SpacetimeDB::bsatn::deserialize<int8_t>(reader));
            break;
        case EnumWithPayloadTag::TagI16:
            value.emplace<int16_t>(SpacetimeDB::bsatn::deserialize<int16_t>(reader));
            break;
        case EnumWithPayloadTag::TagI32:
            value.emplace<int32_t>(SpacetimeDB::bsatn::deserialize<int32_t>(reader));
            break;
        case EnumWithPayloadTag::TagI64:
            value.emplace<int64_t>(SpacetimeDB::bsatn::deserialize<int64_t>(reader));
            break;
        case EnumWithPayloadTag::TagI128:
            value.emplace<spacetimedb::sdk::i128>(SpacetimeDB::bsatn::deserialize<spacetimedb::sdk::i128>(reader));
            break;
        case EnumWithPayloadTag::TagI256:
            value.emplace<spacetimedb::sdk::i256>(SpacetimeDB::bsatn::deserialize<spacetimedb::sdk::i256>(reader));
            break;
        case EnumWithPayloadTag::TagBool:
            value.emplace<bool>(SpacetimeDB::bsatn::deserialize<bool>(reader));
            break;
        case EnumWithPayloadTag::TagF32:
            value.emplace<float>(SpacetimeDB::bsatn::deserialize<float>(reader));
            break;
        case EnumWithPayloadTag::TagF64:
            value.emplace<double>(SpacetimeDB::bsatn::deserialize<double>(reader));
            break;
        case EnumWithPayloadTag::TagStr:
            value.emplace<std::string>(SpacetimeDB::bsatn::deserialize<std::string>(reader));
            break;
        case EnumWithPayloadTag::TagIdentity:
            value.emplace<spacetimedb::sdk::Identity>(SpacetimeDB::bsatn::deserialize<spacetimedb::sdk::Identity>(reader));
            break;
        case EnumWithPayloadTag::TagConnectionId:
            value.emplace<spacetimedb::sdk::ConnectionId>(SpacetimeDB::bsatn::deserialize<spacetimedb::sdk::ConnectionId>(reader));
            break;
        case EnumWithPayloadTag::TagTimestamp:
            value.emplace<spacetimedb::sdk::Timestamp>(SpacetimeDB::bsatn::deserialize<spacetimedb::sdk::Timestamp>(reader));
            break;
        case EnumWithPayloadTag::TagBytes:
            value.emplace<std::vector<uint8_t>>(SpacetimeDB::bsatn::deserialize<std::vector<uint8_t>>(reader));
            break;
        case EnumWithPayloadTag::TagInts:
             value.emplace<std::vector<int32_t>>(SpacetimeDB::bsatn::deserialize<std::vector<int32_t>>(reader));
            break;
        case EnumWithPayloadTag::TagStrings:
            value.emplace<std::vector<std::string>>(SpacetimeDB::bsatn::deserialize<std::vector<std::string>>(reader));
            break;
        case EnumWithPayloadTag::TagSimpleEnums:
            value.emplace<std::vector<SimpleEnum>>(SpacetimeDB::bsatn::deserialize<std::vector<SimpleEnum>>(reader));
            break;
        // Default case could throw an error or handle unknown tags if necessary
        default:
            // Handle error: unknown tag
            // For now, this might leave 'value' in a valueless_by_exception state or default constructed
            // depending on variant's behavior with unhandled emplace.
            // Consider throwing an exception or logging.
            break;
    }
}

} // namespace sdk_test_cpp

// Table and Primary Key Registrations
namespace sdk_test_cpp {

// OneU8
SPACETIMEDB_TABLE(sdk_test_cpp::OneU8Row, "one_u8", true);
SPACETIMEDB_PRIMARY_KEY("one_u8", "n");

// OneU16
SPACETIMEDB_TABLE(sdk_test_cpp::OneU16Row, "one_u16", true);
SPACETIMEDB_PRIMARY_KEY("one_u16", "n");

// OneU32
SPACETIMEDB_TABLE(sdk_test_cpp::OneU32Row, "one_u32", true);
SPACETIMEDB_PRIMARY_KEY("one_u32", "n");

// OneU64
SPACETIMEDB_TABLE(sdk_test_cpp::OneU64Row, "one_u64", true);
SPACETIMEDB_PRIMARY_KEY("one_u64", "n");

// OneU128
SPACETIMEDB_TABLE(sdk_test_cpp::OneU128Row, "one_u128", true);
SPACETIMEDB_PRIMARY_KEY("one_u128", "n");

// OneU256
SPACETIMEDB_TABLE(sdk_test_cpp::OneU256Row, "one_u256", true);
SPACETIMEDB_PRIMARY_KEY("one_u256", "n");

// OneI8
SPACETIMEDB_TABLE(sdk_test_cpp::OneI8Row, "one_i8", true);
SPACETIMEDB_PRIMARY_KEY("one_i8", "n");

// OneI16
SPACETIMEDB_TABLE(sdk_test_cpp::OneI16Row, "one_i16", true);
SPACETIMEDB_PRIMARY_KEY("one_i16", "n");

// OneI32
SPACETIMEDB_TABLE(sdk_test_cpp::OneI32Row, "one_i32", true);
SPACETIMEDB_PRIMARY_KEY("one_i32", "n");

// OneI64
SPACETIMEDB_TABLE(sdk_test_cpp::OneI64Row, "one_i64", true);
SPACETIMEDB_PRIMARY_KEY("one_i64", "n");

// OneI128
SPACETIMEDB_TABLE(sdk_test_cpp::OneI128Row, "one_i128", true);
SPACETIMEDB_PRIMARY_KEY("one_i128", "n");

// OneI256
SPACETIMEDB_TABLE(sdk_test_cpp::OneI256Row, "one_i256", true);
SPACETIMEDB_PRIMARY_KEY("one_i256", "n");

// OneBool
SPACETIMEDB_TABLE(sdk_test_cpp::OneBoolRow, "one_bool", true);
SPACETIMEDB_PRIMARY_KEY("one_bool", "n");

// OneF32
SPACETIMEDB_TABLE(sdk_test_cpp::OneF32Row, "one_f32", true);
SPACETIMEDB_PRIMARY_KEY("one_f32", "n");

// OneF64
SPACETIMEDB_TABLE(sdk_test_cpp::OneF64Row, "one_f64", true);
SPACETIMEDB_PRIMARY_KEY("one_f64", "n");

// OneString
SPACETIMEDB_TABLE(sdk_test_cpp::OneStringRow, "one_string", true);
SPACETIMEDB_PRIMARY_KEY("one_string", "n");

// OneIdentity
SPACETIMEDB_TABLE(sdk_test_cpp::OneIdentityRow, "one_identity", true);
SPACETIMEDB_PRIMARY_KEY("one_identity", "n");

// OneConnectionId
SPACETIMEDB_TABLE(sdk_test_cpp::OneConnectionIdRow, "one_connection_id", true);
SPACETIMEDB_PRIMARY_KEY("one_connection_id", "n");

// OneTimestamp
SPACETIMEDB_TABLE(sdk_test_cpp::OneTimestampRow, "one_timestamp", true);
SPACETIMEDB_PRIMARY_KEY("one_timestamp", "n");

// OneSimpleEnum
SPACETIMEDB_TABLE(sdk_test_cpp::OneSimpleEnumRow, "one_simple_enum", true);
SPACETIMEDB_PRIMARY_KEY("one_simple_enum", "n");

// OneEnumWithPayload
SPACETIMEDB_TABLE(sdk_test_cpp::OneEnumWithPayloadRow, "one_enum_with_payload", true);
SPACETIMEDB_PRIMARY_KEY("one_enum_with_payload", "n"); // PK on 'n' which is EnumWithPayload struct; might need specific handling if not directly comparable/indexable.

// OneUnitStruct
SPACETIMEDB_TABLE(sdk_test_cpp::OneUnitStructRow, "one_unit_struct", true);
// No SPACETIMEDB_PRIMARY_KEY for OneUnitStructRow as it has no fields.

// OneByteStruct
SPACETIMEDB_TABLE(sdk_test_cpp::OneByteStructRow, "one_byte_struct", true);
SPACETIMEDB_PRIMARY_KEY("one_byte_struct", "s");

// OneEveryPrimitiveStruct
SPACETIMEDB_TABLE(sdk_test_cpp::OneEveryPrimitiveStructRow, "one_every_primitive_struct", true);
SPACETIMEDB_PRIMARY_KEY("one_every_primitive_struct", "s");

// OneEveryVecStruct
SPACETIMEDB_TABLE(sdk_test_cpp::OneEveryVecStructRow, "one_every_vec_struct", true);
SPACETIMEDB_PRIMARY_KEY("one_every_vec_struct", "s");


// "Vec*" Series Tables
SPACETIMEDB_TABLE(sdk_test_cpp::VecU8Row, "vec_u8", true);
SPACETIMEDB_PRIMARY_KEY("vec_u8", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::VecStringRow, "vec_string", true);
SPACETIMEDB_PRIMARY_KEY("vec_string", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::VecSimpleEnumRow, "vec_simple_enum", true);
SPACETIMEDB_PRIMARY_KEY("vec_simple_enum", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::VecEveryPrimitiveStructRow, "vec_every_primitive_struct", true);
SPACETIMEDB_PRIMARY_KEY("vec_every_primitive_struct", "s");


// "Option*" Series Tables
SPACETIMEDB_TABLE(sdk_test_cpp::OptionI32Row, "option_i32", true);
SPACETIMEDB_PRIMARY_KEY("option_i32", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::OptionStringRow, "option_string", true);
SPACETIMEDB_PRIMARY_KEY("option_string", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::OptionIdentityRow, "option_identity", true);
SPACETIMEDB_PRIMARY_KEY("option_identity", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::OptionSimpleEnumRow, "option_simple_enum", true);
SPACETIMEDB_PRIMARY_KEY("option_simple_enum", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::OptionEveryPrimitiveStructRow, "option_every_primitive_struct", true);
SPACETIMEDB_PRIMARY_KEY("option_every_primitive_struct", "s");

SPACETIMEDB_TABLE(sdk_test_cpp::OptionVecOptionI32Row, "option_vec_option_i32", true);
SPACETIMEDB_PRIMARY_KEY("option_vec_option_i32", "v");


// "Unique*" Series Tables
SPACETIMEDB_TABLE(sdk_test_cpp::UniqueU8Row, "unique_u8", true);
SPACETIMEDB_PRIMARY_KEY("unique_u8", "row_id");

SPACETIMEDB_TABLE(sdk_test_cpp::UniqueStringRow, "unique_string", true);
SPACETIMEDB_PRIMARY_KEY("unique_string", "row_id");

SPACETIMEDB_TABLE(sdk_test_cpp::UniqueIdentityRow, "unique_identity", true);
SPACETIMEDB_PRIMARY_KEY("unique_identity", "row_id");

SPACETIMEDB_TABLE(sdk_test_cpp::UniqueConnectionIdRow, "unique_connection_id", true);
SPACETIMEDB_PRIMARY_KEY("unique_connection_id", "row_id");


// "Pk*" Series Tables
SPACETIMEDB_TABLE(sdk_test_cpp::PkU8Row, "pk_u8", true);
SPACETIMEDB_PRIMARY_KEY("pk_u8", "n");

SPACETIMEDB_TABLE(sdk_test_cpp::PkStringRow, "pk_string", true);
SPACETIMEDB_PRIMARY_KEY("pk_string", "s");

SPACETIMEDB_TABLE(sdk_test_cpp::PkIdentityRow, "pk_identity", true);
SPACETIMEDB_PRIMARY_KEY("pk_identity", "i");

SPACETIMEDB_TABLE(sdk_test_cpp::PkSimpleEnumRow, "pk_simple_enum", true);
SPACETIMEDB_PRIMARY_KEY("pk_simple_enum", "e");

SPACETIMEDB_TABLE(sdk_test_cpp::PkU32TwoRow, "pk_u32_two", true);
SPACETIMEDB_PRIMARY_KEY("pk_u32_two", "a"); // As per simplified approach, 'a' is PK.


// Remaining Complex and Manually Defined Tables

// LargeTableRow
SPACETIMEDB_TABLE(sdk_test_cpp::LargeTableRow, "large_table", true, "");
SPACETIMEDB_PRIMARY_KEY("large_table", "row_id");

// TableHoldsTableRow
SPACETIMEDB_TABLE(sdk_test_cpp::TableHoldsTableRow, "table_holds_table", true, "");
SPACETIMEDB_PRIMARY_KEY("table_holds_table", "row_id");

// ScheduledTableRow
SPACETIMEDB_TABLE(sdk_test_cpp::ScheduledTableRow, "scheduled_table", true, "send_scheduled_message");
SPACETIMEDB_PRIMARY_KEY("scheduled_table", "scheduled_id");

// IndexedTableRow
SPACETIMEDB_TABLE(sdk_test_cpp::IndexedTableRow, "indexed_table", false, "");
SPACETIMEDB_PRIMARY_KEY("indexed_table", "row_id");
SPACETIMEDB_INDEX("indexed_table", "idx_player_id", { "player_id" });

// IndexedTable2Row
SPACETIMEDB_TABLE(sdk_test_cpp::IndexedTable2Row, "indexed_table_2", false, "");
SPACETIMEDB_PRIMARY_KEY("indexed_table_2", "row_id");
SPACETIMEDB_INDEX("indexed_table_2", "player_id_snazz_index", { "player_id", "player_snazz" });

// BTreeU32Row
SPACETIMEDB_TABLE(sdk_test_cpp::BTreeU32Row, "btree_u32", true, "");
SPACETIMEDB_PRIMARY_KEY("btree_u32", "row_id");
SPACETIMEDB_INDEX("btree_u32", "idx_n", { "n" });

// UsersRow
SPACETIMEDB_TABLE(sdk_test_cpp::UsersRow, "users", true, "");
SPACETIMEDB_PRIMARY_KEY("users", "identity");

// IndexedSimpleEnumRow
SPACETIMEDB_TABLE(sdk_test_cpp::IndexedSimpleEnumRow, "indexed_simple_enum", true, "");
SPACETIMEDB_PRIMARY_KEY("indexed_simple_enum", "row_id");
SPACETIMEDB_INDEX("indexed_simple_enum", "idx_n", { "n" });

//
// Reducer Implementations
//

// --- Reducers generated by define_tables! ---

// insert_one_u8
void insert_one_u8_impl(spacetimedb::sdk::ReducerContext& ctx, uint8_t n_val) {
    OneU8Row new_row;
    new_row.n = n_val;
    ctx.db().get_table<OneU8Row>("one_u8").insert(new_row);
}
SPACETIMEDB_REDUCER_NAMED("insert_one_u8", sdk_test_cpp::insert_one_u8_impl,
    {SPACETIMEDB_REDUCER_PARAM("n", SpacetimeDb::CoreType::U8)},
    uint8_t);

// insert_pk_string
void insert_pk_string_impl(spacetimedb::sdk::ReducerContext& ctx, std::string s_val, int32_t data_val) {
    PkStringRow new_row;
    new_row.s = s_val;
    new_row.data = data_val;
    ctx.db().get_table<PkStringRow>("pk_string").insert(new_row);
}
SPACETIMEDB_REDUCER_NAMED("insert_pk_string", sdk_test_cpp::insert_pk_string_impl,
    {SPACETIMEDB_REDUCER_PARAM("s", SpacetimeDb::CoreType::String), SPACETIMEDB_REDUCER_PARAM("data", SpacetimeDb::CoreType::I32)},
    std::string, int32_t);

// update_unique_u8
// Logic: Upsert based on 'n_val'. If a row with 'n_val' exists, its 'data' is updated.
// Its 'row_id' (PK) might change in this simplified upsert (delete by old PK, insert new).
void update_unique_u8_impl(spacetimedb::sdk::ReducerContext& ctx, uint8_t n_val, int32_t data_val) {
    auto table = ctx.db().get_table<UniqueU8Row>("unique_u8");
    // Column index 1 for 'n', 0 for 'row_id' (PK)
    std::vector<UniqueU8Row> existing_rows = table.find_by_col_eq(1, n_val);

    for(const auto& old_row : existing_rows) {
        table.delete_by_col_eq(0, old_row.row_id);
    }

    UniqueU8Row new_row;
    new_row.row_id = 0; // Mark for auto-increment
    new_row.n = n_val;
    new_row.data = data_val;
    table.insert(new_row);
    // Note: new_row.row_id is now updated if insert was successful.
}
SPACETIMEDB_REDUCER_NAMED("update_unique_u8", sdk_test_cpp::update_unique_u8_impl,
    {SPACETIMEDB_REDUCER_PARAM("n_val", SpacetimeDb::CoreType::U8), SPACETIMEDB_REDUCER_PARAM("data_val", SpacetimeDb::CoreType::I32)},
    uint8_t, int32_t);

// delete_pk_string
void delete_pk_string_impl(spacetimedb::sdk::ReducerContext& ctx, std::string s_val) {
    // PK 's' is column index 0 for PkStringRow
    ctx.db().get_table<PkStringRow>("pk_string").delete_by_col_eq(0, s_val);
}
SPACETIMEDB_REDUCER_NAMED("delete_pk_string", sdk_test_cpp::delete_pk_string_impl,
    {SPACETIMEDB_REDUCER_PARAM("s_val", SpacetimeDb::CoreType::String)},
    std::string);

// --- Manually defined reducers ---

// no_op_succeeds
void no_op_succeeds_impl(spacetimedb::sdk::ReducerContext& ctx) {
    // No operation
}
SPACETIMEDB_REDUCER_NAMED("no_op_succeeds", sdk_test_cpp::no_op_succeeds_impl,
    {/* no params */},
    /* no C++ types for invoker */);

// insert_caller_one_identity
void insert_caller_one_identity_impl(spacetimedb::sdk::ReducerContext& ctx) {
    OneIdentityRow row;
    row.n = ctx.get_sender(); // Use actual sender identity
    ctx.db().get_table<OneIdentityRow>("one_identity").insert(row);
}
SPACETIMEDB_REDUCER_NAMED("insert_caller_one_identity", sdk_test_cpp::insert_caller_one_identity_impl,
    {}, /* No schema params */
    /* No C++ types for invoker, ReducerContext is implicit */);

// insert_unique_u32_update_pk_u32
// This reducer inserts a row into unique_u32 table and upserts a row in pk_u32 table.
// Assumes UniqueU32Row has fields: uint64_t row_id (PK, auto-inc), uint32_t n (unique), int32_t data.
// Assumes PkU32Row has fields: uint32_t n (PK), int32_t data.
void insert_unique_u32_update_pk_u32_impl(spacetimedb::sdk::ReducerContext& ctx, uint32_t n_val, int32_t d_unique_val, int32_t d_pk_val) {
    // Insert into unique_u32 table
    UniqueU32Row unique_row;
    unique_row.row_id = 0; // Explicitly set to 0 for auto-increment
    unique_row.n = n_val;
    unique_row.data = d_unique_val;
    ctx.db().get_table<UniqueU32Row>("unique_u32").insert(unique_row);

    // Upsert into pk_u32 table
    auto pk_u32_table = ctx.db().get_table<PkU32Row>("pk_u32");
    // 'n' is PK for PkU32Row, so its column index is 0.
    pk_u32_table.delete_by_col_eq(0, n_val);

    PkU32Row pk_row;
    pk_row.n = n_val;
    pk_row.data = d_pk_val;
    pk_u32_table.insert(pk_row);
}
SPACETIMEDB_REDUCER_NAMED(
    "insert_unique_u32_update_pk_u32",
    sdk_test_cpp::insert_unique_u32_update_pk_u32_impl,
    {
        SPACETIMEDB_REDUCER_PARAM("n_val", SpacetimeDb::CoreType::U32),
        SPACETIMEDB_REDUCER_PARAM("d_unique_val", SpacetimeDb::CoreType::I32),
        SPACETIMEDB_REDUCER_PARAM("d_pk_val", SpacetimeDb::CoreType::I32)
    },
    uint32_t, int32_t, int32_t
);

// send_scheduled_message
void send_scheduled_message_impl(spacetimedb::sdk::ReducerContext& ctx, sdk_test_cpp::ScheduledTableRow arg) {
    // Logic for the scheduled reducer. For now, it can be empty or log.
    // Example: Log the received text.
    // spacetimedb::sdk::log::info("Scheduled message received: " + arg.text);
    // (Assuming a logging mechanism like spacetimedb::sdk::log is available)
}
SPACETIMEDB_REDUCER_SCHEDULED("send_scheduled_message", sdk_test_cpp::send_scheduled_message_impl,
    {SPACETIMEDB_REDUCER_PARAM_CUSTOM("arg", "ScheduledTableRow")}, /* Schema for arg */
    sdk_test_cpp::ScheduledTableRow /* C++ type for invoker */);


} // namespace sdk_test_cpp


// Placeholder for main logic or further definitions
// int main() { return 0; } // Only if building as a native test executable initially
                         // For a WASM module, there's no main() like this.
                         // For now, this makes it a valid C++ file.
                         // It should be removed when actual WASM exports are primary.
