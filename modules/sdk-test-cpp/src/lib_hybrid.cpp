// SpacetimeDB C++ Module - Hybrid approach
// Uses module library for WASI shims but manual module exports
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

// Import functions from SpacetimeDB
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    int16_t bytes_source_read(uint32_t source, uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, const uint8_t* row, size_t* row_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
}

// BSATN writer
class BsatnWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t v) { buffer.push_back(v); }
    
    void write_u32_le(uint32_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
        buffer.push_back((v >> 16) & 0xFF);
        buffer.push_back((v >> 24) & 0xFF);
    }
    
    void write_string(const std::string& s) {
        write_u32_le(static_cast<uint32_t>(s.length()));
        for (char c : s) buffer.push_back(static_cast<uint8_t>(c));
    }
    
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
};

// BSATN reader
class BsatnReader {
    const uint8_t* data;
    size_t len;
    size_t pos;
public:
    BsatnReader(const uint8_t* d, size_t l) : data(d), len(l), pos(0) {}
    
    uint32_t read_u32() {
        if (pos + 4 > len) return 0;
        uint32_t val = data[pos] | (data[pos+1] << 8) | (data[pos+2] << 16) | (data[pos+3] << 24);
        pos += 4;
        return val;
    }
    
    std::string read_string() {
        uint32_t str_len = read_u32();
        if (pos + str_len > len) return "";
        std::string result(reinterpret_cast<const char*>(data + pos), str_len);
        pos += str_len;
        return result;
    }
};

// Helper to write to bytes sink
void write_to_sink(uint32_t sink, const uint8_t* data, size_t len) {
    size_t written = len;
    bytes_sink_write(sink, data, &written);
}

// Logging helper
void log_info(const std::string& msg) {
    console_log(2, // INFO level
               reinterpret_cast<const uint8_t*>("module"), 6,
               reinterpret_cast<const uint8_t*>(__FILE__), sizeof(__FILE__) - 1,
               __LINE__,
               reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length());
}

// Module exports
extern "C" {

// Describe module - module with one table and multiple reducers
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    BsatnWriter w;
    
    // RawModuleDef::V9 (variant 1)
    w.write_u8(1);
    
    // Typespace with 1 type (for our table)
    w.write_u32_le(1);
    
    // Type 0: ProductType for User table
    // AlgebraicType::Product (variant 2)
    w.write_u8(2);
    
    // ProductType with 2 elements
    w.write_u32_le(2);
    
    // Element 0: id (u32)
    w.write_u8(0); // Some
    w.write_string("id");
    w.write_u8(9); // U32
    
    // Element 1: name (String)
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    
    // 1 table
    w.write_u32_le(1);
    
    // Table: User
    w.write_string("User");
    w.write_u32_le(0);  // product_type_ref = 0
    w.write_u32_le(1);  // primary_key = [0]
    w.write_u32_le(0);  // column 0 is pk
    w.write_u32_le(0);  // indexes = empty
    w.write_u32_le(0);  // constraints = empty  
    w.write_u32_le(0);  // sequences = empty
    w.write_u8(1);      // schedule = None
    w.write_u8(1);      // table_type = User
    w.write_u8(0);      // table_access = Public
    
    // 3 reducers
    w.write_u32_le(3);
    
    // Reducer 0: "test_stdlib"
    w.write_string("test_stdlib");
    w.write_u32_le(0); // params: empty ProductType
    w.write_u8(1); // lifecycle: None
    
    // Reducer 1: "add_user"
    w.write_string("add_user");
    // params: ProductType with 2 elements
    w.write_u32_le(2);
    w.write_u8(0); // Some
    w.write_string("id");
    w.write_u8(9); // U32
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    w.write_u8(1); // lifecycle: None
    
    // Reducer 2: "list_users"
    w.write_string("list_users");
    w.write_u32_le(0); // params: empty ProductType
    w.write_u8(1); // lifecycle: None
    
    // Empty arrays for the rest
    w.write_u32_le(0); // types
    w.write_u32_le(0); // misc_exports
    w.write_u32_le(0); // row_level_security
    
    auto buffer = w.take_buffer();
    write_to_sink(sink, buffer.data(), buffer.size());
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
    if (id == 0) { // test_stdlib reducer
        // Demonstrate C++ standard library features working with WASI shims
        
        // 1. Using std::string
        std::string base_message = "C++ Standard Library Demo:";
        
        // 2. Using std::vector
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        
        // 3. Using std::algorithm
        std::transform(numbers.begin(), numbers.end(), numbers.begin(), 
                      [](int n) { return n * n; });
        
        // 4. Using std::stringstream
        std::stringstream ss;
        ss << base_message << " squares = [";
        for (size_t i = 0; i < numbers.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << numbers[i];
        }
        ss << "]";
        
        // 5. Using std::cout (through WASI shim)
        std::cout << "Debug: " << ss.str() << std::endl;
        
        // Log the result
        log_info(ss.str());
        
        return 0; // Success
    }
    else if (id == 1) { // add_user reducer
        // Read arguments
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        BsatnReader reader(buffer, len);
        uint32_t user_id = reader.read_u32();
        std::string user_name = reader.read_string();
        
        // Get table ID
        uint32_t table_id = 0;
        const std::string table_name = "User";
        if (table_id_from_name(reinterpret_cast<const uint8_t*>(table_name.c_str()), 
                               table_name.length(), &table_id) != 0) {
            log_info("Failed to get table ID");
            return -1;
        }
        
        // Serialize row
        BsatnWriter w;
        w.write_u32_le(user_id);
        w.write_string(user_name);
        
        auto row_buffer = w.take_buffer();
        size_t insert_len = row_buffer.size();
        
        // Insert into table
        uint16_t err = datastore_insert_bsatn(table_id, row_buffer.data(), &insert_len);
        if (err == 0) {
            std::stringstream msg;
            msg << "Added user: " << user_id << " - " << user_name;
            log_info(msg.str());
        } else {
            log_info("Failed to insert user");
            return -2;
        }
        
        return 0;
    }
    else if (id == 2) { // list_users reducer
        log_info("List users reducer called - table iteration not yet implemented");
        return 0;
    }
    
    return -999; // No such reducer
}

} // extern "C"