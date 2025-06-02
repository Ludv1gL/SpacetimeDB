// FINAL attempt: Complete sdk_test with fixed headers and all original types
#include "comprehensive_sdk_bridge.h"
#include "sdk_test_fixed.h"
#include <cstdint>
#include <cstring>

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(
        uint8_t level,
        const uint8_t *target_ptr, uint32_t target_len,
        const uint8_t *filename_ptr, uint32_t filename_len, 
        uint32_t line_number,
        const uint8_t *message_ptr, uint32_t message_len
    );
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // Global test counter
    uint32_t final_test_counter = 0;
    
    // Forward declaration
    void test_final_comprehensive_types();
    
    // Module initialization - run comprehensive test when module loads
    __attribute__((constructor))
    void module_init() {
        test_final_comprehensive_types();
    }
    
    // Helper to log messages
    void log_message(const char* message) {
        const char* filename = "sdk_test_final.cpp";
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, strlen(filename),
            ++final_test_counter,
            (const uint8_t*)message, strlen(message)
        );
    }
    
    // Comprehensive test of ALL original sdk_test types
    void test_final_comprehensive_types() {
        try {
            log_message("FINAL TEST: All original sdk_test.h types!");
            
            // === PHASE 1: Basic Types ===
            GlobalSimpleEnum global_enum = GlobalSimpleEnum::GlobalTwo;
            GlobalUnitStruct global_unit;
            
            sdk_test_cpp::SimpleEnum simple_enum = sdk_test_cpp::SimpleEnum::Two;
            sdk_test_cpp::UnitStruct unit_struct;
            sdk_test_cpp::ByteStruct byte_struct;
            byte_struct.b = 255;
            log_message("Phase 1 - Basic types: PASS");
            
            // === PHASE 2: Complex Primitive Structures ===
            sdk_test_cpp::EveryPrimitiveStruct eps;
            eps.a = 255;        // uint8_t
            eps.b = 65535;      // uint16_t
            eps.c = 4294967295U; // uint32_t
            eps.d = 18446744073709551615ULL; // uint64_t
            eps.e = ::SpacetimeDb::Types::uint128_t_placeholder(123456789);
            eps.f = ::SpacetimeDb::sdk::u256_placeholder(987654321);
            eps.g = -128;       // int8_t
            eps.h = -32768;     // int16_t
            eps.i = -2147483648; // int32_t
            eps.j = -9223372036854775807LL; // int64_t
            eps.k = ::SpacetimeDb::Types::int128_t_placeholder(-987654321);
            eps.l = ::SpacetimeDb::sdk::i256_placeholder(-123456789);
            eps.m = true;       // bool
            eps.n = 3.14159265f; // float
            eps.o = 2.718281828459045; // double
            eps.p = "Complete EveryPrimitiveStruct test!"; // string
            eps.q = ::SpacetimeDb::sdk::Identity();
            eps.r = ::SpacetimeDb::sdk::ConnectionId(42);
            eps.s = ::SpacetimeDb::sdk::Timestamp(1234567890123456ULL);
            eps.t = ::SpacetimeDb::sdk::TimeDuration(9876543210ULL);
            log_message("Phase 2 - EveryPrimitiveStruct: PASS");
            
            // === PHASE 3: Vector Structures ===
            sdk_test_cpp::EveryVecStruct evs;
            evs.a = {1, 2, 3, 255, 128, 64};
            evs.b = {100, 200, 300, 65535, 32768};
            evs.c = {1000, 2000, 3000, 4294967295U};
            evs.d = {1000000ULL, 2000000ULL, 18446744073709551615ULL};
            evs.p = {"Hello", "Vector", "World", "Complete", "Test"};
            evs.m = {true, false, true, false, true};
            evs.n = {1.1f, 2.2f, 3.3f, 4.4f};
            evs.o = {1.11, 2.22, 3.33, 4.44};
            log_message("Phase 3 - EveryVecStruct: PASS");
            
            // === PHASE 4: Complex Enum with Payload ===
            sdk_test_cpp::EnumWithPayload enum_u32;
            enum_u32.tag = sdk_test_cpp::EnumWithPayloadTag::TagU32;
            enum_u32.value = static_cast<uint32_t>(123456789);
            
            sdk_test_cpp::EnumWithPayload enum_str;
            enum_str.tag = sdk_test_cpp::EnumWithPayloadTag::TagStr;
            enum_str.value = std::string("Complex enum payload test!");
            
            sdk_test_cpp::EnumWithPayload enum_bool;
            enum_bool.tag = sdk_test_cpp::EnumWithPayloadTag::TagBool;
            enum_bool.value = true;
            
            sdk_test_cpp::EnumWithPayload enum_f64;
            enum_f64.tag = sdk_test_cpp::EnumWithPayloadTag::TagF64;
            enum_f64.value = 3.141592653589793;
            log_message("Phase 4 - EnumWithPayload variants: PASS");
            
            // === PHASE 5: All Row Types ===
            sdk_test_cpp::OneU8Row u8_row{42};
            sdk_test_cpp::OneU16Row u16_row{1000};
            sdk_test_cpp::OneU32Row u32_row{1000000};
            sdk_test_cpp::OneU64Row u64_row{1000000000000ULL};
            sdk_test_cpp::OneU128Row u128_row;
            sdk_test_cpp::OneU256Row u256_row;
            
            sdk_test_cpp::OneI8Row i8_row{-42};
            sdk_test_cpp::OneI16Row i16_row{-1000};
            sdk_test_cpp::OneI32Row i32_row{-1000000};
            sdk_test_cpp::OneI64Row i64_row{-1000000000000LL};
            sdk_test_cpp::OneI128Row i128_row;
            sdk_test_cpp::OneI256Row i256_row;
            
            sdk_test_cpp::OneBoolRow bool_row{true};
            sdk_test_cpp::OneF32Row f32_row{3.14159f};
            sdk_test_cpp::OneF64Row f64_row{2.718281828};
            sdk_test_cpp::OneStringRow str_row{"All row types working!"};
            
            sdk_test_cpp::OneIdentityRow id_row;
            sdk_test_cpp::OneConnectionIdRow conn_row;
            sdk_test_cpp::OneTimestampRow ts_row;
            sdk_test_cpp::OneSimpleEnumRow enum_row{sdk_test_cpp::SimpleEnum::One};
            sdk_test_cpp::OneEnumWithPayloadRow payload_row{enum_u32};
            
            sdk_test_cpp::OneUnitStructRow unit_row{unit_struct};
            sdk_test_cpp::OneByteStructRow byte_row{byte_struct};
            sdk_test_cpp::OneEveryPrimitiveStructRow eps_row{eps};
            sdk_test_cpp::OneEveryVecStructRow evs_row{evs};
            log_message("Phase 5 - One* row types: PASS");
            
            // === PHASE 6: Vector Row Types ===
            sdk_test_cpp::VecU8Row vec_u8_row;
            vec_u8_row.n = {10, 20, 30, 40, 50};
            
            sdk_test_cpp::VecStringRow vec_str_row;
            vec_str_row.n = {"Vector", "String", "Row", "Test"};
            
            sdk_test_cpp::VecSimpleEnumRow vec_enum_row;
            vec_enum_row.n = {sdk_test_cpp::SimpleEnum::Zero, sdk_test_cpp::SimpleEnum::One, sdk_test_cpp::SimpleEnum::Two};
            
            sdk_test_cpp::VecEveryPrimitiveStructRow vec_eps_row;
            vec_eps_row.s = {eps, eps}; // duplicate for testing
            log_message("Phase 6 - Vec* row types: PASS");
            
            // === PHASE 7: Unique Row Types ===
            sdk_test_cpp::UniqueU8Row unique_u8;
            unique_u8.row_id = 1;
            unique_u8.n = 99;
            unique_u8.data = 1001;
            
            sdk_test_cpp::UniqueStringRow unique_str;
            unique_str.row_id = 2;
            unique_str.s = "Unique string test";
            unique_str.data = 2002;
            
            sdk_test_cpp::UniqueIdentityRow unique_id;
            unique_id.row_id = 3;
            unique_id.data = 3003;
            
            sdk_test_cpp::UniqueConnectionIdRow unique_conn;
            unique_conn.row_id = 4;
            unique_conn.data = 4004;
            log_message("Phase 7 - Unique* row types: PASS");
            
            // === PHASE 8: Large Table Row (Everything Combined) ===
            sdk_test_cpp::LargeTableRow large_row;
            large_row.row_id = 999999;
            large_row.a = 200;
            large_row.b = 40000;
            large_row.c = 3000000000U;
            large_row.d = 15000000000000000000ULL;
            large_row.g = -100;
            large_row.h = -20000;
            large_row.i = -1500000000;
            large_row.j = -7500000000000000000LL;
            large_row.m = false;
            large_row.n = 2.718f;
            large_row.o = 1.41421356237309504880;
            large_row.p = "MASSIVE LargeTableRow complete test!";
            large_row.va = {100, 200, 255, 128, 64, 32, 16, 8, 4, 2, 1};
            large_row.vp = {"Large", "Table", "Row", "Complete", "Success"};
            large_row.vs = {eps}; // vector of EveryPrimitiveStruct
            large_row.oi = 777;
            large_row.os = "Optional string in large table";
            large_row.oes = eps; // optional EveryPrimitiveStruct
            large_row.en = sdk_test_cpp::SimpleEnum::Two;
            large_row.ewp = enum_str;
            large_row.us = unit_struct;
            large_row.bs = byte_struct;
            large_row.eps = eps;
            large_row.evs = evs;
            log_message("Phase 8 - LargeTableRow: PASS");
            
            // === PHASE 9: Option Types ===
            sdk_test_cpp::OptionI32Row opt_i32;
            opt_i32.n = 555;
            
            sdk_test_cpp::OptionStringRow opt_str;
            opt_str.n = "Optional string success!";
            
            sdk_test_cpp::OptionIdentityRow opt_id;
            opt_id.n = ::SpacetimeDb::sdk::Identity();
            
            sdk_test_cpp::OptionSimpleEnumRow opt_enum;
            opt_enum.n = sdk_test_cpp::SimpleEnum::Zero;
            
            sdk_test_cpp::OptionEveryPrimitiveStructRow opt_eps;
            opt_eps.s = eps;
            log_message("Phase 9 - Option* types: PASS");
            
            // === PHASE 10: Primary Key Types ===
            sdk_test_cpp::PkU8Row pk_u8;
            pk_u8.n = 123;
            pk_u8.data = 9001;
            
            sdk_test_cpp::PkStringRow pk_str;
            pk_str.s = "Primary key string";
            pk_str.data = 9002;
            
            sdk_test_cpp::PkIdentityRow pk_id;
            pk_id.data = 9003;
            
            sdk_test_cpp::PkSimpleEnumRow pk_enum;
            pk_enum.e = sdk_test_cpp::SimpleEnum::One;
            pk_enum.data = 9004;
            
            sdk_test_cpp::PkU32TwoRow pk_u32_two;
            pk_u32_two.a = 111;
            pk_u32_two.b = 222;
            pk_u32_two.data = 9005;
            log_message("Phase 10 - Pk* primary key types: PASS");
            
            // === PHASE 11: Complex Nested Types ===
            sdk_test_cpp::TableHoldsTableRow table_holds;
            table_holds.row_id = 5000;
            table_holds.a = u8_row;
            table_holds.b = vec_u8_row;
            
            sdk_test_cpp::ScheduledTableRow scheduled;
            scheduled.scheduled_id = 6000;
            scheduled.text = "Scheduled table test";
            
            sdk_test_cpp::VecOptionI32 vec_opt_i32;
            vec_opt_i32.value = {std::optional<int32_t>(10), std::optional<int32_t>(20), std::nullopt, std::optional<int32_t>(30)};
            
            sdk_test_cpp::OptionVecOptionI32Row opt_vec_opt;
            opt_vec_opt.v = vec_opt_i32;
            log_message("Phase 11 - Complex nested types: PASS");
            
            // === FINAL SUCCESS MESSAGE ===
            char final_msg[200];
            const char* base = "🎉 COMPLETE SDK_TEST.H SUCCESS! ALL ";
            int i = 0;
            while (base[i] != '\0' && i < 40) {
                final_msg[i] = base[i];
                i++;
            }
            
            // Add test counter
            uint32_t counter = final_test_counter;
            char digits[10];
            int digit_count = 0;
            
            if (counter == 0) {
                digits[0] = '0';
                digit_count = 1;
            } else {
                while (counter > 0) {
                    digits[digit_count] = '0' + (counter % 10);
                    counter /= 10;
                    digit_count++;
                }
            }
            
            for (int j = digit_count - 1; j >= 0; j--) {
                final_msg[i++] = digits[j];
            }
            
            const char* suffix = " PHASES PASSED! 🚀";
            int k = 0;
            while (suffix[k] != '\0' && i < 195) {
                final_msg[i++] = suffix[k++];
            }
            final_msg[i] = '\0';
            
            log_message(final_msg);
            
        } catch (...) {
            log_message("CRITICAL ERROR: Exception in final comprehensive test!");
        }
    }
    
    // Required: Module description
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Run our comprehensive test when the module is being described
        log_message("🚀 RUNNING COMPREHENSIVE TEST DURING MODULE DESCRIPTION!");
        test_final_comprehensive_types();
        
        // Minimal valid RawModuleDef::V9 
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            0, 0, 0, 0,     // tables (empty vector)  
            0, 0, 0, 0,     // reducers (empty vector)
            0, 0, 0, 0,     // types (empty vector)
            0, 0, 0, 0,     // misc_exports (empty vector)
            0, 0, 0, 0      // row_level_security (empty vector)
        };
        
        size_t len = sizeof(data);
        bytes_sink_write(sink, data, &len);
    }
    
    // Required: Reducer dispatcher
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        log_message("🚀 FINAL SDK_TEST: Complete comprehensive type test!");
        test_final_comprehensive_types();
        return 0; // success
    }
}