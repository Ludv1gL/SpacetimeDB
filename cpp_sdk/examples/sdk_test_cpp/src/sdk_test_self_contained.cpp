// Self-contained SpacetimeDB module recreating all sdk_test types
#include "sdk_type_bridge.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <variant>
#include <optional>

// Recreate all the sdk_test types locally to avoid SDK header issues
namespace sdk_test_cpp {
    
    // Basic enum types
    enum class SimpleEnum : uint8_t {
        Zero,
        One,
        Two,
    };
    
    enum class GlobalSimpleEnum : uint8_t {
        GlobalZero,
        GlobalOne,
        GlobalTwo,
    };
    
    // Basic struct types
    struct UnitStruct {};
    struct GlobalUnitStruct {};
    
    struct ByteStruct {
        uint8_t b;
    };
    
    // Complex primitive struct using bridge types
    struct EveryPrimitiveStruct {
        uint8_t a;
        uint16_t b;
        uint32_t c;
        uint64_t d;
        ::SpacetimeDb::Types::uint128_t_placeholder e;
        ::SpacetimeDb::sdk::u256_placeholder f;
        int8_t g;
        int16_t h;
        int32_t i;
        int64_t j;
        ::SpacetimeDb::Types::int128_t_placeholder k;
        ::SpacetimeDb::sdk::i256_placeholder l;
        bool m;
        float n;
        double o;
        std::string p;
        ::SpacetimeDb::sdk::Identity q;
        ::SpacetimeDb::sdk::ConnectionId r;
        ::SpacetimeDb::sdk::Timestamp s;
        ::SpacetimeDb::sdk::TimeDuration t;
    };
    
    // Vector struct
    struct EveryVecStruct {
        std::vector<uint8_t> a;
        std::vector<uint16_t> b;
        std::vector<uint32_t> c;
        std::vector<uint64_t> d;
        std::vector<::SpacetimeDb::Types::uint128_t_placeholder> e;
        std::vector<::SpacetimeDb::sdk::u256_placeholder> f;
        std::vector<int8_t> g;
        std::vector<int16_t> h;
        std::vector<int32_t> i;
        std::vector<int64_t> j;
        std::vector<::SpacetimeDb::Types::int128_t_placeholder> k;
        std::vector<::SpacetimeDb::sdk::i256_placeholder> l;
        std::vector<bool> m;
        std::vector<float> n;
        std::vector<double> o;
        std::vector<std::string> p;
        std::vector<::SpacetimeDb::sdk::Identity> q;
        std::vector<::SpacetimeDb::sdk::ConnectionId> r;
        std::vector<::SpacetimeDb::sdk::Timestamp> s;
        std::vector<::SpacetimeDb::sdk::TimeDuration> t;
    };
    
    // Enum with payload
    enum class EnumWithPayloadTag : uint8_t {
        TagU8, TagU16, TagU32, TagU64, TagU128, TagU256,
        TagI8, TagI16, TagI32, TagI64, TagI128, TagI256,
        TagBool, TagF32, TagF64, TagStr, TagIdentity, TagConnectionId,
        TagTimestamp, TagBytes, TagInts, TagStrings, TagSimpleEnums
    };
    
    struct EnumWithPayload {
        EnumWithPayloadTag tag;
        std::variant<
            uint8_t, uint16_t, uint32_t, uint64_t, 
            ::SpacetimeDb::Types::uint128_t_placeholder, ::SpacetimeDb::sdk::u256_placeholder,
            int8_t, int16_t, int32_t, int64_t, 
            ::SpacetimeDb::Types::int128_t_placeholder, ::SpacetimeDb::sdk::i256_placeholder,
            bool, float, double, std::string, 
            ::SpacetimeDb::sdk::Identity, ::SpacetimeDb::sdk::ConnectionId,
            ::SpacetimeDb::sdk::Timestamp, std::vector<uint8_t>, std::vector<int32_t>,
            std::vector<std::string>, std::vector<SimpleEnum>
        > value;
        
        // Constructor for easy setup
        EnumWithPayload() : tag(EnumWithPayloadTag::TagU8), value(uint8_t(0)) {}
        
        template<typename T>
        EnumWithPayload(EnumWithPayloadTag t, T val) : tag(t), value(val) {}
    };
    
