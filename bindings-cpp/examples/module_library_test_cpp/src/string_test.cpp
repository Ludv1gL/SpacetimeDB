#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

// Minimal BSATN writer
class MinimalWriter {
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
        write_u32_le(s.length());
        for (char c : s) buffer.push_back(c);
    }
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
};

// Minimal BSATN reader
class MinimalReader {
    const uint8_t* data;
    size_t pos;
    size_t len;
public:
    MinimalReader(const uint8_t* d, size_t l) : data(d), pos(0), len(l) {}
    
    uint8_t read_u8() {
        if (pos >= len) return 0;
        return data[pos++];
    }
    
    uint32_t read_u32_le() {
        if (pos + 4 > len) return 0;
        uint32_t result = 0;
        for (int i = 0; i < 4; i++) {
            result |= (uint32_t(data[pos++]) << (i * 8));
        }
        return result;
    }
    
    std::string read_string() {
        uint32_t str_len = read_u32_le();
        if (pos + str_len > len) return "";
        std::string result(reinterpret_cast<const char*>(data + pos), str_len);
        pos += str_len;
        return result;
    }
    
    bool has_more() const { return pos < len; }
    size_t remaining() const { return len - pos; }
};

// FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    int16_t bytes_source_read(uint32_t source, uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row, size_t* row_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("table_id_from_name")))
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
}

// Simple logging
void log_info(const std::string& msg) {
    const char* filename = "string_test.cpp";
    console_log(2, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg.c_str(), msg.length());
}

// AlgebraicType tags
enum class AlgebraicTypeTag : uint8_t {
    Ref = 0, Sum = 1, Product = 2, Array = 3, String = 4,
    Bool = 5, I8 = 6, U8 = 7, I16 = 8, U16 = 9,
    I32 = 10, U32 = 11, I64 = 12, U64 = 13,
    I128 = 14, U128 = 15, I256 = 16, U256 = 17,
    F32 = 18, F64 = 19
};

// Helper to write a field definition
void write_field(MinimalWriter& w, const std::string& name, AlgebraicTypeTag type) {
    w.write_u8(0); // Some (field name present)
    w.write_string(name);
    w.write_u8(static_cast<uint8_t>(type));
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        MinimalWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace - 2 types
        w.write_u32_le(2);
        
        // Type 0: OneString
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(1); // 1 field
        write_field(w, "s", AlgebraicTypeTag::String);
        
        // Type 1: Person (id: u32, name: String, age: u8)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(3); // 3 fields
        write_field(w, "id", AlgebraicTypeTag::U32);
        write_field(w, "name", AlgebraicTypeTag::String);
        write_field(w, "age", AlgebraicTypeTag::U8);
        
        // Tables
        w.write_u32_le(2);
        
        // Table: one_string
        w.write_string("one_string");
        w.write_u32_le(0); // type_ref = 0
        w.write_u32_le(0); // primary_key = empty vec
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1);     // schedule = None
        w.write_u8(1);     // table_type = User
        w.write_u8(0);     // table_access = Public
        
        // Table: person
        w.write_string("person");
        w.write_u32_le(1); // type_ref = 1
        w.write_u32_le(0); // primary_key = empty vec
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1);     // schedule = None
        w.write_u8(1);     // table_type = User
        w.write_u8(0);     // table_access = Public
        
        // Reducers
        w.write_u32_le(2);
        
        // Reducer: insert_one_string
        w.write_string("insert_one_string");
        w.write_u32_le(1); // 1 param
        write_field(w, "s", AlgebraicTypeTag::String);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_person
        w.write_string("insert_person");
        w.write_u32_le(3); // 3 params
        write_field(w, "id", AlgebraicTypeTag::U32);
        write_field(w, "name", AlgebraicTypeTag::String);
        write_field(w, "age", AlgebraicTypeTag::U8);
        w.write_u8(1); // lifecycle = None
        
        // Empty types, misc exports, row level security
        w.write_u32_le(0);
        w.write_u32_le(0);
        w.write_u32_le(0);
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        bytes_sink_write(sink, buffer.data(), &len);
    }
    
    // Helper to insert into a table
    bool insert_value(const std::string& table_name, MinimalWriter& row_writer) {
        uint32_t table_id = 0;
        if (table_id_from_name((const uint8_t*)table_name.c_str(), table_name.length(), &table_id) != 0) {
            log_info("Failed to get table ID for " + table_name);
            return false;
        }
        
        auto buffer = row_writer.take_buffer();
        size_t len = buffer.size();
        
        uint16_t err = datastore_insert_bsatn(table_id, buffer.data(), &len);
        if (err == 0) {
            log_info("Successfully inserted into " + table_name);
            return true;
        } else {
            log_info("Failed to insert into " + table_name);
            return false;
        }
    }
    
    void insert_one_string(uint32_t args, uint32_t error) {
        log_info("insert_one_string called");
        
        // Read the entire args buffer
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        MinimalReader reader(buffer, len);
        std::string value = reader.read_string();
        
        log_info("Inserting string: " + value);
        
        MinimalWriter w;
        w.write_string(value);
        insert_value("one_string", w);
    }
    
    void insert_person(uint32_t args, uint32_t error) {
        log_info("insert_person called");
        
        // Read the entire args buffer
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        MinimalReader reader(buffer, len);
        uint32_t id = reader.read_u32_le();
        std::string name = reader.read_string();
        uint8_t age = reader.read_u8();
        
        log_info("Inserting person: id=" + std::to_string(id) + ", name=" + name + ", age=" + std::to_string(age));
        
        MinimalWriter w;
        w.write_u32_le(id);
        w.write_string(name);
        w.write_u8(age);
        insert_value("person", w);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id,
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        switch (id) {
            case 0: insert_one_string(args, error); return 0;
            case 1: insert_person(args, error); return 0;
            default: return -1;
        }
    }
}