#include "test_common.h"      // For ASSERT macros and host stubs
#include "test_types.h"       // For SpacetimeDB::Test types
#include "spacetimedb/spacetime_sdk_runtime.h" // For SDK runtime wrappers to test
#include "spacetimedb/spacetime_module_def.h"  // For get_serialized_module_definition_bytes()
#include "spacetimedb/spacetime_module_abi.h"  // For get_spacetimedb_module_def_data/size()

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept> // For std::out_of_range in error tests

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

    ASSERT_EQ(reader.read_bool(), true, "bool");
    ASSERT_EQ(reader.read_u8(), 0xAB, "u8");
    ASSERT_EQ(reader.read_u16_le(), 0xABCD, "u16");
    ASSERT_EQ(reader.read_u32_le(), 0xABCDEF01, "u32");
    ASSERT_EQ(reader.read_u64_le(), 0x0123456789ABCDEFULL, "u64");
    ASSERT_EQ(reader.read_u128_le(), (uint128_t_placeholder{0x1122334455667788ULL, 0xAABBCCDDEEFF0011ULL}), "u128");
    ASSERT_EQ(reader.read_i8(), -12, "i8");
    ASSERT_EQ(reader.read_i16_le(), -12345, "i16");
    ASSERT_EQ(reader.read_i32_le(), -123456789, "i32");
    ASSERT_EQ(reader.read_i64_le(), -1234567890123456789LL, "i64");
    ASSERT_EQ(reader.read_i128_le(), (int128_t_placeholder{0x1122334455667788ULL, -1}), "i128");
    ASSERT_EQ(reader.read_f32_le(), 123.456f, "f32");
    ASSERT_EQ(reader.read_f64_le(), 789.0123456789, "f64");
    ASSERT_EQ(reader.read_string(), "hello bsatn", "string");
    ASSERT_EQ(reader.read_bytes(), (std::vector<std::byte>{std::byte{0xCA}, std::byte{0xFE}}), "bytes");

    ASSERT_TRUE(reader.is_eos(), "Reader EOS after all reads");
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

    // Malformed string length (too large)
    bsatn::Writer writer_bad_str;
    writer_bad_str.write_u32_le(0xFFFFFFFF); // Max uint32_t, likely too large for sanity check
    // writer_bad_str.write_bytes_raw("test", 4); // Don't need actual string data for length check
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

    // Test BasicEnum
    BasicEnum enum_orig = BasicEnum::ValTwo;
    bsatn::Writer enum_writer;
    SpacetimeDB::bsatn::serialize(enum_writer, enum_orig);
    std::vector<std::byte> enum_bytes = enum_writer.take_buffer();
    bsatn::Reader enum_reader(enum_bytes);
    BasicEnum enum_deser = SpacetimeDB::bsatn::deserialize_BasicEnum(enum_reader); // or bsatn::deserialize<BasicEnum>(enum_reader);
    ASSERT_EQ(enum_orig, enum_deser, "BasicEnum (de)serialization");

    // Test NestedData
    NestedData nested_orig = {123, "Test Nested", true};
    bsatn::Writer nested_writer;
    SpacetimeDB::bsatn::serialize(nested_writer, nested_orig);
    std::vector<std::byte> nested_bytes = nested_writer.take_buffer();
    bsatn::Reader nested_reader(nested_bytes);
    NestedData nested_deser = SpacetimeDB::bsatn::deserialize_NestedData(nested_reader);
    ASSERT_EQ(nested_orig, nested_deser, "NestedData (de)serialization");

    // Test ComplexType
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
    std::vector<std::byte> complex_bytes = complex_writer.take_buffer();
    print_bytes_test(complex_bytes, "Serialized ComplexType: ");
    bsatn::Reader complex_reader(complex_bytes);
    ComplexType complex_deser = SpacetimeDB::bsatn::deserialize_ComplexType(complex_reader);

    ASSERT_EQ(complex_orig.id_field, complex_deser.id_field, "ComplexType.id_field");
    ASSERT_EQ(complex_orig, complex_deser, "ComplexType (de)serialization (full)");

    // Test with absent optionals
    complex_orig.opt_i32_field.reset();
    complex_orig.opt_string_field.reset();
    complex_orig.opt_nested_field.reset();
    complex_orig.opt_enum_field.reset();
    bsatn::Writer complex_writer_absent;
    SpacetimeDB::bsatn::serialize(complex_writer_absent, complex_orig);
    complex_bytes = complex_writer_absent.take_buffer();
    bsatn::Reader complex_reader_absent(complex_bytes);
    complex_deser = SpacetimeDB::bsatn::deserialize_ComplexType(complex_reader_absent);
    ASSERT_EQ(complex_orig, complex_deser, "ComplexType (de)serialization with absent optionals)");


    std::cout << "Macro (De)serialization Tests: SUCCESS" << std::endl;
}