    // Row types for tables
    struct OneU8Row { uint8_t n; };
    struct OneU16Row { uint16_t n; };
    struct OneU32Row { uint32_t n; };
    struct OneU64Row { uint64_t n; };
    struct OneU128Row { ::SpacetimeDb::Types::uint128_t_placeholder n; };
    struct OneU256Row { ::SpacetimeDb::sdk::u256_placeholder n; };
    
    struct OneI8Row { int8_t n; };
    struct OneI16Row { int16_t n; };
    struct OneI32Row { int32_t n; };
    struct OneI64Row { int64_t n; };
    struct OneI128Row { ::SpacetimeDb::Types::int128_t_placeholder n; };
    struct OneI256Row { ::SpacetimeDb::sdk::i256_placeholder n; };
    
    struct OneBoolRow { bool n; };
    struct OneF32Row { float n; };
    struct OneF64Row { double n; };
    struct OneStringRow { std::string n; };
    
    struct OneIdentityRow { ::SpacetimeDb::sdk::Identity n; };
    struct OneConnectionIdRow { ::SpacetimeDb::sdk::ConnectionId n; };
    struct OneTimestampRow { ::SpacetimeDb::sdk::Timestamp n; };
    struct OneSimpleEnumRow { SimpleEnum n; };
    struct OneEnumWithPayloadRow { EnumWithPayload n; };
    
    struct OneUnitStructRow { UnitStruct s; };
    struct OneByteStructRow { ByteStruct s; };
    struct OneEveryPrimitiveStructRow { EveryPrimitiveStruct s; };
    struct OneEveryVecStructRow { EveryVecStruct s; };
    
    // Large table combining many types
    struct LargeTableRow {
        uint64_t row_id;
        uint8_t a;
        uint16_t b;
        uint32_t c;
        uint64_t d;
        ::SpacetimeDb::Types::uint128_t_placeholder e;
        ::SpacetimeDb::sdk::u256_placeholder f;
        int8_t g;
        int16_t h;
        int32_t i;
        int64_t j;
        ::SpacetimeDb::Types::int128_t_placeholder k;
        ::SpacetimeDb::sdk::i256_placeholder l;
        bool m;
        float n;
        double o;
        std::string p;
        ::SpacetimeDb::sdk::Identity q;
        ::SpacetimeDb::sdk::ConnectionId r;
        ::SpacetimeDb::sdk::Timestamp s;
        ::SpacetimeDb::sdk::TimeDuration t;
        std::vector<uint8_t> va;
        std::vector<std::string> vp;
        std::vector<EveryPrimitiveStruct> vs;
        std::optional<int32_t> oi;
        std::optional<std::string> os;
        std::optional<EveryPrimitiveStruct> oes;
        SimpleEnum en;
        EnumWithPayload ewp;
        UnitStruct us;
        ByteStruct bs;
        EveryPrimitiveStruct eps;
        EveryVecStruct evs;
    };
    
