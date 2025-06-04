#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <optional>

// Enhanced BSATN writer with more type support
class ComprehensiveWriter {
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
    
    void write_f32_le(float v) {
        union { float f; uint32_t u; } converter;
        converter.f = v;
        write_u32_le(converter.u);
    }
    
    void write_f64_le(double v) {
        union { double f; uint64_t u; } converter;
        converter.f = v;
        write_u64_le(converter.u);
    }
    
    void write_bool(bool v) { write_u8(v ? 1 : 0); }
    
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
    
    // Write Option<T> - tag 0 for Some, tag 1 for None  
    void write_option_some() { write_u8(0); }
    void write_option_none() { write_u8(1); }
    
    // Write sum type tag
    void write_sum_tag(uint8_t tag) { write_u8(tag); }
    
    // Write array/vector
    template<typename T>
    void write_vec(const std::vector<T>& vec) {
        write_u32_le(vec.size());
        for (const auto& item : vec) {
            // For primitive types, write directly
            if constexpr (std::is_same_v<T, uint8_t>) {
                write_u8(item);
            } else if constexpr (std::is_same_v<T, uint16_t>) {
                write_u16_le(item);
            } else if constexpr (std::is_same_v<T, uint32_t>) {
                write_u32_le(item);
            } else if constexpr (std::is_same_v<T, uint64_t>) {
                write_u64_le(item);
            } else if constexpr (std::is_same_v<T, int8_t>) {
                write_i8(item);
            } else if constexpr (std::is_same_v<T, int16_t>) {
                write_i16_le(item);
            } else if constexpr (std::is_same_v<T, int32_t>) {
                write_i32_le(item);
            } else if constexpr (std::is_same_v<T, int64_t>) {
                write_i64_le(item);
            } else if constexpr (std::is_same_v<T, float>) {
                write_f32_le(item);
            } else if constexpr (std::is_same_v<T, double>) {
                write_f64_le(item);
            } else if constexpr (std::is_same_v<T, bool>) {
                write_bool(item);
            } else if constexpr (std::is_same_v<T, std::string>) {
                write_string(item);
            }
        }
    }
    
    std::vector<uint8_t> take_buffer() { return std::move(buffer); }
};

// Enhanced BSATN reader
class ComprehensiveReader {
    const uint8_t* data;
    size_t pos;
    size_t len;
public:
    ComprehensiveReader(const uint8_t* d, size_t l) : data(d), pos(0), len(l) {}
    
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
    
    int8_t read_i8() { return static_cast<int8_t>(read_u8()); }
    int16_t read_i16_le() { return static_cast<int16_t>(read_u16_le()); }
    int32_t read_i32_le() { return static_cast<int32_t>(read_u32_le()); }
    int64_t read_i64_le() { return static_cast<int64_t>(read_u64_le()); }
    
    float read_f32_le() {
        union { float f; uint32_t u; } converter;
        converter.u = read_u32_le();
        return converter.f;
    }
    
    double read_f64_le() {
        union { double f; uint64_t u; } converter;
        converter.u = read_u64_le();
        return converter.f;
    }
    
    bool read_bool() { return read_u8() != 0; }
    
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
    
    // Read Option<T> - tag 0 for Some, tag 1 for None
    template<typename T>
    std::optional<T> read_option() {
        uint8_t tag = read_u8();
        if (tag == 0) {
            // Some - read the value
            if constexpr (std::is_same_v<T, std::string>) {
                return read_string();
            } else if constexpr (std::is_same_v<T, uint8_t>) {
                return read_u8();
            } else if constexpr (std::is_same_v<T, uint32_t>) {
                return read_u32_le();
            }
            // Add more types as needed
        }
        return std::nullopt; // None
    }
    
    uint8_t read_sum_tag() { return read_u8(); }
    
