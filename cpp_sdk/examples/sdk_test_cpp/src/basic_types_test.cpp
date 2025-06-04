#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

// Minimal BSATN writer
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
    void write_i8(int8_t v) { write_u8(static_cast<uint8_t>(v)); }
    void write_i16_le(int16_t v) { write_u16_le(static_cast<uint16_t>(v)); }
    void write_i32_le(int32_t v) { write_u32_le(static_cast<uint32_t>(v)); }
    void write_i64_le(int64_t v) { write_u64_le(static_cast<uint64_t>(v)); }
    void write_bool(bool v) { write_u8(v ? 1 : 0); }
    void write_string(const std::string& s) {
        write_u32_le(s.length());
        for (char c : s) buffer.push_back(c);
    }
    void write_bytes(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            buffer.push_back(data[i]);
        }
    }
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
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
    const char* filename = "basic_types_test.cpp";
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
        
        // Typespace - define multiple types
        w.write_u32_le(4); // 4 types
        
        // Type 0: OneU8
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(1); // 1 field
        write_field(w, "n", AlgebraicTypeTag::U8);
        
        // Type 1: OneU16
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(1); // 1 field
        write_field(w, "n", AlgebraicTypeTag::U16);
        
        // Type 2: OneU32
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(1); // 1 field
        write_field(w, "n", AlgebraicTypeTag::U32);
        
        // Type 3: VecU8
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(1); // 1 field
        w.write_u8(0); // Some
        w.write_string("n");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Array)); // Array type
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::U8)); // Element type
        
        // Tables - one for each type
        w.write_u32_le(4);
        
        // Table: one_u8
        w.write_string("one_u8");
        w.write_u32_le(0); // type_ref = 0
        w.write_u32_le(0); // primary_key = empty vec
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1);     // schedule = None
        w.write_u8(1);     // table_type = User
        w.write_u8(0);     // table_access = Public
        
        // Table: one_u16
        w.write_string("one_u16");
        w.write_u32_le(1); // type_ref = 1
        w.write_u32_le(0); // primary_key = empty vec
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1);     // schedule = None
        w.write_u8(1);     // table_type = User
        w.write_u8(0);     // table_access = Public
        
        // Table: one_u32
        w.write_string("one_u32");
        w.write_u32_le(2); // type_ref = 2
        w.write_u32_le(0); // primary_key = empty vec
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1);     // schedule = None
        w.write_u8(1);     // table_type = User
        w.write_u8(0);     // table_access = Public
        
        // Table: vec_u8
        w.write_string("vec_u8");
        w.write_u32_le(3); // type_ref = 3
        w.write_u32_le(0); // primary_key = empty vec
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1);     // schedule = None
        w.write_u8(1);     // table_type = User
        w.write_u8(0);     // table_access = Public
        
        // Reducers
        w.write_u32_le(4);
        
        // Reducer: insert_one_u8
        w.write_string("insert_one_u8");
        w.write_u32_le(1); // 1 param
        write_field(w, "n", AlgebraicTypeTag::U8);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_one_u16
        w.write_string("insert_one_u16");
        w.write_u32_le(1); // 1 param
        write_field(w, "n", AlgebraicTypeTag::U16);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_one_u32
        w.write_string("insert_one_u32");
        w.write_u32_le(1); // 1 param
        write_field(w, "n", AlgebraicTypeTag::U32);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: insert_vec_u8
        w.write_string("insert_vec_u8");
        w.write_u32_le(1); // 1 param
        w.write_u8(0); // Some
        w.write_string("n");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Array)); // Array type
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::U8)); // Element type
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
    
    void insert_one_u8(uint32_t args, uint32_t error) {
        log_info("insert_one_u8 called");
        
        uint8_t value = 0;
        size_t len = 1;
        bytes_source_read(args, &value, &len);
        
        MinimalWriter w;
        w.write_u8(value);
        insert_value("one_u8", w);
    }
    
    void insert_one_u16(uint32_t args, uint32_t error) {
        log_info("insert_one_u16 called");
        
        uint16_t value = 0;
        size_t len = 2;
        bytes_source_read(args, (uint8_t*)&value, &len);
        
        MinimalWriter w;
        w.write_u16_le(value);
        insert_value("one_u16", w);
    }
    
    void insert_one_u32(uint32_t args, uint32_t error) {
        log_info("insert_one_u32 called");
        
        uint32_t value = 0;
        size_t len = 4;
        bytes_source_read(args, (uint8_t*)&value, &len);
        
        MinimalWriter w;
        w.write_u32_le(value);
        insert_value("one_u32", w);
    }
    
    void insert_vec_u8(uint32_t args, uint32_t error) {
        log_info("insert_vec_u8 called");
        
        // Read length first
        uint32_t vec_len = 0;
        size_t len = 4;
        bytes_source_read(args, (uint8_t*)&vec_len, &len);
        
        // Read vector data
        std::vector<uint8_t> data(vec_len);
        len = vec_len;
        bytes_source_read(args, data.data(), &len);
        
        MinimalWriter w;
        w.write_u32_le(vec_len);
        w.write_bytes(data.data(), vec_len);
        insert_value("vec_u8", w);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id,
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        switch (id) {
            case 0: insert_one_u8(args, error); return 0;
            case 1: insert_one_u16(args, error); return 0;
            case 2: insert_one_u32(args, error); return 0;
            case 3: insert_vec_u8(args, error); return 0;
            default: return -1;
        }
    }
}