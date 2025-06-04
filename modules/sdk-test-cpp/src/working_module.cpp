/**
 * Working SpacetimeDB C++ module
 * Implements basic functionality without using the problematic SDK
 */

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

// FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void _console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t _bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t _bytes_source_read(uint32_t source, uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t _table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t _datastore_insert_bsatn(uint32_t table_id, uint8_t* row, size_t* row_len);
}

void log_info(const std::string& msg) {
    const char* filename = "working_module.cpp";
    _console_log(2, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg.c_str(), msg.length());
}

// BSATN writer
class BsatnWriter {
    std::vector<uint8_t> buffer;
    
public:
    void write_u8(uint8_t val) {
        buffer.push_back(val);
    }
    
    void write_u32_le(uint32_t val) {
        buffer.push_back(val & 0xFF);
        buffer.push_back((val >> 8) & 0xFF);
        buffer.push_back((val >> 16) & 0xFF);
        buffer.push_back((val >> 24) & 0xFF);
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
};

// Forward declarations
extern "C" {
    void __init__(uint32_t source, uint32_t sink);
    void add_person(uint32_t source, uint32_t sink);
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        BsatnWriter w;
        
        // RawModuleDef::V9
        w.write_u8(1);
        
        // Typespace: types vec
        w.write_vec_len(1); // 1 type: Person
        
        // Person type (product with 2 fields)
        w.write_u8(2); // AlgebraicType::Product
        w.write_vec_len(2); // 2 fields
        
        // Field 1: id (u32)
        w.write_u8(0); // Some
        w.write_string("id");
        w.write_u8(9); // AlgebraicType::U32
        
        // Field 2: name (String)
        w.write_u8(0); // Some
        w.write_string("name");
        w.write_u8(15); // AlgebraicType::String
        
        // Typespace: names vec
        w.write_vec_len(1);
        w.write_vec_len(0); // empty scope
        w.write_string("Person");
        w.write_u32_le(0); // type ref to Person
        
        // Tables
        w.write_vec_len(1); // 1 table
        
        w.write_string("person"); // table name
        w.write_u32_le(0); // product_type_ref to Person
        w.write_u8(1); // primary_key: None
        w.write_vec_len(0); // indexes
        w.write_vec_len(0); // constraints
        w.write_vec_len(0); // sequences
        w.write_u8(1); // schedule: None
        w.write_u8(0); // table_type: User
        w.write_u8(0); // table_access: Public
        
        // Reducers
        w.write_vec_len(2); // 2 reducers
        
        // __init__ reducer
        w.write_string("__init__");
        w.write_u8(2); // Product type
        w.write_vec_len(0); // No params
        w.write_u8(0); // Some
        w.write_u8(1); // Lifecycle::Init
        
        // add_person reducer
        w.write_string("add_person");
        w.write_u8(2); // Product type
        w.write_vec_len(2); // 2 params
        
        // Param 1: id (u32)
        w.write_u8(0); // Some
        w.write_string("id");
        w.write_u8(9); // U32
        
        // Param 2: name (String)
        w.write_u8(0); // Some
        w.write_string("name");
        w.write_u8(15); // String
        
        w.write_u8(1); // None (no lifecycle)
        
        // types: Vec<RawTypeDefV9> (empty)
        w.write_vec_len(0);
        
        // misc_exports: Vec<RawMiscModuleExportV9> (empty)
        w.write_vec_len(0);
        
        // row_level_security: Vec<RawRowLevelSecurityDefV9> (empty)
        w.write_vec_len(0);
        
        // Write to sink
        auto& buffer = w.get_buffer();
        size_t len = buffer.size();
        _bytes_sink_write(sink, buffer.data(), &len);
        
        log_info("Module description written: " + std::to_string(len) + " bytes");
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t reducer_id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp_us,
        uint32_t args_source,
        uint32_t error_sink
    ) {
        log_info("__call_reducer__ called with id: " + std::to_string(reducer_id));
        
        switch (reducer_id) {
            case 0: // __init__
                __init__(args_source, error_sink);
                return 0;
            case 1: // add_person
                add_person(args_source, error_sink);
                return 0;
            default:
                log_info("Unknown reducer ID: " + std::to_string(reducer_id));
                return -1;
        }
    }
    
    __attribute__((export_name("__init__")))
    void __init__(uint32_t source, uint32_t sink) {
        log_info("Working module initialized!");
    }
    
    __attribute__((export_name("add_person")))
    void add_person(uint32_t source, uint32_t sink) {
        log_info("add_person called");
        
        // Read arguments
        uint8_t args[1024];
        size_t args_len = sizeof(args);
        if (_bytes_source_read(source, args, &args_len) != 0) {
            log_info("Failed to read arguments");
            return;
        }
        
        // Simple argument parsing (expecting u32 id, string name)
        if (args_len < 8) {
            log_info("Arguments too short");
            return;
        }
        
        uint32_t id = args[0] | (args[1] << 8) | (args[2] << 16) | (args[3] << 24);
        uint32_t name_len = args[4] | (args[5] << 8) | (args[6] << 16) | (args[7] << 24);
        
        if (args_len < 8 + name_len) {
            log_info("Name too short");
            return;
        }
        
        std::string name((char*)&args[8], name_len);
        
        log_info("Adding person: id=" + std::to_string(id) + ", name=" + name);
        
        // Get table ID
        uint32_t table_id = 0;
        const char* table_name = "person";
        if (_table_id_from_name((const uint8_t*)table_name, strlen(table_name), &table_id) != 0) {
            log_info("Failed to get table ID");
            return;
        }
        
        // Serialize row
        BsatnWriter writer;
        writer.write_u32_le(id);
        writer.write_string(name);
        
        // Insert
        auto& buffer = writer.get_buffer();
        size_t len = buffer.size();
        auto err = _datastore_insert_bsatn(table_id, buffer.data(), &len);
        
        if (err == 0) {
            log_info("Person added successfully");
        } else {
            log_info("Failed to add person, error: " + std::to_string(err));
        }
    }
}