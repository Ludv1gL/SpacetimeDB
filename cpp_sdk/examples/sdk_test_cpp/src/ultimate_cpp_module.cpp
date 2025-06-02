#include <cstdint>
#include <cstddef>
#include <memory>
#include <array>

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
    
    // Template class for managing data
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
    
    // Service class using RAII pattern
    class LoggerService {
    private:
        const char* module_name;
        uint32_t log_count;
        
    public:
        LoggerService(const char* name) : module_name(name), log_count(0) {}
        
        void log(const char* message) {
            console_log(1, (const uint8_t*)"", 0,
                       (const uint8_t*)module_name, 20, // strlen("ultimate_cpp_module")
                       100 + log_count, 
                       (const uint8_t*)message, string_length(message));
            log_count++;
        }
        
        uint32_t get_log_count() const { return log_count; }
        
    private:
        size_t string_length(const char* str) {
            size_t len = 0;
            while (str[len] != '\0') len++;
            return len;
        }
    };
    
    // Global state using smart management
    static DataContainer<User> user_container;
    static uint32_t next_id = 1;
    
    // Template function for serialization
    template<typename T>
    void serialize_to_buffer(const T& value, uint8_t* buffer, size_t& offset) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
        for (size_t i = 0; i < sizeof(T); i++) {
            buffer[offset++] = bytes[i];
        }
    }
    
    // Advanced processing function
    void process_user_data() {
        LoggerService logger("ultimate_cpp_module");
        
        // Demonstrate template usage
        logger.log("Processing user data with templates");
        
        // Create users using class constructors
        User admin(next_id++, "Administrator", 30);
        User guest(next_id++, "Guest", 25);
        
        // Add to container
        user_container.add(admin);
        user_container.add(guest);
        
        // Log container size
        char size_msg[64];
        snprintf(size_msg, sizeof(size_msg), "Container now has %zu users", user_container.get_size());
        logger.log(size_msg);
        
        // Process all users
        for (size_t i = 0; i < user_container.get_size(); i++) {
            const User& user = user_container.get(i);
            char user_msg[128];
            snprintf(user_msg, sizeof(user_msg), "User %u: %s (age %u)", 
                    user.get_id(), user.get_name(), user.get_age());
            logger.log(user_msg);
        }
        
        // Demonstrate identity with class method
        std::array<uint8_t, 32> identity_data;
        identity(identity_data.data());
        
        char hex_msg[128] = "Identity bytes: ";
        for (int i = 0; i < 8; i++) {
            char hex[4];
            snprintf(hex, sizeof(hex), "%02X ", identity_data[i]);
            strncat(hex_msg, hex, sizeof(hex_msg) - strlen(hex_msg) - 1);
        }
        logger.log(hex_msg);
        
        // Log final stats
        char final_msg[64];
        snprintf(final_msg, sizeof(final_msg), "Logged %u messages total", logger.get_log_count());
        logger.log(final_msg);
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
        LoggerService logger("ultimate_cpp_module");
        logger.log("Ultimate C++ reducer activated!");
        
        // Demonstrate advanced C++ features
        process_user_data();
        
        logger.log("Ultimate C++ processing completed successfully!");
        return 0;
    }
}