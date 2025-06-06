// Module using direct FFI approach - no library dependency
#include <cstdint>
#include <cstring>
#include <cstdio>

// Direct FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t log_level, const uint8_t* target, uint32_t target_len,
                     const uint8_t* filename, uint32_t filename_len, uint32_t line_number,
                     const uint8_t* message, uint32_t message_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, const uint8_t* row, size_t len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* data, size_t* len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* buffer, size_t* len);
}

// BSATN AlgebraicType tags
enum {
    TYPE_PRODUCT = 2,
    TYPE_STRING = 4,
    TYPE_U8 = 7,
    TYPE_U32 = 11,
};

// Simple byte buffer for building module definition
struct ByteBuffer {
    uint8_t data[4096];
    size_t size = 0;
    
    void push(uint8_t byte) {
        data[size++] = byte;
    }
    
    void push_u16(uint16_t value) {
        push(value & 0xFF);
        push((value >> 8) & 0xFF);
    }
    
    void push_u32(uint32_t value) {
        push(value & 0xFF);
        push((value >> 8) & 0xFF);
        push((value >> 16) & 0xFF);
        push((value >> 24) & 0xFF);
    }
    
    void push_bytes(const uint8_t* bytes, size_t len) {
        memcpy(data + size, bytes, len);
        size += len;
    }
    
    void push_string(const char* str) {
        uint32_t len = strlen(str);
        push_u32(len);
        push_bytes(reinterpret_cast<const uint8_t*>(str), len);
    }
};

// Person struct
struct Person {
    uint32_t id;
    char name[64];  // Fixed size instead of std::string
    uint8_t age;
};

// Serialize Person to BSATN
size_t serialize_person(uint8_t* buffer, const Person& person) {
    size_t offset = 0;
    
    // id: u32
    memcpy(buffer + offset, &person.id, 4);
    offset += 4;
    
    // name: string
    uint32_t name_len = strlen(person.name);
    memcpy(buffer + offset, &name_len, 4);
    offset += 4;
    memcpy(buffer + offset, person.name, name_len);
    offset += name_len;
    
    // age: u8
    buffer[offset] = person.age;
    offset += 1;
    
    return offset;
}

// Table operations
void insert_person(const Person& person) {
    uint8_t buffer[1024];
    size_t len = serialize_person(buffer, person);
    datastore_insert_bsatn(0, buffer, len); // table_id = 0
}

// Logging helper
void log_message(const char* msg) {
    const char* target = "";
    const char* filename = __FILE__;
    console_log(2, // INFO level
                reinterpret_cast<const uint8_t*>(target), 0,
                reinterpret_cast<const uint8_t*>(filename), strlen(filename),
                __LINE__,
                reinterpret_cast<const uint8_t*>(msg), strlen(msg));
}

// Reducers
void add_person(uint32_t id, const char* name, uint8_t age) {
    Person person;
    person.id = id;
    strncpy(person.name, name, 63);
    person.name[63] = '\0';
    person.age = age;
    
    insert_person(person);
    
    // Log message
    char log_buf[256];
    snprintf(log_buf, sizeof(log_buf), "Added person: %s (id=%u, age=%u)", name, id, age);
    log_message(log_buf);
}

void list_people() {
    log_message("Listing all people in the table");
}

// Module exports
extern "C" {

__attribute__((export_name("__describe_module__")))
void __describe_module__(uint32_t description) {
    ByteBuffer def;
    
    // Write RawModuleDef::V9 (variant 1)
    def.push(1);
    
    // Typespace with 1 type (Person struct)
    def.push_u32(1);
    
    // Type 0: ProductType (variant 2)
    def.push(2);
    
    // ProductType with 3 elements
    def.push_u32(3);
    
    // Element 0: id (u32)
    def.push(0); // Some
    def.push_string("id");
    def.push(11); // U32 = 11
    
    // Element 1: name (string)
    def.push(0); // Some
    def.push_string("name");
    def.push(4); // String = 4
    
    // Element 2: age (u8)
    def.push(0); // Some
    def.push_string("age");
    def.push(7); // U8 = 7
    
    // Tables: 1 table (Person)
    def.push_u32(1);
    
    // Table: Person
    def.push_string("Person");
    def.push_u32(0); // product_type_ref = 0
    def.push_u32(0); // primary_key (empty)
    def.push_u32(0); // indexes (empty)
    def.push_u32(0); // constraints (empty)
    def.push_u32(0); // sequences (empty)
    def.push(1);     // schedule: None
    def.push(1);     // table_type: User
    def.push(0);     // table_access: Public
    
    // Reducers: 2 reducers
    def.push_u32(2);
    
    // Reducer 0: add_person(id: u32, name: string, age: u8)
    def.push_string("add_person");
    def.push_u32(3); // params: 3 fields
    // Field 0: id
    def.push(0); // Some
    def.push_string("id");
    def.push(11); // U32 = 11
    // Field 1: name
    def.push(0); // Some
    def.push_string("name");
    def.push(4); // String = 4
    // Field 2: age
    def.push(0); // Some
    def.push_string("age");
    def.push(7); // U8 = 7
    def.push(1); // lifecycle: None
    
    // Reducer 1: list_people()
    def.push_string("list_people");
    def.push_u32(0); // params: empty
    def.push(1);     // lifecycle: None
    
    // Empty arrays for the rest
    def.push_u32(0); // types
    def.push_u32(0); // misc_exports
    def.push_u32(0); // row_level_security
    
    // Send module definition using BytesSink  
    size_t written = def.size;
    bytes_sink_write(description, def.data, &written);
}

__attribute__((export_name("__call_reducer__")))
int16_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp,
    uint32_t args_handle,
    uint32_t error_handle
) {
    // For this simple example, we'll use a static buffer for args
    // In a real implementation, you'd use the BytesSource API
    static uint8_t args_buffer[1024];
    
    // Read args using BytesSource (simplified - assumes args fit in buffer)
    size_t args_len = sizeof(args_buffer);
    bytes_source_read(args_handle, args_buffer, &args_len);
    
    if (id == 0) { // add_person
        // Deserialize args
        uint32_t person_id;
        memcpy(&person_id, args_buffer, 4);
        
        uint32_t name_len;
        memcpy(&name_len, args_buffer + 4, 4);
        
        char name[64];
        if (name_len >= sizeof(name)) name_len = sizeof(name) - 1;
        memcpy(name, args_buffer + 8, name_len);
        name[name_len] = '\0';
        
        uint8_t age = args_buffer[8 + name_len];
        
        add_person(person_id, name, age);
    } else if (id == 1) { // list_people
        list_people();
    } else {
        return -1; // NO_SUCH_REDUCER
    }
    
    return 0; // Success
}

} // extern "C"