// --- Reducer Dispatch Tests ---
static std::vector<std::string> g_reducer_call_log; // Global to check side effects

void test_reducer_simple_impl(uint32_t val1, std::string val2) {
    g_reducer_call_log.push_back("test_reducer_simple_impl called with: " + std::to_string(val1) + ", " + val2);
}
SPACETIMEDB_REDUCER("SimpleReducer", test_reducer_simple_impl, {
    SPACETIMEDB_REDUCER_PARAM("val1", SpacetimeDb::CoreType::U32),
    SPACETIMEDB_REDUCER_PARAM("val2", SpacetimeDb::CoreType::String)
}, uint32_t, std::string);


void test_reducer_complex_args_impl(const SpacetimeDB::Test::ReducerArgsTestStruct& s, bool b) {
    g_reducer_call_log.push_back("test_reducer_complex_args_impl called with: " +
        s.s_arg + ", " + std::to_string(s.u_arg) + ", b=" + (b?"true":"false"));
    if(s.opt_nested_arg) {
        g_reducer_call_log.push_back("  opt_nested_arg: " + s.opt_nested_arg->item_name);
    }
     g_reducer_call_log.push_back("  vec_enum_arg size: " + std::to_string(s.vec_enum_arg.size()));
}
SPACETIMEDB_REDUCER("ComplexArgsReducer", test_reducer_complex_args_impl, {
    SPACETIMEDB_REDUCER_PARAM_CUSTOM("s", "TestReducerArgsStruct"),
    SPACETIMEDB_REDUCER_PARAM("b", SpacetimeDb::CoreType::Bool)
}, SpacetimeDB::Test::ReducerArgsTestStruct, bool);


void test_reducer_dispatch() {
    std::cout << "Running Reducer Dispatch Tests..." << std::endl;
    g_reducer_call_log.clear();

    // Test SimpleReducer
    bsatn::Writer writer_simple;
    SpacetimeDB::bsatn::serialize(writer_simple, static_cast<uint32_t>(123));
    SpacetimeDB::bsatn::serialize(writer_simple, std::string("hello_reducer"));
    std::vector<std::byte> args_simple = writer_simple.take_buffer();
    _spacetimedb_dispatch_reducer("SimpleReducer", 13, reinterpret_cast<const unsigned char*>(args_simple.data()), args_simple.size());
    ASSERT_EQ(g_reducer_call_log.size(), 1, "SimpleReducer call count");
    ASSERT_EQ(g_reducer_call_log[0], "test_reducer_simple_impl called with: 123, hello_reducer", "SimpleReducer log match");

    // Test ComplexArgsReducer
    g_reducer_call_log.clear();
    SpacetimeDB::Test::ReducerArgsTestStruct complex_arg_s;
    complex_arg_s.s_arg = "complex_s";
    complex_arg_s.u_arg = 98765ULL;
    complex_arg_s.opt_nested_arg = SpacetimeDB::Test::NestedData{55, "opt_nest", true};
    complex_arg_s.vec_enum_arg = {SpacetimeDB::Test::BasicEnum::ValOne, SpacetimeDB::Test::BasicEnum::ValTwo};

    bsatn::Writer writer_complex;
    SpacetimeDB::bsatn::serialize(writer_complex, complex_arg_s);
    SpacetimeDB::bsatn::serialize(writer_complex, true); // bool b
    std::vector<std::byte> args_complex = writer_complex.take_buffer();
    _spacetimedb_dispatch_reducer("ComplexArgsReducer", 18, reinterpret_cast<const unsigned char*>(args_complex.data()), args_complex.size());

    ASSERT_EQ(g_reducer_call_log.size(), 3, "ComplexArgsReducer call count (3 log entries)"); // Name, opt_nested, vec_enum_size
    ASSERT_EQ(g_reducer_call_log[0], "test_reducer_complex_args_impl called with: complex_s, 98765, b=true", "ComplexArgsReducer log match");
    ASSERT_EQ(g_reducer_call_log[1], "  opt_nested_arg: opt_nest", "ComplexArgsReducer opt_nested log");
    ASSERT_EQ(g_reducer_call_log[2], "  vec_enum_arg size: 2", "ComplexArgsReducer vec_enum_size log");

    std::cout << "Reducer Dispatch Tests: SUCCESS" << std::endl;
}

