#include "test_common.h"      // For ASSERT macros, host stubs, print_bytes_test_common
#include "test_types.h"       // For SpacetimeDB::Test types
#include "spacetimedb/sdk/logging.h"           // For SpacetimeDB::log_info etc.
#include "spacetimedb/sdk/database.h"          // For SpacetimeDB::sdk::table_insert etc.
#include "spacetimedb/internal/Module.h"   // Updated to use new Module API
// spacetime_module_exports.h (for __describe_module__ etc.) is implicitly included via test_common.h
#include "spacetimedb/bsatn/writer.h"          // For bsatn::Writer (updated to new path style)
#include "spacetimedb/bsatn/reader.h"          // For bsatn::Reader (updated to new path style)
#include "spacetimedb/abi/abi_utils.h"         // For SpacetimeDB::Abi::Utils e.g. ManagedBytesSink

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept> // For std::out_of_range in error tests
#include <numeric>   // For std::iota if needed for vector data

// --- BSATN Reader/Writer Primitive Tests ---
void test_bsatn_primitives() {
    std::cout << "Running BSATN Primitive R/W Tests..." << std::endl;
    using namespace SpacetimeDB::Types; // For uint128_t_placeholder etc.

    bsatn::Writer writer;
    writer.write_bool(true);
    writer.write_u8(0xAB);
    writer.write_u16_le(0xABCD);
    writer.write_u32_le(0xABCDEF01);
    writer.write_u64_le(0x0123456789ABCDEFULL);
    writer.write_u128_le(uint128_t_placeholder{0x1122334455667788ULL, 0xAABBCCDDEEFF0011ULL});
    writer.write_i8(-12); // 0xF4
    writer.write_i16_le(-12345); // 0xCFC7
    writer.write_i32_le(-123456789); // 0xF8A432EB
    writer.write_i64_le(-1234567890123456789LL); // 0xF2CB5A79C949A6EB LL
    writer.write_i128_le(int128_t_placeholder{0x1122334455667788ULL, -1});
    writer.write_f32_le(123.456f);
    writer.write_f64_le(789.0123456789);
    writer.write_string("hello bsatn");
    writer.write_bytes({std::byte{0xCA}, std::byte{0xFE}});

    std::vector<std::byte> buffer = writer.take_buffer();
    bsatn::Reader reader(buffer);

    ASSERT_EQ(reader.read_bool(), true, "bool read");
    ASSERT_EQ(reader.read_u8(), 0xAB, "u8 read");
    ASSERT_EQ(reader.read_u16_le(), 0xABCD, "u16 read");
    ASSERT_EQ(reader.read_u32_le(), 0xABCDEF01, "u32 read");
    ASSERT_EQ(reader.read_u64_le(), 0x0123456789ABCDEFULL, "u64 read");
    ASSERT_EQ(reader.read_u128_le(), (uint128_t_placeholder{0x1122334455667788ULL, 0xAABBCCDDEEFF0011ULL}), "u128 read");
    ASSERT_EQ(reader.read_i8(), -12, "i8 read");
    ASSERT_EQ(reader.read_i16_le(), -12345, "i16 read");
    ASSERT_EQ(reader.read_i32_le(), -123456789, "i32 read");
    ASSERT_EQ(reader.read_i64_le(), -1234567890123456789LL, "i64 read");
    ASSERT_EQ(reader.read_i128_le(), (int128_t_placeholder{0x1122334455667788ULL, -1}), "i128 read");
    ASSERT_EQ(reader.read_f32_le(), 123.456f, "f32 read");
    ASSERT_EQ(reader.read_f64_le(), 789.0123456789, "f64 read");
    ASSERT_EQ(reader.read_string(), "hello bsatn", "string read");
    ASSERT_EQ(reader.read_bytes(), (std::vector<std::byte>{std::byte{0xCA}, std::byte{0xFE}}), "bytes read");

    ASSERT_TRUE(reader.is_eos(), "Reader EOS after all reads check");
    std::cout << "BSATN Primitive R/W Tests: SUCCESS" << std::endl;
}

