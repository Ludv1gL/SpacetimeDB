// Final Working SpacetimeDB C++ Module
// Demonstrates tables, reducers, and C++ features without WASI dependencies
#include <cstdint>
#include <string>
#include <vector>
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

// Simple string builder without sstream (which needs WASI)
class StringBuilder {
    std::string str;
public:
    StringBuilder& append(const std::string& s) { 
        str += s; 
        return *this;
    }
    StringBuilder& append(int n) {
        // Simple int to string conversion
        if (n == 0) {
            str += "0";
            return *this;
        }
        
        std::string num;
        int temp = n < 0 ? -n : n;
        while (temp > 0) {
            num = char('0' + (temp % 10)) + num;
            temp /= 10;
        }
        if (n < 0) num = "-" + num;
        str += num;
        return *this;
    }
    const std::string& get() const { return str; }
};

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
    w.write_u8(2); // AlgebraicType::Product
    w.write_u32_le(2); // 2 elements
    // Element 0: id (u32)
    w.write_u8(0); // Some
    w.write_string("id");
    w.write_u8(9); // U32
    // Element 1: name (String)
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    
    // Type 1: ProductType for Counter table
    w.write_u8(2); // AlgebraicType::Product
    w.write_u32_le(2); // 2 elements
    // Element 0: name (String)
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    // Element 1: value (i32)
    w.write_u8(0); // Some
    w.write_string("value");
    w.write_u8(11); // I32
    
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
    
    // Table 1: Counter
    w.write_string("Counter");
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
    
    // Reducer 0: "test_cpp_features"
    w.write_string("test_cpp_features");
    w.write_u32_le(0); // params: empty ProductType
    w.write_u8(1); // lifecycle: None
    
    // Reducer 1: "add_user"
    w.write_string("add_user");
    w.write_u32_le(2); // params: 2 elements
    w.write_u8(0); // Some
    w.write_string("id");
    w.write_u8(9); // U32
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    w.write_u8(1); // lifecycle: None
    
    // Reducer 2: "increment_counter"
    w.write_string("increment_counter");
    w.write_u32_le(1); // params: 1 element
    w.write_u8(0); // Some
    w.write_string("name");
    w.write_u8(17); // String
    w.write_u8(1); // lifecycle: None
    
    // Reducer 3: "demo_algorithms"
    w.write_string("demo_algorithms");
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
    if (id == 0) { // test_cpp_features
        log_info("Testing C++ features without WASI dependencies:");
        
        // 1. std::string operations
        std::string str1 = "Hello";
        std::string str2 = " SpacetimeDB!";
        std::string combined = str1 + str2;
        log_info("String concatenation: " + combined);
        
        // 2. std::vector operations
        std::vector<int> vec = {3, 1, 4, 1, 5, 9};
        StringBuilder sb;
        sb.append("Vector before sort: [");
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) sb.append(", ");
            sb.append(vec[i]);
        }
        sb.append("]");
        log_info(sb.get());
        
        // 3. std::sort algorithm
        std::sort(vec.begin(), vec.end());
        
        StringBuilder sb2;
        sb2.append("Vector after sort: [");
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) sb2.append(", ");
            sb2.append(vec[i]);
        }
        sb2.append("]");
        log_info(sb2.get());
        
        // 4. Lambda expressions
        int sum = 0;
        std::for_each(vec.begin(), vec.end(), [&sum](int n) { sum += n; });
        
        StringBuilder sb3;
        sb3.append("Sum of elements: ").append(sum);
        log_info(sb3.get());
        
        return 0;
    }
    else if (id == 1) { // add_user
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
            StringBuilder msg;
            msg.append("Added user: ").append(user_id).append(" - ").append(user_name);
            log_info(msg.get());
        } else {
            log_info("Failed to insert user");
            return -2;
        }
        
        return 0;
    }
    else if (id == 2) { // increment_counter
        // Read counter name
        uint8_t buffer[256];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        BsatnReader reader(buffer, len);
        std::string counter_name = reader.read_string();
        
        // Get table ID
        uint32_t table_id = 0;
        const std::string table_name = "Counter";
        if (table_id_from_name(reinterpret_cast<const uint8_t*>(table_name.c_str()), 
                               table_name.length(), &table_id) != 0) {
            log_info("Failed to get table ID");
            return -1;
        }
        
        // For now, just insert a new counter with value 1
        // (In a real implementation, we'd query and update)
        BsatnWriter w;
        w.write_string(counter_name);
        // Write i32 as 4 bytes (little-endian)
        int32_t value = 1;
        w.write_u8(value & 0xFF);
        w.write_u8((value >> 8) & 0xFF);
        w.write_u8((value >> 16) & 0xFF);
        w.write_u8((value >> 24) & 0xFF);
        
        auto row_buffer = w.take_buffer();
        size_t insert_len = row_buffer.size();
        
        uint16_t err = datastore_insert_bsatn(table_id, row_buffer.data(), &insert_len);
        if (err == 0) {
            log_info("Incremented counter: " + counter_name);
        } else {
            log_info("Failed to update counter");
            return -2;
        }
        
        return 0;
    }
    else if (id == 3) { // demo_algorithms
        log_info("Demonstrating C++ STL algorithms:");
        
        // Create a vector of strings
        std::vector<std::string> words = {"spacetime", "database", "cpp", "module", "algorithm"};
        
        // Sort the strings
        std::sort(words.begin(), words.end());
        
        StringBuilder sb;
        sb.append("Sorted words: ");
        for (size_t i = 0; i < words.size(); ++i) {
            if (i > 0) sb.append(", ");
            sb.append(words[i]);
        }
        log_info(sb.get());
        
        // Find a specific word
        auto it = std::find(words.begin(), words.end(), "database");
        if (it != words.end()) {
            log_info("Found 'database' in the vector");
        }
        
        // Transform to uppercase (manual since no locale support)
        std::vector<std::string> upper_words;
        for (const auto& word : words) {
            std::string upper;
            for (char c : word) {
                if (c >= 'a' && c <= 'z') {
                    upper += char(c - 'a' + 'A');
                } else {
                    upper += c;
                }
            }
            upper_words.push_back(upper);
        }
        
        StringBuilder sb2;
        sb2.append("Uppercase words: ");
        for (size_t i = 0; i < upper_words.size(); ++i) {
            if (i > 0) sb2.append(", ");
            sb2.append(upper_words[i]);
        }
        log_info(sb2.get());
        
        return 0;
    }
    
    return -999; // No such reducer
}

} // extern "C"