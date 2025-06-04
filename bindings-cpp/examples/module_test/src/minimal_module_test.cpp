#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

// Basic BSATN writer
class MinimalWriter {
    std::vector<uint8_t> buffer;
public:
    void write_u8(uint8_t v) { buffer.push_back(v); }
    void write_u16_le(uint16_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
    }
    void write_u32_le(uint32_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
        buffer.push_back((v >> 16) & 0xFF);
        buffer.push_back((v >> 24) & 0xFF);
    }
    void write_u64_le(uint64_t v) {
        for (int i = 0; i < 8; i++) {
            buffer.push_back((v >> (i * 8)) & 0xFF);
        }
    }
    void write_i64_le(int64_t v) { 
        write_u64_le(static_cast<uint64_t>(v)); 
    }
    void write_string(const std::string& s) {
        write_u32_le(s.length());
        for (char c : s) buffer.push_back(c);
    }
    void write_bytes(const uint8_t* data, size_t len) {
        write_u32_le(len);
        for (size_t i = 0; i < len; i++) {
            buffer.push_back(data[i]);
        }
    }
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
};

// BSATN reader
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
    
    uint16_t read_u16_le() {
        if (pos + 2 > len) return 0;
        uint16_t result = 0;
        for (int i = 0; i < 2; i++) {
            result |= (uint16_t(data[pos++]) << (i * 8));
        }
        return result;
    }
    
    uint32_t read_u32_le() {
        if (pos + 4 > len) return 0;
        uint32_t result = 0;
        for (int i = 0; i < 4; i++) {
            result |= (uint32_t(data[pos++]) << (i * 8));
        }
        return result;
    }
    
    uint64_t read_u64_le() {
        if (pos + 8 > len) return 0;
        uint64_t result = 0;
        for (int i = 0; i < 8; i++) {
            result |= (uint64_t(data[pos++]) << (i * 8));
        }
        return result;
    }
    
    int16_t read_i16_le() {
        return static_cast<int16_t>(read_u16_le());
    }
    
    int32_t read_i32_le() {
        return static_cast<int32_t>(read_u32_le());
    }
    
    int64_t read_i64_le() {
        return static_cast<int64_t>(read_u64_le());
    }
    
    std::string read_string() {
        uint32_t str_len = read_u32_le();
        if (pos + str_len > len) return "";
        std::string result(reinterpret_cast<const char*>(data + pos), str_len);
        pos += str_len;
        return result;
    }
    
    std::vector<uint8_t> read_bytes() {
        uint32_t vec_len = read_u32_le();
        if (pos + vec_len > len) return {};
        std::vector<uint8_t> result(data + pos, data + pos + vec_len);
        pos += vec_len;
        return result;
    }
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
    const char* filename = "minimal_module_test.cpp";
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

