#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

// This module manually generates the schema that matches the Rust sdk-test module
// It's what a working C++ SDK would generate automatically

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // BSATN encoding helpers
    class BsatnWriter {
        std::vector<uint8_t> buffer;
        
    public:
        void write_u8(uint8_t val) {
            buffer.push_back(val);
        }
        
        void write_u32(uint32_t val) {
            buffer.push_back(val & 0xFF);
            buffer.push_back((val >> 8) & 0xFF);
            buffer.push_back((val >> 16) & 0xFF);
            buffer.push_back((val >> 24) & 0xFF);
        }
        
        void write_string(const std::string& str) {
            write_u32(str.length());
            for (char c : str) {
                buffer.push_back(c);
            }
        }
        
        void write_option_string(const std::string* str) {
            if (str) {
                write_u8(1); // Some
                write_string(*str);
            } else {
                write_u8(0); // None
            }
        }
        
        void write_vec_u32(const std::vector<uint32_t>& vec) {
            write_u32(vec.size());
            for (uint32_t val : vec) {
                write_u32(val);
            }
        }
        
        const std::vector<uint8_t>& get_buffer() const { return buffer; }
    };
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        BsatnWriter writer;
        
        // RawModuleDef::V9
        writer.write_u8(1);  // variant V9 = 1
        
        // === Typespace ===
        // We'll define types for: OneU8Row, SimpleEnum
        writer.write_u32(2);  // typespace vector length
        
        // Type 0: OneU8Row - Product type with one field
        writer.write_u8(0);  // AlgebraicType::Product
        writer.write_u32(1);  // elements count
        // Field "n"
        writer.write_u8(1);  // Some(name)
        writer.write_string("n");
        writer.write_u8(1);  // AlgebraicType::U8
        
        // Type 1: SimpleEnum - Sum type with 3 variants
        writer.write_u8(1);  // AlgebraicType::Sum
        writer.write_u32(3);  // variants count
        // Variant 0: Zero
        writer.write_u8(1);  // Some(name)
        writer.write_string("Zero");
        writer.write_u8(0);  // AlgebraicType::Product
        writer.write_u32(0);  // no fields
        // Variant 1: One
        writer.write_u8(1);  // Some(name)
        writer.write_string("One");
        writer.write_u8(0);  // AlgebraicType::Product
        writer.write_u32(0);  // no fields
        // Variant 2: Two
        writer.write_u8(1);  // Some(name)
        writer.write_string("Two");
        writer.write_u8(0);  // AlgebraicType::Product
        writer.write_u32(0);  // no fields
        
        // === Tables ===
        writer.write_u32(1);  // tables vector length
        
        // Table: one_u8
        writer.write_string("one_u8");
        writer.write_u32(0);  // product_type_ref = 0 (OneU8Row)
        writer.write_vec_u32({});  // primary_key (empty)
        writer.write_vec_u32({});  // indexes (empty)
        writer.write_vec_u32({});  // constraints (empty)
        writer.write_vec_u32({});  // sequences (empty)
        writer.write_u8(0);  // schedule: None
        writer.write_u8(0);  // table_type: User
        writer.write_u8(0);  // table_access: Public
        
        // === Reducers ===
        writer.write_u32(1);  // reducers vector length
        
        // Reducer: insert_one_u8
        writer.write_string("insert_one_u8");
        writer.write_u32(1);  // args count
        // Arg 0: n
        writer.write_option_string(nullptr);  // no name
        writer.write_u8(1);  // AlgebraicType::U8
        
        // === Types (named types) ===
        writer.write_u32(2);  // types vector length
        
        // Named type: OneU8
        writer.write_u8(0);  // ScopedName with empty scope
        writer.write_u32(0);  // scope length
        writer.write_string("OneU8");
        writer.write_u32(0);  // ty = 0 (index into typespace)
        writer.write_u8(1);   // custom_ordering = true
        
        // Named type: SimpleEnum  
        writer.write_u8(0);  // ScopedName with empty scope
        writer.write_u32(0);  // scope length
        writer.write_string("SimpleEnum");
        writer.write_u32(1);  // ty = 1 (index into typespace)
        writer.write_u8(1);   // custom_ordering = true
        
        // === MiscExports ===
        writer.write_u32(0);  // misc_exports (empty)
        
        // === RowLevelSecurity ===
        writer.write_u32(1);  // row_level_security vector length
        writer.write_string("SELECT * FROM one_u8");  // SQL filter
        
        // Write the complete buffer
        const auto& buffer = writer.get_buffer();
        size_t len = buffer.size();
        bytes_sink_write(sink, buffer.data(), &len);
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
        // For now, just return success
        return 0;
    }
}