void test_bsatn_error_conditions() {
    std::cout << "Running BSATN Error Condition Tests..." << std::endl;
    std::vector<std::byte> empty_buffer;
    bsatn::Reader reader1(empty_buffer);
    try { reader1.read_u8(); ASSERT_TRUE(false, "Should have thrown on read_u8 from empty buffer"); }
    catch (const std::out_of_range&) { /* Expected */ }

    std::vector<std::byte> short_buffer = {std::byte{1}};
    bsatn::Reader reader2(short_buffer);
    try { reader2.read_u32_le(); ASSERT_TRUE(false, "Should have thrown on read_u32_le from short buffer"); }
    catch (const std::out_of_range&) { /* Expected */ }

    bsatn::Writer writer_bad_str;
    writer_bad_str.write_u32_le(0xFFFFFFFF);
    std::vector<std::byte> bad_str_buf = writer_bad_str.take_buffer();
    bsatn::Reader reader_bad_str(bad_str_buf);
    try { reader_bad_str.read_string(); ASSERT_TRUE(false, "Should have thrown on too large string length"); }
    catch (const std::runtime_error&) { /* Expected, from sanity check */ }

    std::cout << "BSATN Error Condition Tests: SUCCESS (if exceptions were caught)" << std::endl;
}

// --- Macro-Generated (De)serialization Tests ---
void test_macro_serialization() {
    std::cout << "Running Macro (De)serialization Tests..." << std::endl;
    using namespace SpacetimeDB::Test;

    BasicEnum enum_orig = BasicEnum::ValTwo;
    bsatn::Writer enum_writer;
    SpacetimeDB::bsatn::serialize(enum_writer, enum_orig);
    std::vector<std::byte> enum_bytes = enum_writer.take_buffer();
    bsatn::Reader enum_reader(enum_bytes);
    BasicEnum enum_deser = SpacetimeDB::bsatn::deserialize_BasicEnum(enum_reader);
    ASSERT_EQ(enum_orig, enum_deser, "BasicEnum deserialize_BasicEnum");
    bsatn::Reader enum_reader_generic(enum_bytes);
    BasicEnum enum_deser_generic = bsatn::deserialize<BasicEnum>(enum_reader_generic);
    ASSERT_EQ(enum_orig, enum_deser_generic, "BasicEnum bsatn::deserialize<T>");

    NestedData nested_orig = {123, "Test Nested", true};
    bsatn::Writer nested_writer;
    SpacetimeDB::bsatn::serialize(nested_writer, nested_orig);
    std::vector<std::byte> nested_bytes = nested_writer.take_buffer();
    bsatn::Reader nested_reader(nested_bytes);
    NestedData nested_deser = SpacetimeDB::bsatn::deserialize_NestedData(nested_reader);
    ASSERT_EQ(nested_orig, nested_deser, "NestedData deserialize_NestedData");
    bsatn::Reader nested_reader_generic(nested_bytes);
    NestedData nested_deser_generic = bsatn::deserialize<NestedData>(nested_reader_generic);
    ASSERT_EQ(nested_orig, nested_deser_generic, "NestedData bsatn::deserialize<T>");

    ComplexType complex_orig;
    complex_orig.id_field = 999ULL;
    complex_orig.string_field = "Complex String";
    complex_orig.u128_field = {0xABC, 0xDEF};
    complex_orig.enum_field = BasicEnum::ValOne;
    complex_orig.opt_i32_field = -500;
    complex_orig.opt_string_field = "Optional string here";
    complex_orig.opt_nested_field = NestedData{777, "Optional Nested", false};
    complex_orig.opt_enum_field = BasicEnum::ValZero;
    complex_orig.vec_u8_field = {10, 20, 30};
    complex_orig.vec_string_field = {"str1", "str2"};
    complex_orig.vec_nested_field = {{1, "vn1", true}, {2, "vn2", std::nullopt}};
    complex_orig.vec_enum_field = {BasicEnum::ValOne, BasicEnum::ValTwo};
    complex_orig.vec_opt_i32_field = {std::nullopt, 42, std::nullopt, -100};

    bsatn::Writer complex_writer;
    SpacetimeDB::bsatn::serialize(complex_writer, complex_orig);
    std::vector<std::byte> complex_bytes_vec = complex_writer.take_buffer(); // Renamed to avoid clash
    print_bytes_test_common(complex_bytes_vec, "Serialized ComplexType: ");
    bsatn::Reader complex_reader(complex_bytes_vec);
    ComplexType complex_deser = SpacetimeDB::bsatn::deserialize_ComplexType(complex_reader);

    ASSERT_EQ(complex_orig.id_field, complex_deser.id_field, "ComplexType.id_field direct compare");
    ASSERT_EQ(complex_orig, complex_deser, "ComplexType (de)serialization (full object compare)");

    ComplexType complex_orig_absent_opts = complex_orig;
    complex_orig_absent_opts.opt_i32_field.reset();
    complex_orig_absent_opts.opt_string_field.reset();
    complex_orig_absent_opts.opt_nested_field.reset();
    complex_orig_absent_opts.opt_enum_field.reset();
    bsatn::Writer complex_writer_absent;
    SpacetimeDB::bsatn::serialize(complex_writer_absent, complex_orig_absent_opts);
    std::vector<std::byte> complex_bytes_absent = complex_writer_absent.take_buffer();
    print_bytes_test_common(complex_bytes_absent, "Serialized ComplexType (absent optionals): ");
    bsatn::Reader complex_reader_absent(complex_bytes_absent);
    ComplexType complex_deser_absent = SpacetimeDB::bsatn::deserialize_ComplexType(complex_reader_absent);
    ASSERT_EQ(complex_orig_absent_opts, complex_deser_absent, "ComplexType (de)serialization with absent optionals)");

    std::cout << "Macro (De)serialization Tests: SUCCESS" << std::endl;
}

