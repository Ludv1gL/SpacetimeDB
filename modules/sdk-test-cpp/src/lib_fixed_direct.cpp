/**
 * SpacetimeDB C++ Module - Direct FFI Implementation
 * 
 * This module demonstrates all primitive SpacetimeDB types using direct FFI.
 * It avoids the C++ module library issues with reducer parameter registration.
 */

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <climits>
#include <cmath>

// SpacetimeDB FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row, size_t* row_len);
}

// Helper functions
void write_to_sink(uint32_t sink, const uint8_t* data, size_t len) {
    size_t written = len;
    bytes_sink_write(sink, data, &written);
}

void log_info(const std::string& message) {
    console_log(2, // INFO level
               (const uint8_t*)"module", 6,
               (const uint8_t*)__FILE__, strlen(__FILE__),
               __LINE__,
               (const uint8_t*)message.c_str(), message.length());
}

// BSATN writer
class BsatnWriter {
    std::vector<uint8_t> buffer;
    
public:
    void write_u8(uint8_t value) {
        buffer.push_back(value);
    }
    
    void write_i8(int8_t value) {
        buffer.push_back(static_cast<uint8_t>(value));
    }
    
    void write_u16_le(uint16_t value) {
        buffer.push_back(value & 0xFF);
        buffer.push_back((value >> 8) & 0xFF);
    }
    
    void write_i16_le(int16_t value) {
        write_u16_le(static_cast<uint16_t>(value));
    }
    
    void write_u32_le(uint32_t value) {
        buffer.push_back(value & 0xFF);
        buffer.push_back((value >> 8) & 0xFF);
        buffer.push_back((value >> 16) & 0xFF);
        buffer.push_back((value >> 24) & 0xFF);
    }
    
    void write_i32_le(int32_t value) {
        write_u32_le(static_cast<uint32_t>(value));
    }
    
    void write_u64_le(uint64_t value) {
        for (int i = 0; i < 8; i++) {
            buffer.push_back((value >> (i * 8)) & 0xFF);
        }
    }
    
    void write_i64_le(int64_t value) {
        write_u64_le(static_cast<uint64_t>(value));
    }
    
    void write_f32_le(float value) {
        uint32_t bits;
        memcpy(&bits, &value, sizeof(float));
        write_u32_le(bits);
    }
    
    void write_f64_le(double value) {
        uint64_t bits;
        memcpy(&bits, &value, sizeof(double));
        write_u64_le(bits);
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.length());
        buffer.insert(buffer.end(), str.begin(), str.end());
    }
    
    const std::vector<uint8_t>& get_buffer() const { return buffer; }
};

// BSATN reader
class BsatnReader {
    const uint8_t* data;
    size_t size;
    size_t pos;
    
public:
    BsatnReader(const uint8_t* d, size_t s) : data(d), size(s), pos(0) {}
    
    uint8_t read_u8() {
        if (pos >= size) return 0;
        return data[pos++];
    }
    
    int8_t read_i8() {
        return static_cast<int8_t>(read_u8());
    }
    
    uint16_t read_u16_le() {
        if (pos + 2 > size) return 0;
        uint16_t value = data[pos] | (data[pos+1] << 8);
        pos += 2;
        return value;
    }
    
    int16_t read_i16_le() {
        return static_cast<int16_t>(read_u16_le());
    }
    
    uint32_t read_u32_le() {
        if (pos + 4 > size) return 0;
        uint32_t value = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24);
        pos += 4;
        return value;
    }
    
    int32_t read_i32_le() {
        return static_cast<int32_t>(read_u32_le());
    }
    
    uint64_t read_u64_le() {
        if (pos + 8 > size) return 0;
        uint64_t value = 0;
        for (int i = 0; i < 8; i++) {
            value |= static_cast<uint64_t>(data[pos + i]) << (i * 8);
        }
        pos += 8;
        return value;
    }
    
    int64_t read_i64_le() {
        return static_cast<int64_t>(read_u64_le());
    }
    
    float read_f32_le() {
        uint32_t bits = read_u32_le();
        float value;
        memcpy(&value, &bits, sizeof(float));
        return value;
    }
    
    double read_f64_le() {
        uint64_t bits = read_u64_le();
        double value;
        memcpy(&value, &bits, sizeof(double));
        return value;
    }
    
    std::string read_string() {
        uint32_t len = read_u32_le();
        if (pos + len > size) return "";
        std::string str((const char*)(data + pos), len);
        pos += len;
        return str;
    }
};

// Global table ID cache
struct TableIds {
    uint32_t test_u8 = 0;
    uint32_t test_i8 = 0;
    uint32_t test_u16 = 0;
    uint32_t test_i16 = 0;
    uint32_t test_u32 = 0;
    uint32_t test_i32 = 0;
    uint32_t test_u64 = 0;
    uint32_t test_i64 = 0;
    uint32_t test_f32 = 0;
    uint32_t test_f64 = 0;
} g_table_ids;