// Write identity type (U256)
void write_identity(MinimalWriter& w) {
    // Identity is a newtype wrapper around U256
    w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
    w.write_u32_le(1); // 1 field
    write_field(w, "__identity__", AlgebraicTypeTag::U256);
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        MinimalWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace - core types from module-test
        w.write_u32_le(6); // 6 types
        
        // Type 0: Person (id: u32, name: String, age: u8)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(3); // 3 fields
        write_field(w, "id", AlgebraicTypeTag::U32);
        write_field(w, "name", AlgebraicTypeTag::String);
        write_field(w, "age", AlgebraicTypeTag::U8);
        
        // Type 1: TestA (x: u64, y: u32, z: u16)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(3); // 3 fields
        write_field(w, "x", AlgebraicTypeTag::U64);
        write_field(w, "y", AlgebraicTypeTag::U32);
        write_field(w, "z", AlgebraicTypeTag::U16);
        
        // Type 2: Identity (wrapper for U256)
        write_identity(w);
        
        // Type 3: Point (x: i64, y: i64)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(2); // 2 fields
        write_field(w, "x", AlgebraicTypeTag::I64);
        write_field(w, "y", AlgebraicTypeTag::I64);
        
        // Type 4: SimpleTable (id: Identity, data: u8)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(2); // 2 fields
        w.write_u8(0); // Some
        w.write_string("id");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Ref));
        w.write_u32_le(2); // Ref to Identity
        write_field(w, "data", AlgebraicTypeTag::U8);
        
        // Type 5: EveryPrimitiveStruct (subset for testing)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(11); // 11 fields
        w.write_u8(0); // Some
        w.write_string("id");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Ref));
        w.write_u32_le(2); // Ref to Identity
        write_field(w, "a_u8", AlgebraicTypeTag::U8);
        write_field(w, "a_u16", AlgebraicTypeTag::U16);
        write_field(w, "a_u32", AlgebraicTypeTag::U32);
        write_field(w, "a_u64", AlgebraicTypeTag::U64);
        write_field(w, "a_i8", AlgebraicTypeTag::I8);
        write_field(w, "a_i16", AlgebraicTypeTag::I16);
        write_field(w, "a_i32", AlgebraicTypeTag::I32);
        write_field(w, "a_i64", AlgebraicTypeTag::I64);
        write_field(w, "a_bool", AlgebraicTypeTag::Bool);
        write_field(w, "a_string", AlgebraicTypeTag::String);
        
        // Tables
        w.write_u32_le(5); // 5 tables
        
        // Table: person (public)
        w.write_string("person");
        w.write_u32_le(0); // type_ref = 0
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Table: test_a (public)
        w.write_string("test_a");
        w.write_u32_le(1); // type_ref = 1
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Table: point (public)
        w.write_string("point");
        w.write_u32_le(3); // type_ref = 3
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Table: simple_table (private)
        w.write_string("simple_table");
        w.write_u32_le(4); // type_ref = 4
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(1); // table_access = Private
        
        // Table: every_primitive_struct (private)
        w.write_string("every_primitive_struct");
        w.write_u32_le(5); // type_ref = 5
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(1); // table_access = Private
        
        // Reducers
        w.write_u32_le(6); // 6 reducers
        
        // Reducer: init
        w.write_string("init");
        w.write_u32_le(0); // 0 params
        w.write_u8(0); // lifecycle = Some(Init)
        w.write_u8(0); // Init variant
        
        // Reducer: insert_person
        w.write_string("insert_person");
        w.write_u32_le(2); // 2 params
        write_field(w, "name", AlgebraicTypeTag::String);
        write_field(w, "age", AlgebraicTypeTag::U8);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_test_a
        w.write_string("insert_test_a");
        w.write_u32_le(3); // 3 params
        write_field(w, "x", AlgebraicTypeTag::U64);
        write_field(w, "y", AlgebraicTypeTag::U32);
        write_field(w, "z", AlgebraicTypeTag::U16);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_point
        w.write_string("insert_point");
        w.write_u32_le(2); // 2 params
        write_field(w, "x", AlgebraicTypeTag::I64);
        write_field(w, "y", AlgebraicTypeTag::I64);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_simple_table
        w.write_string("insert_simple_table");
        w.write_u32_le(1); // 1 param
        write_field(w, "data", AlgebraicTypeTag::U8);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_every_primitive
        w.write_string("insert_every_primitive");
        w.write_u32_le(10); // 10 params (subset)
        write_field(w, "a_u8", AlgebraicTypeTag::U8);
        write_field(w, "a_u16", AlgebraicTypeTag::U16);
        write_field(w, "a_u32", AlgebraicTypeTag::U32);
        write_field(w, "a_u64", AlgebraicTypeTag::U64);
        write_field(w, "a_i8", AlgebraicTypeTag::I8);
        write_field(w, "a_i16", AlgebraicTypeTag::I16);
        write_field(w, "a_i32", AlgebraicTypeTag::I32);
        write_field(w, "a_i64", AlgebraicTypeTag::I64);
        write_field(w, "a_bool", AlgebraicTypeTag::Bool);
        write_field(w, "a_string", AlgebraicTypeTag::String);
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
    
    // Identity helper - writes 32 bytes of zeros
    void write_identity_value(MinimalWriter& w) {
        for (int i = 0; i < 32; i++) {
            w.write_u8(0);
        }
    }
    
    void init_reducer() {
        log_info("init reducer called");
        
        // Insert some initial data
        MinimalWriter w;
        w.write_u32_le(1);  // id
        w.write_string("Alice");
        w.write_u8(30);  // age
        insert_value("person", w);
        
        MinimalWriter w2;
        w2.write_u32_le(2);  // id
        w2.write_string("Bob");
        w2.write_u8(25);  // age
        insert_value("person", w2);
    }
    
    void insert_person(uint32_t args) {
        log_info("insert_person called");
        
        // Read args
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        MinimalReader reader(buffer, len);
        std::string name = reader.read_string();
        uint8_t age = reader.read_u8();
        
        log_info("Inserting person: " + name + ", age=" + std::to_string(age));
        
        // Insert with id=0 (would be auto-incremented in real SDK)
        MinimalWriter w;
        w.write_u32_le(0);  // id
        w.write_string(name);
        w.write_u8(age);
        insert_value("person", w);
    }
    
    void insert_test_a(uint32_t args) {
        log_info("insert_test_a called");
        
        // Read args
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        MinimalReader reader(buffer, len);
        uint64_t x = reader.read_u64_le();
        uint32_t y = reader.read_u32_le();
        uint16_t z = reader.read_u16_le(); // Read as u16
        
        log_info("Inserting test_a: x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", z=" + std::to_string(z));
        
        MinimalWriter w;
        w.write_u64_le(x);
        w.write_u32_le(y);
        w.write_u16_le(z);
        insert_value("test_a", w);
    }
    
    void insert_point(uint32_t args) {
        log_info("insert_point called");
        
        // Read args
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        MinimalReader reader(buffer, len);
        int64_t x = reader.read_i64_le();
        int64_t y = reader.read_i64_le();
        
        log_info("Inserting point: x=" + std::to_string(x) + ", y=" + std::to_string(y));
        
        MinimalWriter w;
        w.write_i64_le(x);
        w.write_i64_le(y);
        insert_value("point", w);
    }
    
    void insert_simple_table(uint32_t args) {
        log_info("insert_simple_table called");
        
        // Read args
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        MinimalReader reader(buffer, len);
        uint8_t data = reader.read_u8();
        
        log_info("Inserting simple_table: data=" + std::to_string(data));
        
        MinimalWriter w;
        write_identity_value(w);  // id
        w.write_u8(data);
        insert_value("simple_table", w);
    }
    
    void insert_every_primitive(uint32_t args) {
        log_info("insert_every_primitive called");
        
        // Read args
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        MinimalReader reader(buffer, len);
        uint8_t a_u8 = reader.read_u8();
        uint16_t a_u16 = reader.read_u16_le();
        uint32_t a_u32 = reader.read_u32_le();
        uint64_t a_u64 = reader.read_u64_le();
        int8_t a_i8 = static_cast<int8_t>(reader.read_u8());
        int16_t a_i16 = reader.read_i16_le();
        int32_t a_i32 = reader.read_i32_le();
        int64_t a_i64 = reader.read_i64_le();
        bool a_bool = reader.read_u8() != 0;
        std::string a_string = reader.read_string();
        
        log_info("Inserting every_primitive_struct");
        
        MinimalWriter w;
        write_identity_value(w);  // id
        w.write_u8(a_u8);
        w.write_u16_le(a_u16);
        w.write_u32_le(a_u32);
        w.write_u64_le(a_u64);
        w.write_u8(static_cast<uint8_t>(a_i8));
        w.write_u16_le(static_cast<uint16_t>(a_i16));
        w.write_u32_le(static_cast<uint32_t>(a_i32));
        w.write_i64_le(a_i64);
        w.write_u8(a_bool ? 1 : 0);
        w.write_string(a_string);
        insert_value("every_primitive_struct", w);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id,
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        switch (id) {
            case 0: init_reducer(); return 0;
            case 1: insert_person(args); return 0;
            case 2: insert_test_a(args); return 0;
            case 3: insert_point(args); return 0;
            case 4: insert_simple_table(args); return 0;
            case 5: insert_every_primitive(args); return 0;
            default: return -1;
        }
    }
}