#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

// Advanced C++ module demonstrating complex features while avoiding SDK macro issues
extern "C" {
    // Core SpacetimeDB ABI functions
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(
        uint8_t level,
        const uint8_t *target_ptr, uint32_t target_len,
        const uint8_t *filename_ptr, uint32_t filename_len, 
        uint32_t line_number,
        const uint8_t *message_ptr, uint32_t message_len
    );
    
    __attribute__((import_module("spacetime_10.0"), import_name("identity")))
    void identity(uint8_t* out_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, uint32_t name_len, uint32_t* out);
    
    // Global state for our module
    struct User {
        uint32_t id;
        std::string name;
        uint32_t age;
        
        User() : id(0), age(0) {}
        User(uint32_t id, const std::string& name, uint32_t age) 
            : id(id), name(name), age(age) {}
    };
    
    // In-memory user storage (for demonstration)
    std::vector<User> users;
    uint32_t next_user_id = 1;
    
    // Helper function for logging with std::string
    void log_string(const std::string& message) {
        const char* filename = "advanced_cpp_module.cpp";
        console_log(
            1, // info level
            (const uint8_t*)"", 0,
            (const uint8_t*)filename, 22, // strlen("advanced_cpp_module.cpp")
            50, // line number
            (const uint8_t*)message.c_str(), message.length()
        );
    }
    
    // Helper function to serialize user data
    std::vector<uint8_t> serialize_user(const User& user) {
        std::vector<uint8_t> data;
        
        // Serialize ID (4 bytes, little endian)
        data.push_back(user.id & 0xFF);
        data.push_back((user.id >> 8) & 0xFF);
        data.push_back((user.id >> 16) & 0xFF);
        data.push_back((user.id >> 24) & 0xFF);
        
        // Serialize name length (4 bytes, little endian)
        uint32_t name_len = user.name.length();
        data.push_back(name_len & 0xFF);
        data.push_back((name_len >> 8) & 0xFF);
        data.push_back((name_len >> 16) & 0xFF);
        data.push_back((name_len >> 24) & 0xFF);
        
        // Serialize name data
        for (char c : user.name) {
            data.push_back(static_cast<uint8_t>(c));
        }
        
        // Serialize age (4 bytes, little endian)
        data.push_back(user.age & 0xFF);
        data.push_back((user.age >> 8) & 0xFF);
        data.push_back((user.age >> 16) & 0xFF);
        data.push_back((user.age >> 24) & 0xFF);
        
        return data;
    }
    
    // Create module definition with user table
    void create_user_module_definition(uint8_t* buffer, size_t* len) {
        // Create a simple module with minimal structure
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            0, 0, 0, 0,     // tables (empty vector)  
            0, 0, 0, 0,     // reducers (empty vector)
            0, 0, 0, 0,     // types (empty vector)
            0, 0, 0, 0,     // misc_exports (empty vector)
            0, 0, 0, 0      // row_level_security (empty vector)
        };
        
        size_t data_size = sizeof(data);
        if (*len >= data_size) {
            for (size_t i = 0; i < data_size; i++) {
                buffer[i] = data[i];
            }
            *len = data_size;
        } else {
            *len = 0;
        }
    }
    
    // Module exports
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        
        create_user_module_definition(buffer, &len);
        
        if (len > 0) {
            bytes_sink_write(sink, buffer, &len);
        }
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp, 
        uint32_t args_source, 
        uint32_t error_sink
    ) {
        // Demonstrate advanced C++ functionality
        log_string("Advanced C++ reducer called!");
        
        // Create a test user using C++ features
        User test_user(next_user_id++, "TestUser" + std::to_string(next_user_id), 25);
        users.push_back(test_user);
        
        log_string("Created user: " + test_user.name + " (ID: " + std::to_string(test_user.id) + ")");
        
        // Demonstrate vector operations
        log_string("Total users in memory: " + std::to_string(users.size()));
        
        // Demonstrate serialization
        auto serialized = serialize_user(test_user);
        log_string("Serialized user data length: " + std::to_string(serialized.size()) + " bytes");
        
        // Demonstrate identity function with vector
        std::vector<uint8_t> identity_data(32);
        identity(identity_data.data());
        
        // Create hex string from identity
        std::string hex_string = "Identity: ";
        for (int i = 0; i < 8; i++) {
            uint8_t byte = identity_data[i];
            char hex[3];
            hex[0] = (byte >> 4) < 10 ? '0' + (byte >> 4) : 'A' + (byte >> 4) - 10;
            hex[1] = (byte & 0xF) < 10 ? '0' + (byte & 0xF) : 'A' + (byte & 0xF) - 10;
            hex[2] = '\0';
            hex_string += hex;
        }
        
        log_string(hex_string);
        
        // Find users by age (demonstrate STL algorithms)
        uint32_t young_users = 0;
        for (const auto& user : users) {
            if (user.age < 30) {
                young_users++;
            }
        }
        
        log_string("Young users (age < 30): " + std::to_string(young_users));
        
        return 0; // success
    }
}