// --- Reducer Dispatch Tests ---
static std::vector<std::string> g_reducer_call_log_test; // Renamed to avoid clash with test_common.h

void test_reducer_simple_impl_unit(uint32_t val1, std::string val2) {
    g_reducer_call_log_test.push_back("test_reducer_simple_impl_unit called with: " + std::to_string(val1) + ", " + val2);
}
SPACETIMEDB_REDUCER("SimpleReducerUnit", test_reducer_simple_impl_unit, {
    SPACETIMEDB_REDUCER_PARAM("val1", SpacetimeDb::CoreType::U32),
    SPACETIMEDB_REDUCER_PARAM("val2", SpacetimeDb::CoreType::String)
}, uint32_t, std::string);


void test_reducer_complex_args_impl_unit(const SpacetimeDB::Test::ReducerArgsTestStruct& s, bool b) {
    g_reducer_call_log_test.push_back("test_reducer_complex_args_impl_unit called with: " +
        s.s_arg + ", " + std::to_string(s.u_arg) + ", b=" + (b?"true":"false"));
    if(s.opt_nested_arg) {
        g_reducer_call_log_test.push_back("  opt_nested_arg: " + s.opt_nested_arg->item_name);
    }
     g_reducer_call_log_test.push_back("  vec_enum_arg size: " + std::to_string(s.vec_enum_arg.size()));
}
SPACETIMEDB_REDUCER("ComplexArgsReducerUnit", test_reducer_complex_args_impl_unit, {
    SPACETIMEDB_REDUCER_PARAM_CUSTOM("s", "TestReducerArgsStruct"),
    SPACETIMEDB_REDUCER_PARAM("b", SpacetimeDb::CoreType::Bool)
}, SpacetimeDB::Test::ReducerArgsTestStruct, bool);