    template<typename T>
    std::vector<T> read_vec() {
        uint32_t count = read_u32_le();
        std::vector<T> result;
        result.reserve(count);
        for (uint32_t i = 0; i < count; i++) {
            if constexpr (std::is_same_v<T, uint8_t>) {
                result.push_back(read_u8());
            } else if constexpr (std::is_same_v<T, uint16_t>) {
                result.push_back(read_u16_le());
            } else if constexpr (std::is_same_v<T, uint32_t>) {
                result.push_back(read_u32_le());
            } else if constexpr (std::is_same_v<T, uint64_t>) {
                result.push_back(read_u64_le());
            } else if constexpr (std::is_same_v<T, int8_t>) {
                result.push_back(read_i8());
            } else if constexpr (std::is_same_v<T, int16_t>) {
                result.push_back(read_i16_le());
            } else if constexpr (std::is_same_v<T, int32_t>) {
                result.push_back(read_i32_le());
            } else if constexpr (std::is_same_v<T, int64_t>) {
                result.push_back(read_i64_le());
            } else if constexpr (std::is_same_v<T, float>) {
                result.push_back(read_f32_le());
            } else if constexpr (std::is_same_v<T, double>) {
                result.push_back(read_f64_le());
            } else if constexpr (std::is_same_v<T, bool>) {
                result.push_back(read_bool());
            } else if constexpr (std::is_same_v<T, std::string>) {
                result.push_back(read_string());
            }
        }
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

// Enhanced logging
void log_info(const std::string& msg) {
    const char* filename = "comprehensive_module_test.cpp";
    console_log(2, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg.c_str(), msg.length());
}

void log_debug(const std::string& msg) {
    const char* filename = "comprehensive_module_test.cpp";
    console_log(1, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg.c_str(), msg.length());
}

void log_error(const std::string& msg) {
    const char* filename = "comprehensive_module_test.cpp";
    console_log(4, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
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
void write_field(ComprehensiveWriter& w, const std::string& name, AlgebraicTypeTag type) {
    w.write_u8(0); // Some (field name present)
    w.write_string(name);
    w.write_u8(static_cast<uint8_t>(type));
}

// Write field with type reference 
void write_field_ref(ComprehensiveWriter& w, const std::string& name, uint32_t type_ref) {
    w.write_u8(0); // Some (field name present)
    w.write_string(name);
    w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Ref));
    w.write_u32_le(type_ref);
}

// Write Identity type (U256 wrapper)
void write_identity_type(ComprehensiveWriter& w) {
    w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
    w.write_u32_le(1); // 1 field
    write_field(w, "__identity__", AlgebraicTypeTag::U256);
}

// Write ConnectionId type (U128 wrapper)
void write_connection_id_type(ComprehensiveWriter& w) {
    w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
    w.write_u32_le(1); // 1 field
    write_field(w, "__connection_id__", AlgebraicTypeTag::U128);
}

// Write sum type (enum)
void write_enum_type(ComprehensiveWriter& w, const std::vector<std::string>& variants) {
    w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Sum));
    w.write_u32_le(variants.size());
    for (size_t i = 0; i < variants.size(); i++) {
        w.write_u8(0); // Some (variant name present)
        w.write_string(variants[i]);
        // For simple enums, each variant has no data (unit type)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(0); // 0 fields for unit variants
    }
}

// Write array type
void write_array_type(ComprehensiveWriter& w, AlgebraicTypeTag element_type) {
    w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Array));
    w.write_u8(static_cast<uint8_t>(element_type));
}

// Write identity value (32 bytes of zeros)
void write_identity_value(ComprehensiveWriter& w) {
    for (int i = 0; i < 32; i++) {
        w.write_u8(0);
    }
}

// Write connection id value (16 bytes of zeros)
void write_connection_id_value(ComprehensiveWriter& w) {
    for (int i = 0; i < 16; i++) {
        w.write_u8(0);
    }
}