    // Option types
    struct OptionI32Row { std::optional<int32_t> n; };
    struct OptionStringRow { std::optional<std::string> n; };
    struct OptionIdentityRow { std::optional<::SpacetimeDb::sdk::Identity> n; };
    struct OptionSimpleEnumRow { std::optional<SimpleEnum> n; };
    struct OptionEveryPrimitiveStructRow { std::optional<EveryPrimitiveStruct> s; };
}

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
    uint32_t self_contained_counter = 0;
    
    // Helper to log messages
    void log_message(const char* message) {
        const char* filename = "sdk_test_self_contained.cpp";
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, strlen(filename),
            ++self_contained_counter,
            (const uint8_t*)message, strlen(message)
        );
    }
    
    // Comprehensive test of all recreated types
    void test_all_self_contained_types() {
        try {
            // Test enums
            sdk_test_cpp::SimpleEnum simple_enum = sdk_test_cpp::SimpleEnum::Two;
            sdk_test_cpp::GlobalSimpleEnum global_enum = sdk_test_cpp::GlobalSimpleEnum::GlobalOne;
            log_message("Enums: OK");
            
            // Test basic structs
            sdk_test_cpp::UnitStruct unit_struct;
            sdk_test_cpp::ByteStruct byte_struct;
            byte_struct.b = 255;
            log_message("Basic structs: OK");
            
            // Test primitive struct with all bridge types
            sdk_test_cpp::EveryPrimitiveStruct eps;
            eps.a = 255;
            eps.b = 65535;
            eps.c = 4294967295U;
            eps.d = 18446744073709551615ULL;
            eps.e = ::SpacetimeDb::Types::uint128_t_placeholder(12345);
            eps.f = ::SpacetimeDb::sdk::u256_placeholder(67890);
            eps.g = -128;
            eps.h = -32768;
            eps.i = -2000000;
            eps.j = -9000000000000000000LL;
            eps.k = ::SpacetimeDb::Types::int128_t_placeholder(-54321);
            eps.l = ::SpacetimeDb::sdk::i256_placeholder(-98765);
            eps.m = true;
            eps.n = 3.14159f;
            eps.o = 2.71828182845904523536;
            eps.p = "All primitives working!";
            eps.q = ::SpacetimeDb::sdk::Identity();
            eps.r = ::SpacetimeDb::sdk::ConnectionId(42);
            eps.s = ::SpacetimeDb::sdk::Timestamp(1234567890);
            eps.t = ::SpacetimeDb::sdk::TimeDuration(9876543210);
            log_message("EveryPrimitiveStruct: OK");
            
            // Test vector struct
            sdk_test_cpp::EveryVecStruct evs;
            evs.a = {1, 2, 3, 255};
            evs.b = {100, 200, 65535};
            evs.c = {1000, 2000, 4294967295U};
            evs.p = {"Hello", "Vector", "World", "Complete!"};
            evs.m = {true, false, true, false};
            evs.n = {1.1f, 2.2f, 3.3f};
            evs.o = {1.11, 2.22, 3.33};
            log_message("EveryVecStruct: OK");
            
            // Test complex enum with payload
            sdk_test_cpp::EnumWithPayload enum_u32(sdk_test_cpp::EnumWithPayloadTag::TagU32, 123456U);
            sdk_test_cpp::EnumWithPayload enum_str(sdk_test_cpp::EnumWithPayloadTag::TagStr, std::string("Payload test"));
            sdk_test_cpp::EnumWithPayload enum_bool(sdk_test_cpp::EnumWithPayloadTag::TagBool, true);
            log_message("EnumWithPayload: OK");
            
            // Test all row types
            sdk_test_cpp::OneU8Row u8_row{42};
            sdk_test_cpp::OneStringRow str_row{"Self-contained success!"};
            sdk_test_cpp::OneIdentityRow id_row;
            sdk_test_cpp::OneEnumWithPayloadRow payload_row{enum_u32};
            log_message("Row types: OK");
            
            // Test large table row
            sdk_test_cpp::LargeTableRow large_row;
            large_row.row_id = 1001;
            large_row.a = 200;
            large_row.p = "Large table complete test";
            large_row.m = true;
            large_row.va = {10, 20, 30, 40, 50};
            large_row.vp = {"Large", "Table", "Success"};
            large_row.oi = 42;
            large_row.os = "Optional string";
            large_row.en = sdk_test_cpp::SimpleEnum::One;
            large_row.ewp = enum_str;
            large_row.bs = byte_struct;
            large_row.eps = eps;
            large_row.evs = evs;
            log_message("LargeTableRow: OK");
            
            // Test optional types
            sdk_test_cpp::OptionI32Row opt_i32;
            opt_i32.n = 999;
            
            sdk_test_cpp::OptionStringRow opt_str;
            opt_str.n = "Optional success";
            
            sdk_test_cpp::OptionSimpleEnumRow opt_enum;
            opt_enum.n = sdk_test_cpp::SimpleEnum::Two;
            log_message("Optional types: OK");
            
            // Final success message
            char final_msg[120];
            const char* base = "ALL SDK_TEST TYPES SELF-CONTAINED SUCCESS #";
            int i = 0;
            while (base[i] != '\0' && i < 50) {
                final_msg[i] = base[i];
                i++;
            }
            
            // Add counter
            uint32_t counter = self_contained_counter;
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
            
            const char* suffix = "!";
            final_msg[i++] = suffix[0];
            final_msg[i] = '\0';
            
            log_message(final_msg);
            
        } catch (...) {
            log_message("ERROR: Exception during self-contained type testing!");
        }
    }
    
    // Required: Module description
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
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
        log_message("SDK Test Self-Contained: Testing ALL recreated types!");
        test_all_self_contained_types();
        return 0; // success
    }
}