void test_reducer_dispatch() {
    std::cout << "Running Reducer Dispatch Tests (Unit)..." << std::endl;
    g_reducer_call_log_test.clear();

    // Test SimpleReducerUnit
    bsatn::Writer writer_simple;
    SpacetimeDB::bsatn::serialize(writer_simple, static_cast<uint32_t>(123));
    SpacetimeDB::bsatn::serialize(writer_simple, std::string("hello_reducer_unit"));
    std::vector<std::byte> args_simple_vec = writer_simple.take_buffer(); // Renamed

    BytesSource source_simple = _bytes_source_create_from_bytes(reinterpret_cast<const uint8_t*>(args_simple_vec.data()), args_simple_vec.size());
    BytesSink err_sink_simple = _bytes_sink_create();

    // Find SimpleReducerUnit's ID (temporary method by iteration)
    uint32_t simple_reducer_id = UINT32_MAX;
    uint32_t current_id = 0;
    for(const auto& pair : SpacetimeDb::ModuleSchema::instance().reducers) {
        if (pair.first == "SimpleReducerUnit") {
            simple_reducer_id = current_id;
            break;
        }
        current_id++;
    }
    ASSERT_NE(simple_reducer_id, UINT32_MAX, "SimpleReducerUnit ID not found for dispatch test");

    int16_t status_simple = __call_reducer__(simple_reducer_id, 0,0,0,0,0,0,0, source_simple, err_sink_simple);
    ASSERT_EQ(status_simple, 0, "SimpleReducerUnit dispatch status OK");
    ASSERT_EQ(g_reducer_call_log_test.size(), 1, "SimpleReducerUnit call count");
    ASSERT_EQ(g_reducer_call_log_test[0], "test_reducer_simple_impl_unit called with: 123, hello_reducer_unit", "SimpleReducerUnit log match");
    _bytes_source_done(source_simple); // Clean up mock source/sink
    _bytes_sink_done(err_sink_simple);


    // Test ComplexArgsReducerUnit
    g_reducer_call_log_test.clear();
    SpacetimeDB::Test::ReducerArgsTestStruct complex_arg_s;
    complex_arg_s.s_arg = "complex_s_unit";
    complex_arg_s.u_arg = 98765ULL;
    complex_arg_s.opt_nested_arg = SpacetimeDB::Test::NestedData{55, "opt_nest_unit", true};
    complex_arg_s.vec_enum_arg = {SpacetimeDB::Test::BasicEnum::ValOne, SpacetimeDB::Test::BasicEnum::ValTwo};

    bsatn::Writer writer_complex;
    SpacetimeDB::bsatn::serialize(writer_complex, complex_arg_s);
    SpacetimeDB::bsatn::serialize(writer_complex, true);
    std::vector<std::byte> args_complex_vec = writer_complex.take_buffer(); // Renamed

    BytesSource source_complex = _bytes_source_create_from_bytes(reinterpret_cast<const uint8_t*>(args_complex_vec.data()), args_complex_vec.size());
    BytesSink err_sink_complex = _bytes_sink_create();

    uint32_t complex_reducer_id = UINT32_MAX;
    current_id = 0;
    for(const auto& pair : SpacetimeDb::ModuleSchema::instance().reducers) {
        if (pair.first == "ComplexArgsReducerUnit") {
            complex_reducer_id = current_id;
            break;
        }
        current_id++;
    }
    ASSERT_NE(complex_reducer_id, UINT32_MAX, "ComplexArgsReducerUnit ID not found for dispatch test");

    int16_t status_complex = __call_reducer__(complex_reducer_id, 0,0,0,0,0,0,0, source_complex, err_sink_complex);
    ASSERT_EQ(status_complex, 0, "ComplexArgsReducerUnit dispatch status OK");
    ASSERT_EQ(g_reducer_call_log_test.size(), 3, "ComplexArgsReducerUnit call count (3 log entries)");
    ASSERT_EQ(g_reducer_call_log_test[0], "test_reducer_complex_args_impl_unit called with: complex_s_unit, 98765, b=true", "ComplexArgsReducerUnit log match");
    _bytes_source_done(source_complex);
    _bytes_sink_done(err_sink_complex);

    std::cout << "Reducer Dispatch Tests (Unit): SUCCESS" << std::endl;
}