// Table helper
bool insert_value(const std::string& table_name, ComprehensiveWriter& row_writer) {
    uint32_t table_id = 0;
    if (table_id_from_name((const uint8_t*)table_name.c_str(), table_name.length(), &table_id) != 0) {
        log_error("Failed to get table ID for " + table_name);
        return false;
    }
    
    auto buffer = row_writer.take_buffer();
    size_t len = buffer.size();
    
    uint16_t err = datastore_insert_bsatn(table_id, buffer.data(), &len);
    if (err == 0) {
        log_info("Successfully inserted into " + table_name);
        return true;
    } else {
        log_error("Failed to insert into " + table_name + " (error: " + std::to_string(err) + ")");
        return false;
    }
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        ComprehensiveWriter w;
        
        // RawModuleDef::V9 tag
        w.write_u8(1);
        
        // Typespace with expanded types from Rust module-test
        w.write_u32_le(10); // 10 types
        
        // Type 0: Person (id: u32, name: String, age: u8)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(3); // 3 fields
        write_field(w, "id", AlgebraicTypeTag::U32);
        write_field(w, "name", AlgebraicTypeTag::String);
        write_field(w, "age", AlgebraicTypeTag::U8);
        
        // Type 1: TestA (x: u32, y: u32, z: String) - matches Rust
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(3); // 3 fields
        write_field(w, "x", AlgebraicTypeTag::U32);
        write_field(w, "y", AlgebraicTypeTag::U32);
        write_field(w, "z", AlgebraicTypeTag::String);
        
        // Type 2: TestB (foo: String)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(1); // 1 field
        write_field(w, "foo", AlgebraicTypeTag::String);
        
        // Type 3: TestC enum (Foo, Bar)
        write_enum_type(w, {"Foo", "Bar"});
        
        // Type 4: TestD (test_c: Option<TestC>)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(1); // 1 field
        w.write_u8(0); // Some (field name)
        w.write_string("test_c");
        // Option<TestC> = Sum type with None/Some(TestC)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Sum));
        w.write_u32_le(2); // 2 variants: None, Some
        w.write_u8(0); // Some (variant name)
        w.write_string("none");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(0); // None = unit type
        w.write_u8(0); // Some (variant name)  
        w.write_string("some");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Ref));
        w.write_u32_le(3); // Ref to TestC
        
        // Type 5: TestE (id: u64, name: String)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(2); // 2 fields
        write_field(w, "id", AlgebraicTypeTag::U64);
        write_field(w, "name", AlgebraicTypeTag::String);
        
        // Type 6: Point (x: i64, y: i64)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(2); // 2 fields
        write_field(w, "x", AlgebraicTypeTag::I64);
        write_field(w, "y", AlgebraicTypeTag::I64);
        
        // Type 7: Identity wrapper
        write_identity_type(w);
        
        // Type 8: ConnectionId wrapper  
        write_connection_id_type(w);
        
        // Type 9: Player (identity: Identity, player_id: u64, name: String)
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(3); // 3 fields
        write_field_ref(w, "identity", 7); // Ref to Identity
        write_field(w, "player_id", AlgebraicTypeTag::U64);
        write_field(w, "name", AlgebraicTypeTag::String);
        
        // Tables - focusing on key tables from Rust module-test
        w.write_u32_le(7); // 7 tables
        
        // Table: person (public)
        w.write_string("person");
        w.write_u32_le(0); // type_ref = 0
        w.write_u32_le(1); // primary_key = [0] (id field)
        w.write_u32_le(0); // primary_key field 0
        w.write_u32_le(1); // indexes - age index
        w.write_u32_le(1); // 1 column
        w.write_u32_le(2); // age field (index 2)
        w.write_u32_le(0); // constraints
        w.write_u32_le(1); // sequences - id is auto_inc
        w.write_u32_le(0); // sequence on field 0 (id)
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Table: test_a (private with index)
        w.write_string("test_a");
        w.write_u32_le(1); // type_ref = 1
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(1); // indexes - x index
        w.write_u32_le(1); // 1 column
        w.write_u32_le(0); // x field (index 0)
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(1); // table_access = Private
        
        // Table: test_d (public)
        w.write_string("test_d");
        w.write_u32_le(4); // type_ref = 4
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(0); // indexes
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Table: test_e (private, primary key, index)
        w.write_string("test_e");
        w.write_u32_le(5); // type_ref = 5
        w.write_u32_le(1); // primary_key = [0] (id field)
        w.write_u32_le(0); // primary_key field 0
        w.write_u32_le(1); // indexes - name index
        w.write_u32_le(1); // 1 column
        w.write_u32_le(1); // name field (index 1)
        w.write_u32_le(0); // constraints
        w.write_u32_le(1); // sequences - id is auto_inc
        w.write_u32_le(0); // sequence on field 0 (id)
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(1); // table_access = Private
        
        // Table: points (private, multi-column index)
        w.write_string("points");
        w.write_u32_le(6); // type_ref = 6
        w.write_u32_le(0); // primary_key = empty
        w.write_u32_le(1); // indexes - multi_column_index
        w.write_u32_le(2); // 2 columns
        w.write_u32_le(0); // x field
        w.write_u32_le(1); // y field
        w.write_u32_le(0); // constraints
        w.write_u32_le(0); // sequences
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(1); // table_access = Private
        
        // Table: player (public)
        w.write_string("player");
        w.write_u32_le(9); // type_ref = 9
        w.write_u32_le(1); // primary_key = [0] (identity field)
        w.write_u32_le(0); // primary_key field 0
        w.write_u32_le(2); // indexes - player_id and name
        w.write_u32_le(1); // 1 column for player_id index
        w.write_u32_le(1); // player_id field
        w.write_u32_le(1); // 1 column for name index
        w.write_u32_le(2); // name field
        w.write_u32_le(2); // constraints - unique on player_id and name
        w.write_u32_le(1); // player_id unique
        w.write_u32_le(2); // name unique
        w.write_u32_le(1); // sequences - player_id is auto_inc
        w.write_u32_le(1); // sequence on field 1 (player_id)
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Table: logged_out_player (public, same type as player)
        w.write_string("logged_out_player");
        w.write_u32_le(9); // type_ref = 9 (same as player)
        w.write_u32_le(1); // primary_key = [0] (identity field)
        w.write_u32_le(0); // primary_key field 0
        w.write_u32_le(2); // indexes - player_id and name
        w.write_u32_le(1); // 1 column for player_id index
        w.write_u32_le(1); // player_id field
        w.write_u32_le(1); // 1 column for name index
        w.write_u32_le(2); // name field
        w.write_u32_le(2); // constraints - unique on player_id and name
        w.write_u32_le(1); // player_id unique
        w.write_u32_le(2); // name unique
        w.write_u32_le(1); // sequences - player_id is auto_inc
        w.write_u32_le(1); // sequence on field 1 (player_id)
        w.write_u8(1); // schedule = None
        w.write_u8(1); // table_type = User
        w.write_u8(0); // table_access = Public
        
        // Reducers
        w.write_u32_le(10); // 10 reducers
        
        // Reducer: init
        w.write_string("init");
        w.write_u32_le(0); // 0 params
        w.write_u8(0); // lifecycle = Some(Init)
        w.write_u8(0); // Init variant
        
        // Reducer: add (name: String, age: u8)
        w.write_string("add");
        w.write_u32_le(2); // 2 params
        write_field(w, "name", AlgebraicTypeTag::String);
        write_field(w, "age", AlgebraicTypeTag::U8);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: say_hello
        w.write_string("say_hello");
        w.write_u32_le(0); // 0 params
        w.write_u8(1); // lifecycle = None
        
        // Reducer: list_over_age (age: u8)
        w.write_string("list_over_age");
        w.write_u32_le(1); // 1 param
        write_field(w, "age", AlgebraicTypeTag::U8);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: test (complex args from Rust)
        w.write_string("test");
        w.write_u32_le(4); // 4 params
        w.write_u8(0); // Some (field name)
        w.write_string("arg");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Ref));
        w.write_u32_le(1); // Ref to TestA (TestAlias)
        w.write_u8(0); // Some (field name)
        w.write_string("arg2");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Ref));
        w.write_u32_le(2); // Ref to TestB
        w.write_u8(0); // Some (field name)
        w.write_string("arg3");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Ref));
        w.write_u32_le(3); // Ref to TestC
        w.write_u8(0); // Some (field name)  
        w.write_string("arg4");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Sum)); // TestF enum inline
        w.write_u32_le(3); // 3 variants
        w.write_u8(0); // Some (variant name)
        w.write_string("Foo");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(0); // unit type
        w.write_u8(0); // Some (variant name)
        w.write_string("Bar");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::Product));
        w.write_u32_le(0); // unit type
        w.write_u8(0); // Some (variant name)
        w.write_string("Baz");
        w.write_u8(static_cast<uint8_t>(AlgebraicTypeTag::String)); // String variant
        w.write_u8(1); // lifecycle = None
        
        // Reducer: add_player (name: String)
        w.write_string("add_player");
        w.write_u32_le(1); // 1 param
        write_field(w, "name", AlgebraicTypeTag::String);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: delete_player (id: u64)
        w.write_string("delete_player");
        w.write_u32_le(1); // 1 param
        write_field(w, "id", AlgebraicTypeTag::U64);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: delete_players_by_name (name: String)
        w.write_string("delete_players_by_name");
        w.write_u32_le(1); // 1 param
        write_field(w, "name", AlgebraicTypeTag::String);
        w.write_u8(1); // lifecycle = None
        
        // Reducer: client_connected
        w.write_string("client_connected");
        w.write_u32_le(0); // 0 params
        w.write_u8(0); // lifecycle = Some(ClientConnected)
        w.write_u8(1); // ClientConnected variant
        
        // Empty types, misc exports, row level security
        w.write_u32_le(0);
        w.write_u32_le(0);
        w.write_u32_le(0);
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        bytes_sink_write(sink, buffer.data(), &len);
    }
    
    void init_reducer() {
        log_info("Module initialized with comprehensive types");
        
        // Insert initial person data
        ComprehensiveWriter w;
        w.write_u32_le(1);  // id
        w.write_string("Alice");
        w.write_u8(30);  // age
        insert_value("person", w);
        
        ComprehensiveWriter w2;
        w2.write_u32_le(2);  // id
        w2.write_string("Bob");
        w2.write_u8(25);  // age
        insert_value("person", w2);
        
        // Insert test_a data
        ComprehensiveWriter w3;
        w3.write_u32_le(100);  // x
        w3.write_u32_le(200);  // y
        w3.write_string("initial"); // z
        insert_value("test_a", w3);
    }
    
    void add_reducer(uint32_t args) {
        log_info("add reducer called");
        
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        ComprehensiveReader reader(buffer, len);
        std::string name = reader.read_string();
        uint8_t age = reader.read_u8();
        
        log_info("Adding person: " + name + ", age=" + std::to_string(age));
        
        ComprehensiveWriter w;
        w.write_u32_le(0);  // id (auto-incremented)
        w.write_string(name);
        w.write_u8(age);
        insert_value("person", w);
    }
    
    void say_hello_reducer() {
        log_info("Say hello reducer called");
        // TODO: Implement when we have table iteration
        log_info("Hello, World!");
    }
    
    void list_over_age_reducer(uint32_t args) {
        log_info("list_over_age reducer called");
        
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        ComprehensiveReader reader(buffer, len);
        uint8_t age = reader.read_u8();
        
        log_info("Listing people over age " + std::to_string(age));
        // TODO: Implement when we have table filtering
    }
    
    void test_reducer(uint32_t args) {
        log_info("Test reducer called with complex arguments");
        
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        ComprehensiveReader reader(buffer, len);
        
        // Read TestA (TestAlias)
        uint32_t x = reader.read_u32_le();
        uint32_t y = reader.read_u32_le();
        std::string z = reader.read_string();
        log_info("TestA: x=" + std::to_string(x) + ", y=" + std::to_string(y) + ", z=" + z);
        
        // Read TestB
        std::string foo = reader.read_string();
        log_info("TestB: foo=" + foo);
        
        // Read TestC enum
        uint8_t test_c_tag = reader.read_sum_tag();
        log_info("TestC: tag=" + std::to_string(test_c_tag));
        
        // Read TestF enum  
        uint8_t test_f_tag = reader.read_sum_tag();
        if (test_f_tag == 2) { // Baz variant
            std::string baz_string = reader.read_string();
            log_info("TestF: Baz(" + baz_string + ")");
        } else {
            log_info("TestF: tag=" + std::to_string(test_f_tag));
        }
        
        // Insert some test_a data
        for (int i = 0; i < 10; i++) {
            ComprehensiveWriter w;
            w.write_u32_le(i + x);
            w.write_u32_le(i + y);
            w.write_string("test_" + std::to_string(i));
            insert_value("test_a", w);
        }
    }
    
    void add_player_reducer(uint32_t args) {
        log_info("add_player reducer called");
        
        uint8_t buffer[1024];
        size_t len = sizeof(buffer);
        bytes_source_read(args, buffer, &len);
        
        ComprehensiveReader reader(buffer, len);
        std::string name = reader.read_string();
        
        log_info("Adding player: " + name);
        
        ComprehensiveWriter w;
        write_identity_value(w);  // identity
        w.write_u64_le(0);  // player_id (auto-incremented)
        w.write_string(name);
        insert_value("player", w);
    }
    
    void delete_player_reducer(uint32_t args) {
        log_info("delete_player reducer called");
        // TODO: Implement delete operations
    }
    
    void delete_players_by_name_reducer(uint32_t args) {
        log_info("delete_players_by_name reducer called");
        // TODO: Implement delete operations
    }
    
    void client_connected_reducer() {
        log_info("Client connected reducer called");
        // Built-in reducer for when clients connect
    }
    
    __attribute__((export_name("__call_reducer__")))
    int32_t __call_reducer__(uint32_t id,
                            uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                            uint64_t conn_id_0, uint64_t conn_id_1,
                            uint64_t timestamp, uint32_t args, uint32_t error) {
        switch (id) {
            case 0: init_reducer(); return 0;
            case 1: add_reducer(args); return 0;
            case 2: say_hello_reducer(); return 0;
            case 3: list_over_age_reducer(args); return 0;
            case 4: test_reducer(args); return 0;
            case 5: add_player_reducer(args); return 0;
            case 6: delete_player_reducer(args); return 0;
            case 7: delete_players_by_name_reducer(args); return 0;
            case 8: client_connected_reducer(); return 0;
            default: return -1;
        }
    }
}