// Module exports
extern "C" {

// Describe module with all primitive type tables and reducers
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    BsatnWriter writer;
    
    // Write RawModuleDef::V9 (variant 1)
    writer.write_u8(1);
    
    // Typespace with 10 types (one for each primitive type table)
    writer.write_u32_le(10);
    
    // Type 0: TestU8 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(7); // U8
    
    // Type 1: TestI8 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(6); // I8
    
    // Type 2: TestU16 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(9); // U16
    
    // Type 3: TestI16 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(8); // I16
    
    // Type 4: TestU32 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(11); // U32
    
    // Type 5: TestI32 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(10); // I32
    
    // Type 6: TestU64 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(13); // U64
    
    // Type 7: TestI64 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(12); // I64
    
    // Type 8: TestF32 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(18); // F32
    
    // Type 9: TestF64 struct
    writer.write_u8(2); // Product type
    writer.write_u32_le(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(19); // F64
    
    // Tables: 10 tables
    writer.write_u32_le(10);
    
    // Table 0: test_u8
    writer.write_string("test_u8");
    writer.write_u32_le(0); // type_ref = 0
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 1: test_i8
    writer.write_string("test_i8");
    writer.write_u32_le(1); // type_ref = 1
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 2: test_u16
    writer.write_string("test_u16");
    writer.write_u32_le(2); // type_ref = 2
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 3: test_i16
    writer.write_string("test_i16");
    writer.write_u32_le(3); // type_ref = 3
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 4: test_u32
    writer.write_string("test_u32");
    writer.write_u32_le(4); // type_ref = 4
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 5: test_i32
    writer.write_string("test_i32");
    writer.write_u32_le(5); // type_ref = 5
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 6: test_u64
    writer.write_string("test_u64");
    writer.write_u32_le(6); // type_ref = 6
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 7: test_i64
    writer.write_string("test_i64");
    writer.write_u32_le(7); // type_ref = 7
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 8: test_f32
    writer.write_string("test_f32");
    writer.write_u32_le(8); // type_ref = 8
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Table 9: test_f64
    writer.write_string("test_f64");
    writer.write_u32_le(9); // type_ref = 9
    writer.write_u32_le(0); // primary_key = empty
    writer.write_u32_le(0); // indexes
    writer.write_u32_le(0); // constraints
    writer.write_u32_le(0); // sequences
    writer.write_u8(1);     // schedule = None
    writer.write_u8(1);     // table_type = User
    writer.write_u8(0);     // table_access = Public
    
    // Reducers: 12 reducers (init + test_all_types + 10 insert reducers)
    writer.write_u32_le(12);
    
    // Reducer 0: init
    writer.write_string("init");
    writer.write_u32_le(0); // params: empty ProductType
    writer.write_u8(0);     // lifecycle: Some(Init)
    writer.write_u8(0);     // Init variant
    
    // Reducer 1: test_all_types
    writer.write_string("test_all_types");
    writer.write_u32_le(0); // params: empty ProductType
    writer.write_u8(1);     // lifecycle: None
    
    // Reducer 2: insert_u8(n: u8)
    writer.write_string("insert_u8");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(7); // U8
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 3: insert_i8(n: i8)
    writer.write_string("insert_i8");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(6); // I8
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 4: insert_u16(n: u16)
    writer.write_string("insert_u16");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(9); // U16
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 5: insert_i16(n: i16)
    writer.write_string("insert_i16");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(8); // I16
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 6: insert_u32(n: u32)
    writer.write_string("insert_u32");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(11); // U32
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 7: insert_i32(n: i32)
    writer.write_string("insert_i32");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(10); // I32
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 8: insert_u64(n: u64)
    writer.write_string("insert_u64");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(13); // U64
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 9: insert_i64(n: i64)
    writer.write_string("insert_i64");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(12); // I64
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 10: insert_f32(n: f32)
    writer.write_string("insert_f32");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(18); // F32
    writer.write_u8(1); // lifecycle: None
    
    // Reducer 11: insert_f64(n: f64)
    writer.write_string("insert_f64");
    writer.write_u32_le(1); // params: 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(19); // F64
    writer.write_u8(1); // lifecycle: None
    
    // Empty arrays for the rest
    writer.write_u32_le(0); // types
    writer.write_u32_le(0); // misc_exports
    writer.write_u32_le(0); // row_level_security
    
    // Write to sink
    const auto& buffer = writer.get_buffer();
    write_to_sink(sink, buffer.data(), buffer.size());
}

// Helper to get table ID
uint32_t get_table_id(const char* table_name) {
    uint32_t table_id = 0;
    auto err = table_id_from_name(
        reinterpret_cast<const uint8_t*>(table_name), 
        strlen(table_name), 
        &table_id
    );
    if (err != 0) {
        log_info("Failed to get table ID for " + std::string(table_name));
        return 0;
    }
    return table_id;
}

// Initialize table IDs
void init_table_ids() {
    if (g_table_ids.test_u8 == 0) {
        g_table_ids.test_u8 = get_table_id("test_u8");
        g_table_ids.test_i8 = get_table_id("test_i8");
        g_table_ids.test_u16 = get_table_id("test_u16");
        g_table_ids.test_i16 = get_table_id("test_i16");
        g_table_ids.test_u32 = get_table_id("test_u32");
        g_table_ids.test_i32 = get_table_id("test_i32");
        g_table_ids.test_u64 = get_table_id("test_u64");
        g_table_ids.test_i64 = get_table_id("test_i64");
        g_table_ids.test_f32 = get_table_id("test_f32");
        g_table_ids.test_f64 = get_table_id("test_f64");
    }
}

// Handle reducer calls
__attribute__((export_name("__call_reducer__")))
int32_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    // Initialize table IDs on first reducer call
    init_table_ids();
    
    // Read arguments from source
    size_t args_len = 0;
    bytes_source_read(args, nullptr, &args_len);
    
    std::vector<uint8_t> args_buffer(args_len);
    bytes_source_read(args, args_buffer.data(), &args_len);
    
    BsatnReader reader(args_buffer.data(), args_len);
    
    switch (id) {
        case 0: { // init
            log_info("Initializing module with all primitive types...");
            return 0;
        }
        
        case 1: { // test_all_types
            log_info("Testing all primitive types...");
            
            // Test u8
            {
                BsatnWriter w;
                w.write_u8(255);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_u8, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted u8: 255");
            }
            
            // Test i8
            {
                BsatnWriter w;
                w.write_i8(-128);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_i8, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted i8: -128");
            }
            
            // Test u16
            {
                BsatnWriter w;
                w.write_u16_le(65535);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_u16, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted u16: 65535");
            }
            
            // Test i16
            {
                BsatnWriter w;
                w.write_i16_le(-32768);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_i16, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted i16: -32768");
            }
            
            // Test u32
            {
                BsatnWriter w;
                w.write_u32_le(4294967295U);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_u32, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted u32: 4294967295");
            }
            
            // Test i32
            {
                BsatnWriter w;
                w.write_i32_le(-2147483648);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_i32, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted i32: -2147483648");
            }
            
            // Test u64
            {
                BsatnWriter w;
                w.write_u64_le(18446744073709551615ULL);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_u64, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted u64: 18446744073709551615");
            }
            
            // Test i64
            {
                BsatnWriter w;
                w.write_i64_le(-9223372036854775807LL);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_i64, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted i64: -9223372036854775807");
            }
            
            // Test f32
            {
                BsatnWriter w;
                w.write_f32_le(3.14159f);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_f32, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted f32: 3.14159");
            }
            
            // Test f64
            {
                BsatnWriter w;
                w.write_f64_le(2.718281828459045);
                const auto& buf = w.get_buffer();
                size_t len = buf.size();
                datastore_insert_bsatn(g_table_ids.test_f64, const_cast<uint8_t*>(buf.data()), &len);
                log_info("Inserted f64: 2.718281828459045");
            }
            
            return 0;
        }
        
        case 2: { // insert_u8
            uint8_t value = reader.read_u8();
            BsatnWriter w;
            w.write_u8(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_u8, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted u8: " + std::to_string(value));
            return 0;
        }
        
        case 3: { // insert_i8
            int8_t value = reader.read_i8();
            BsatnWriter w;
            w.write_i8(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_i8, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted i8: " + std::to_string(value));
            return 0;
        }
        
        case 4: { // insert_u16
            uint16_t value = reader.read_u16_le();
            BsatnWriter w;
            w.write_u16_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_u16, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted u16: " + std::to_string(value));
            return 0;
        }
        
        case 5: { // insert_i16
            int16_t value = reader.read_i16_le();
            BsatnWriter w;
            w.write_i16_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_i16, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted i16: " + std::to_string(value));
            return 0;
        }
        
        case 6: { // insert_u32
            uint32_t value = reader.read_u32_le();
            BsatnWriter w;
            w.write_u32_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_u32, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted u32: " + std::to_string(value));
            return 0;
        }
        
        case 7: { // insert_i32
            int32_t value = reader.read_i32_le();
            BsatnWriter w;
            w.write_i32_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_i32, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted i32: " + std::to_string(value));
            return 0;
        }
        
        case 8: { // insert_u64
            uint64_t value = reader.read_u64_le();
            BsatnWriter w;
            w.write_u64_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_u64, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted u64: " + std::to_string(value));
            return 0;
        }
        
        case 9: { // insert_i64
            int64_t value = reader.read_i64_le();
            BsatnWriter w;
            w.write_i64_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_i64, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted i64: " + std::to_string(value));
            return 0;
        }
        
        case 10: { // insert_f32
            float value = reader.read_f32_le();
            BsatnWriter w;
            w.write_f32_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_f32, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted f32: " + std::to_string(value));
            return 0;
        }
        
        case 11: { // insert_f64
            double value = reader.read_f64_le();
            BsatnWriter w;
            w.write_f64_le(value);
            const auto& buf = w.get_buffer();
            size_t len = buf.size();
            datastore_insert_bsatn(g_table_ids.test_f64, const_cast<uint8_t*>(buf.data()), &len);
            log_info("Inserted f64: " + std::to_string(value));
            return 0;
        }
    }
    
    return -999; // Unknown reducer
}

} // extern "C"