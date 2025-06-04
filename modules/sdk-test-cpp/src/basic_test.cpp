/**
 * Basic SpacetimeDB C++ SDK Test
 * 
 * Tests core functionality without complex features.
 */

#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

// FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void _console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t _datastore_insert_bsatn(uint32_t table_id, uint8_t* row, size_t* row_len);
    
    // Remove iterator imports for now
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t _get_table_id(const uint8_t* name, size_t name_len, uint32_t* table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t _bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
}

// Simple logging
void log_info(const std::string& msg) {
    const char* filename = "basic_test.cpp";
    _console_log(2, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg.c_str(), msg.length());
}

// Simple BSATN writer
class SimpleWriter {
    std::vector<uint8_t> buffer;
    
public:
    void write_u8(uint8_t val) {
        buffer.push_back(val);
    }
    
    void write_u16_le(uint16_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
    }
    
    void write_u32_le(uint32_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back((val >> 16) & 0xFF);
        buffer.push_back((val >> 24) & 0xFF);
    }
    
    void write_u64_le(uint64_t val) {
        for (int i = 0; i < 8; i++) {
            buffer.push_back((val >> (i * 8)) & 0xFF);
        }
    }
    
    void write_string(const std::string& str) {
        write_u32_le(str.length());
        for (char c : str) {
            buffer.push_back(static_cast<uint8_t>(c));
        }
    }
    
    void write_vec_len(size_t len) {
        write_u32_le(static_cast<uint32_t>(len));
    }
    
    std::vector<uint8_t>& get_buffer() { return buffer; }
    const std::vector<uint8_t>& get_buffer() const { return buffer; }
};

// Test data structures
struct TestData {
    uint32_t id;
    std::string name;
    int32_t value;
    
    void serialize(SimpleWriter& writer) const {
        writer.write_u32_le(id);
        writer.write_string(name);
        writer.write_u32_le(static_cast<uint32_t>(value));
    }
};

// Module description export
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        SimpleWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace - just write the types vec and names vec
        
        // 1. Typespace
        // types: Vec<AlgebraicType>
        w.write_vec_len(1); // 1 type
        
        // TestData type as Product
        w.write_u8(2); // AlgebraicType::Product
        w.write_vec_len(3); // 3 fields
        
        // Field 1: id
        w.write_u8(0); // Some
        w.write_string("id");
        w.write_u8(9); // U32
        
        // Field 2: name
        w.write_u8(0); // Some
        w.write_string("name");
        w.write_u8(15); // String
        
        // Field 3: value
        w.write_u8(0); // Some
        w.write_string("value");
        w.write_u8(5); // I32
        
        // names: Vec<ScopedTypeName>
        w.write_vec_len(1);
        w.write_vec_len(0); // scope
        w.write_string("TestData");
        w.write_u32_le(0); // type ref
        
        // 2. tables: Vec<RawTableDefV9>
        w.write_vec_len(1); // 1 table
        
        w.write_string("test_data"); // name
        w.write_u32_le(0); // product_type_ref
        w.write_u8(1); // primary_key: None
        w.write_vec_len(0); // indexes
        w.write_vec_len(0); // constraints
        w.write_vec_len(0); // sequences
        w.write_u8(1); // schedule: None
        w.write_u8(0); // table_type: User
        w.write_u8(0); // table_access: Public
        
        // 3. reducers: Vec<RawReducerDefV9>
        w.write_vec_len(3); // 3 reducers
        
        // init reducer
        w.write_string("__init__");
        w.write_u8(2); // Product type
        w.write_vec_len(0); // No params
        w.write_u8(0); // Some
        w.write_u8(1); // Lifecycle::Init
        
        // test_insert reducer
        w.write_string("test_insert");
        w.write_u8(2); // Product type
        w.write_vec_len(0); // No params for simplicity
        w.write_u8(1); // None (no lifecycle)
        
        // test_count reducer
        w.write_string("test_count");
        w.write_u8(2); // Product type
        w.write_vec_len(0); // No params
        w.write_u8(1); // None
        
        // 4. types: Vec<RawTypeDefV9> (empty)
        w.write_vec_len(0);
        
        // 5. misc_exports: Vec<RawMiscModuleExportV9> (empty)
        w.write_vec_len(0);
        
        // 6. row_level_security: Vec<RawRowLevelSecurityDefV9> (empty)
        w.write_vec_len(0);
        
        // Write to sink
        auto& buffer = w.get_buffer();
        size_t len = buffer.size();
        _bytes_sink_write(sink, buffer.data(), &len);
    }
}

// Forward declarations for reducers
extern "C" {
    void __init__(uint32_t source, uint32_t sink);
    void test_insert(uint32_t source, uint32_t sink);
    void test_count(uint32_t source, uint32_t sink);
}

// Module's call reducer implementation
extern "C" {
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t reducer_id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp_us,
        uint32_t args_source,
        uint32_t error_sink
    ) {
        // Simple dispatch based on reducer ID
        switch (reducer_id) {
            case 0: // __init__
                __init__(args_source, error_sink);
                return 0;
            case 1: // test_insert
                test_insert(args_source, error_sink);
                return 0;
            case 2: // test_count
                test_count(args_source, error_sink);
                return 0;
            default:
                log_info("Unknown reducer ID: " + std::to_string(reducer_id));
                return -1; // NO_SUCH_REDUCER
        }
    }
}

// Reducers
extern "C" {
    __attribute__((export_name("__init__")))
    void __init__(uint32_t source, uint32_t sink) {
        log_info("Basic test module initialized");
    }
    
    __attribute__((export_name("test_insert")))
    void test_insert(uint32_t source, uint32_t sink) {
        log_info("test_insert called");
        
        // Get table ID
        uint32_t table_id = 0;
        const char* table_name = "test_data";
        if (_get_table_id((const uint8_t*)table_name, strlen(table_name), &table_id) != 0) {
            log_info("Failed to get table ID");
            return;
        }
        
        // Create test data
        TestData data{42, "test", 100};
        
        // Serialize
        SimpleWriter writer;
        data.serialize(writer);
        
        // Insert
        auto& buffer = writer.get_buffer();
        size_t len = buffer.size();
        auto err = _datastore_insert_bsatn(table_id, buffer.data(), &len);
        
        if (err == 0) {
            log_info("Insert successful");
        } else {
            log_info("Insert failed with error: " + std::to_string(err));
        }
    }
    
    __attribute__((export_name("test_count")))
    void test_count(uint32_t source, uint32_t sink) {
        log_info("test_count called - count feature not implemented in basic test");
    }
}