// --- ModuleDef Generation/ABI Tests ---
void test_module_def_abi() {
    std::cout << "Running ModuleDef Generation/ABI Tests (Unit)..." << std::endl;

    // Ensure schema is populated by macros in test_types.h and reducers in this file.
    // (This happens due to static initialization order when these files are linked.)

    // Use Module API to generate module definition
    SpacetimeDb::Internal::FFI::BytesSink mock_direct_sink{_bytes_sink_create()};
    SpacetimeDb::Internal::Module::__describe_module__(mock_direct_sink);
    std::vector<std::byte> direct_def_bytes = g_mock_sinks_data[mock_direct_sink.handle];
    _bytes_sink_done(mock_direct_sink.handle);
    ASSERT_TRUE(direct_def_bytes.size() > 0, "Serialized ModuleDef (direct) should not be empty.");
    print_bytes_test_common(direct_def_bytes, "Serialized ModuleDef (direct): ");

    BytesSink mock_sink = _bytes_sink_create();
    __describe_module__(mock_sink);

    std::vector<std::byte>& abi_def_bytes = g_mock_sinks_data[mock_sink.inner];
    ASSERT_EQ(abi_def_bytes.size(), direct_def_bytes.size(), "ABI __describe_module__ size matches direct serialization size.");
    if (abi_def_bytes.size() == direct_def_bytes.size()) {
         ASSERT_EQ(abi_def_bytes, direct_def_bytes, "ABI __describe_module__ data matches direct serialization data.");
    }
    _bytes_sink_done(mock_sink); // Clean up mock sink

    std::cout << "ModuleDef Generation/ABI Tests (Unit): SUCCESS" << std::endl;
}

// --- SDK Runtime Wrapper Tests ---
void test_sdk_runtime_wrappers() {
    std::cout << "Running SDK Runtime Wrapper Tests (Unit)..." << std::endl;
    g_host_log_messages.clear();
    g_host_table_ops_log.clear();

    SpacetimeDB::log_info("Testing info log via SDK (Unit).");
    SpacetimeDB::log_error("Testing error log via SDK (Unit).");
    ASSERT_TRUE(g_host_log_messages.size() >= 2, "Log messages should have been captured by host stub.");
    // Check last two messages
    if (g_host_log_messages.size() >=2) {
        ASSERT_TRUE(g_host_log_messages[g_host_log_messages.size()-2].find("[HOST STUB _log_message_abi (INFO)] Testing info log via SDK (Unit).") != std::string::npos, "Info log content check");
        ASSERT_TRUE(g_host_log_messages.back().find("[HOST STUB _log_message_abi (ERROR)] Testing error log via SDK (Unit).") != std::string::npos, "Error log content check");
    }

    SpacetimeDB::Test::NestedData row_to_insert = {222, "Insert SDK Unit"};
    // Note: spacetimedb::sdk::table_insert is in spacetimedb::sdk namespace
    bool insert_success = spacetimedb::sdk::table_insert("MyNestedTable", row_to_insert);
    ASSERT_TRUE(insert_success, "table_insert should return true on stub success.");
    ASSERT_FALSE(g_host_table_ops_log.empty(), "table_insert should log a host table op.");
    if (!g_host_table_ops_log.empty()) {
        ASSERT_TRUE(g_host_table_ops_log.back().find("table_insert Table: MyNestedTable") != std::string::npos, "table_insert op log check");
    }

    std::string pk_to_delete = "key_to_delete_unit";
    bool delete_success = spacetimedb::sdk::table_delete_by_pk<std::string>("AnotherTableUnit", pk_to_delete);
    ASSERT_TRUE(delete_success, "table_delete_by_pk should return true on stub success.");
    ASSERT_FALSE(g_host_table_ops_log.empty(), "table_delete_by_pk should log a host table op.");
     if (g_host_table_ops_log.size() >=2 ) {
        ASSERT_TRUE(g_host_table_ops_log.back().find("table_delete_by_pk Table: AnotherTableUnit") != std::string::npos, "table_delete_by_pk op log check");
    }

    std::cout << "SDK Runtime Wrapper Tests (Unit): SUCCESS" << std::endl;
}


// Main function to run all unit tests
void run_all_unit_tests() {
    std::cout << "========== Starting SDK Unit Tests ==========" << std::endl;
    test_bsatn_primitives();
    test_bsatn_error_conditions();
    test_macro_serialization();
    test_reducer_dispatch();
    test_module_def_abi();
    test_sdk_runtime_wrappers();
    std::cout << "========== All SDK Unit Tests Passed ==========" << std::endl;
}
