// Complete SpacetimeDB C++ Module
// Demonstrates tables, reducers, and C++ standard library usage
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <numeric>

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
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_table_scan_bsatn")))
    uint32_t datastore_table_scan_bsatn(uint32_t table_id);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_query_bsatn")))
    uint32_t datastore_query_bsatn(const uint8_t* query, size_t query_len);
}

// Note: WASI stubs would go here but conflict with Emscripten's headers
// For now, we'll avoid using iostream and other features that need WASI

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
public:
    const uint8_t* data;
    size_t len;
    size_t pos;
    
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

// Describe module
__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t sink) {
    BsatnWriter w;
    
    // RawModuleDef::V9 (variant 1)
    w.write_u8(1);
    
    // Typespace with 2 types
    w.write_u32_le(2);
    
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
    
    // Type 1: ProductType for Product table
    // AlgebraicType::Product (variant 2)
    w.write_u8(2);
    // ProductType with 3 elements
    w.write_u32_le(3);
    // Element 0: id (u32)
    w.write_u8(0); // Some
    w.write_string("id");
    w.write_u8(9); // U32
    // Element 1: name (String)
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    // Element 2: price (f64)
    w.write_u8(0); // Some
    w.write_string("price");
    w.write_u8(15); // F64
    
    // 2 tables
    w.write_u32_le(2);
    
    // Table 0: User
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
    
    // Table 1: Product
    w.write_string("Product");
    w.write_u32_le(1);  // product_type_ref = 1
    w.write_u32_le(1);  // primary_key = [0]
    w.write_u32_le(0);  // column 0 is pk
    w.write_u32_le(0);  // indexes = empty
    w.write_u32_le(0);  // constraints = empty  
    w.write_u32_le(0);  // sequences = empty
    w.write_u8(1);      // schedule = None
    w.write_u8(1);      // table_type = User
    w.write_u8(0);      // table_access = Public
    
    // 4 reducers
    w.write_u32_le(4);
    
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
    
    // Reducer 2: "add_product"
    w.write_string("add_product");
    // params: ProductType with 3 elements
    w.write_u32_le(3);
    w.write_u8(0); // Some
    w.write_string("id");
    w.write_u8(9); // U32
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    w.write_u8(0); // Some
    w.write_string("price");
    w.write_u8(15); // F64
    w.write_u8(1); // lifecycle: None
    
    // Reducer 3: "list_products"
    w.write_string("list_products");
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
        // Demonstrate C++ standard library features
        
        // 1. Using std::string
        std::string message = "C++ Standard Library Test: ";
        
        // 2. Using std::vector and algorithms
        std::vector<int> numbers = {5, 2, 8, 1, 9, 3};
        std::sort(numbers.begin(), numbers.end());
        
        // 3. Using std::stringstream
        std::stringstream ss;
        ss << message << "sorted = [";
        for (size_t i = 0; i < numbers.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << numbers[i];
        }
        ss << "]";
        
        log_info(ss.str());
        
        // 4. More STL features
        auto sum = std::accumulate(numbers.begin(), numbers.end(), 0);
        log_info("Sum of numbers: " + std::to_string(sum));
        
        return 0;
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
            log_info("Added user: " + std::to_string(user_id) + " - " + user_name);
        } else {
            log_info("Failed to insert user");
            return -2;
        }
        
        return 0;
    }
    else if (id == 2) { // add_product reducer
        // Read arguments
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        BsatnReader reader(buffer, len);
        uint32_t product_id = reader.read_u32();
        std::string product_name = reader.read_string();
        
        // Read f64 (8 bytes)
        double price = 0.0;
        if (reader.pos + 8 <= reader.len) {
            uint64_t price_bits = 0;
            for (int i = 0; i < 8; i++) {
                price_bits |= static_cast<uint64_t>(reader.data[reader.pos + i]) << (i * 8);
            }
            price = *reinterpret_cast<double*>(&price_bits);
        }
        
        // Get table ID
        uint32_t table_id = 0;
        const std::string table_name = "Product";
        if (table_id_from_name(reinterpret_cast<const uint8_t*>(table_name.c_str()), 
                               table_name.length(), &table_id) != 0) {
            log_info("Failed to get table ID");
            return -1;
        }
        
        // Serialize row
        BsatnWriter w;
        w.write_u32_le(product_id);
        w.write_string(product_name);
        // Write f64
        uint64_t price_bits = *reinterpret_cast<uint64_t*>(&price);
        for (int i = 0; i < 8; i++) {
            w.write_u8((price_bits >> (i * 8)) & 0xFF);
        }
        
        auto row_buffer = w.take_buffer();
        size_t insert_len = row_buffer.size();
        
        // Insert into table
        uint16_t err = datastore_insert_bsatn(table_id, row_buffer.data(), &insert_len);
        if (err == 0) {
            std::stringstream msg;
            msg << "Added product: " << product_id << " - " << product_name << " ($" << price << ")";
            log_info(msg.str());
        } else {
            log_info("Failed to insert product");
            return -2;
        }
        
        return 0;
    }
    else if (id == 3) { // list_products reducer
        log_info("Listing all products...");
        
        // Get table ID
        uint32_t table_id = 0;
        const std::string table_name = "Product";
        if (table_id_from_name(reinterpret_cast<const uint8_t*>(table_name.c_str()), 
                               table_name.length(), &table_id) != 0) {
            log_info("Failed to get table ID");
            return -1;
        }
        
        // Note: Table iteration would require more SpacetimeDB APIs
        log_info("Table scan functionality not fully implemented in this example");
        
        return 0;
    }
    
    return -999; // No such reducer
}

} // extern "C"