// --- ModuleDef Generation/ABI Tests ---
void test_module_def_abi() {
    std::cout << "Running ModuleDef Generation/ABI Tests..." << std::endl;
    // Note: This test relies on the types registered by test_types.h macros
    // (BasicEnum, NestedData, ComplexType, ReducerArgsTestStruct)
    // and reducers registered in this file (SimpleReducer, ComplexArgsReducer)
    // being part of the global SpacetimeDb::ModuleSchema::instance().

    std::vector<std::byte> module_def_bytes = SpacetimeDB::Internal::get_serialized_module_definition_bytes();
    ASSERT_TRUE(module_def_bytes.size() > 0, "Serialized ModuleDef should not be empty.");
    print_bytes_test(module_def_bytes, "Serialized ModuleDef: ");

    const unsigned char* abi_data = get_spacetimedb_module_def_data();
    int abi_size = get_spacetimedb_module_def_size();

    ASSERT_EQ(static_cast<size_t>(abi_size), module_def_bytes.size(), "ABI size matches direct serialization size.");
    ASSERT_TRUE(abi_data != nullptr || abi_size == 0, "ABI data ptr valid if size > 0");
    if (abi_size > 0) {
         bool match = true;
        for(int i=0; i<abi_size; ++i) {
            if(abi_data[i] != static_cast<unsigned char>(module_def_bytes[i])) {
                match = false;
                break;
            }
        }
        ASSERT_TRUE(match, "ABI data matches direct serialization data.");
    }
    std::cout << "ModuleDef Generation/ABI Tests: SUCCESS" << std::endl;
}

// --- SDK Runtime Wrapper Tests ---
void test_sdk_runtime_wrappers() {
    std::cout << "Running SDK Runtime Wrapper Tests..." << std::endl;
    g_host_log_messages.clear(); // Clear global log capture from test_common.h
    g_host_table_ops_log.clear();

    SpacetimeDB::log_info("Testing info log via SDK.");
    SpacetimeDB::log_error("Testing error log via SDK.");
    ASSERT_TRUE(g_host_log_messages.size() >= 2, "Log messages should have been captured by host stub.");
    if (g_host_log_messages.size() >=2) {
        ASSERT_TRUE(g_host_log_messages[g_host_log_messages.size()-2].find("[HOST STUB LOG (INFO)] Testing info log via SDK.") != std::string::npos, "Info log content check");
        ASSERT_TRUE(g_host_log_messages.back().find("[HOST STUB LOG (ERROR)] Testing error log via SDK.") != std::string::npos, "Error log content check");
    }

    SpacetimeDB::Test::NestedData row_to_insert = {111, "Insert Me"};
    bool insert_success = SpacetimeDB::table_insert("MyNestedTable", row_to_insert);
    ASSERT_TRUE(insert_success, "table_insert should return true on stub success.");
    ASSERT_FALSE(g_host_table_ops_log.empty(), "table_insert should log a host table op.");
    if (!g_host_table_ops_log.empty()) {
        ASSERT_TRUE(g_host_table_ops_log.back().find("INSERT Table: MyNestedTable") != std::string::npos, "table_insert op log check");
    }

    std::string pk_to_delete = "key_to_delete";
    bool delete_success = SpacetimeDB::table_delete_by_pk<std::string>("AnotherTable", pk_to_delete);
    ASSERT_TRUE(delete_success, "table_delete_by_pk should return true on stub success.");
    ASSERT_FALSE(g_host_table_ops_log.empty(), "table_delete_by_pk should log a host table op.");
     if (g_host_table_ops_log.size() >=2 ) { // check last entry
        ASSERT_TRUE(g_host_table_ops_log.back().find("DELETE_BY_PK Table: AnotherTable") != std::string::npos, "table_delete_by_pk op log check");
    }

    std::cout << "SDK Runtime Wrapper Tests: SUCCESS" << std::endl;
}


// Main function to run all unit tests
void run_all_unit_tests() {
    std::cout << "========== Starting SDK Unit Tests ==========" << std::endl;
    test_bsatn_primitives();
    test_bsatn_error_conditions();
    test_macro_serialization();
    test_reducer_dispatch();    // This also registers reducers, must happen before module_def test if schema is shared
    test_module_def_abi();      // Relies on schema being populated by previous tests/macros
    test_sdk_runtime_wrappers();
    std::cout << "========== All SDK Unit Tests Passed ==========" << std::endl;
}
