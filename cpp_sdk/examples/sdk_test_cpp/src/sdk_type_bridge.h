// Bridge header to provide missing SpacetimeDB SDK types for compilation
#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <variant>

// Create the missing namespaces and types that sdk_test.h expects
namespace SpacetimeDb {
    namespace Types {
        // Placeholder implementations for 128-bit types
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
        // 256-bit types as arrays
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
                if (val < 0) {
                    data[1] = data[2] = data[3] = 0xFFFFFFFFFFFFFFFFULL;
                }
            }
        };
        
        // Identity as a simple byte array
        struct Identity {
            std::array<uint8_t, 32> bytes = {};
            
            Identity() = default;
            Identity(const std::array<uint8_t, 32>& b) : bytes(b) {}
        };
        
        // ConnectionId as a simple 64-bit value
        struct ConnectionId {
            uint64_t id = 0;
            
            ConnectionId() = default;
            ConnectionId(uint64_t val) : id(val) {}
        };
        
        // Timestamp as microseconds since Unix epoch
        struct Timestamp {
            uint64_t microseconds = 0;
            
            Timestamp() = default;
            Timestamp(uint64_t us) : microseconds(us) {}
        };
        
        // TimeDuration as microseconds
        struct TimeDuration {
            uint64_t microseconds = 0;
            
            TimeDuration() = default;
            TimeDuration(uint64_t us) : microseconds(us) {}
        };
        
        // ScheduleAt placeholder
        struct ScheduleAt {
            uint64_t timestamp = 0;
            
            ScheduleAt() = default;
            ScheduleAt(uint64_t ts) : timestamp(ts) {}
        };
    }
    
    // Core type enumeration that sdk_test.h expects
    enum class CoreType : uint8_t {
        Bool = 0,
        I8 = 1,
        U8 = 2,
        I16 = 3,
        U16 = 4,
        I32 = 5,
        U32 = 6,
        I64 = 7,
        U64 = 8,
        I128 = 9,
        U128 = 10,
        I256 = 11,
        U256 = 12,
        F32 = 13,
        F64 = 14,
        String = 15,
        Bytes = 16,
        UserDefined = 17
    };
    
    // Minimal type system for schema registration
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
    
    // Minimal BSATN types
    namespace bsatn {
        class Writer {
        public:
            void write_u8(uint8_t val) {}
            void write_u16_le(uint16_t val) {}
            void write_u32_le(uint32_t val) {}
            void write_u64_le(uint64_t val) {}
            void write_i8(int8_t val) {}
            void write_i16_le(int16_t val) {}
            void write_i32_le(int32_t val) {}
            void write_i64_le(int64_t val) {}
            void write_bool(bool val) {}
            void write_f32_le(float val) {}
            void write_f64_le(double val) {}
            void write_string(const std::string& val) {}
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
            
            template<typename T>
            std::vector<T> read_vector() { return std::vector<T>{}; }
        };
        
        // Serialization function templates
        template<typename T>
        void serialize(Writer& writer, const T& value) {
            // Default implementation - would need specialization for each type
        }
        
        template<typename T>
        T deserialize(Reader& reader) {
            return T{};
        }
    }
}