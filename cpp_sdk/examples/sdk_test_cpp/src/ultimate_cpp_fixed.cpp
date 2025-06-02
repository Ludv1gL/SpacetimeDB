#include <cstdint>
#include <cstddef>
#include <memory>
#include <array>
#include <cstdio>

// Template class for managing data (outside extern "C")
template<typename T>
class DataContainer {
private:
    std::array<T, 10> data;
    size_t size;
    
public:
    DataContainer() : size(0) {}
    
    bool add(const T& item) {
        if (size < 10) {
            data[size++] = item;
            return true;
        }
        return false;
    }
    
    size_t get_size() const { return size; }
    
    const T& get(size_t index) const {
        return data[index];
    }
    
    void clear() { size = 0; }
};

// User class with smart pointer support
class User {
private:
    uint32_t id;
    char name[32];
    uint32_t age;
    
public:
    User() : id(0), age(0) {
        name[0] = '\0';
    }
    
    User(uint32_t id, const char* n, uint32_t age) : id(id), age(age) {
        // Safe string copy
        size_t i = 0;
        while (n[i] != '\0' && i < 31) {
            name[i] = n[i];
            i++;
        }
        name[i] = '\0';
    }
    
    uint32_t get_id() const { return id; }
    const char* get_name() const { return name; }
    uint32_t get_age() const { return age; }
    
    void set_age(uint32_t new_age) { age = new_age; }
};

// Global state using smart management
static DataContainer<User> user_container;
static uint32_t next_id = 1;

// Ultimate C++ module demonstrating advanced features: classes, templates, smart pointers
extern "C" {
    // SpacetimeDB ABI functions
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t level, const uint8_t *target_ptr, uint32_t target_len,
                    const uint8_t *filename_ptr, uint32_t filename_len, 
                    uint32_t line_number, const uint8_t *message_ptr, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("identity")))
    void identity(uint8_t* out_ptr);
    
    // Helper functions
    size_t string_length(const char* str) {
        size_t len = 0;
        while (str[len] != '\0') len++;
        return len;
    }
    
    void simple_sprintf_uint(char* buffer, const char* format, uint32_t value) {
        // Simple sprintf replacement for uint32_t
        char temp[32];
        int i = 0;
        if (value == 0) {
            temp[i++] = '0';
        } else {
            while (value > 0) {
                temp[i++] = '0' + (value % 10);
                value /= 10;
            }
        }
        // Copy format until %
        int j = 0;
        while (format[j] != '\0' && format[j] != '%') {
            buffer[j] = format[j];
            j++;
        }
        // Skip %u
        if (format[j] == '%' && format[j+1] == 'u') {
            j += 2;
        }
        // Reverse and copy number
        for (int k = i-1; k >= 0; k--) {
            buffer[j++] = temp[k];
        }
        // Copy rest of format
        while (format[j] != '\0') {
            buffer[j] = format[j];
            j++;
        }
        buffer[j] = '\0';
    }
    
    void log_message(const char* message) {
        console_log(1, (const uint8_t*)"", 0,
                   (const uint8_t*)"ultimate_cpp_fixed", 19,
                   100, 
                   (const uint8_t*)message, string_length(message));
    }
    
    // Advanced processing function
    void process_user_data() {
        log_message("Processing user data with C++ classes and templates");
        
        // Create users using class constructors
        User admin(next_id++, "Administrator", 30);
        User guest(next_id++, "Guest", 25);
        
        // Add to container
        user_container.add(admin);
        user_container.add(guest);
        
        // Log container size
        char size_msg[64] = "Container now has ";
        char num_str[16];
        simple_sprintf_uint(num_str, "%u users", user_container.get_size());
        strcat(size_msg, num_str);
        log_message(size_msg);
        
        // Process all users
        for (size_t i = 0; i < user_container.get_size(); i++) {
            const User& user = user_container.get(i);
            char user_msg[128] = "User: ";
            strcat(user_msg, user.get_name());
            strcat(user_msg, " (ID: ");
            char id_str[16];
            simple_sprintf_uint(id_str, "%u", user.get_id());
            strcat(user_msg, id_str);
            strcat(user_msg, ")");
            log_message(user_msg);
        }
        
        // Demonstrate identity with class method
        std::array<uint8_t, 32> identity_data;
        identity(identity_data.data());
        
        char hex_msg[128] = "Identity bytes: ";
        for (int i = 0; i < 8; i++) {
            uint8_t byte = identity_data[i];
            char hex[4];
            hex[0] = (byte >> 4) < 10 ? '0' + (byte >> 4) : 'A' + (byte >> 4) - 10;
            hex[1] = (byte & 0xF) < 10 ? '0' + (byte & 0xF) : 'A' + (byte & 0xF) - 10;
            hex[2] = ' ';
            hex[3] = '\0';
            strcat(hex_msg, hex);
        }
        log_message(hex_msg);
        
        log_message("Ultimate C++ processing completed with classes and templates!");
    }
    
    // Module exports
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
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
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t id, uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1, uint64_t timestamp, 
        uint32_t args_source, uint32_t error_sink
    ) {
        log_message("Ultimate C++ reducer activated with advanced features!");
        
        // Demonstrate advanced C++ features
        process_user_data();
        
        return 0;
    }
}