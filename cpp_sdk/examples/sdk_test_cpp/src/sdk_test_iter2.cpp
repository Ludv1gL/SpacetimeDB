#include <cstdint>
#include <cstring>
#include <vector>

// Iteration 2: Add one table with proper BSATN encoding

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // Simple BSATN writer
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
        
        void write_string(const char* str) {
            uint32_t len = strlen(str);
            write_u32(len);
            for (uint32_t i = 0; i < len; i++) {
                buffer.push_back(str[i]);
            }
        }
        
        void write_option_some() {
            buffer.push_back(1);  // Some = 1
        }
        
        void write_option_none() {
            buffer.push_back(0);  // None = 0
        }
        
        const std::vector<uint8_t>& data() const { return buffer; }
    };
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        BsatnWriter w;
        
        // RawModuleDef::V9
        w.write_u8(1);  // V9 = 1
        
        // === Typespace ===
        w.write_u32(1);  // 1 type
        
        // Type 0: Product type for OneU8 table  
        w.write_u8(2);   // AlgebraicType::Product = 2 (based on enum order)
        w.write_u32(1);  // 1 field
        
        // Field: n
        w.write_option_some();  // field has name
        w.write_string("n");
        w.write_u8(7);  // AlgebraicType::U8 = 7
        
        // === Tables ===
        w.write_u32(1);  // 1 table
        
        // Table: one_u8
        w.write_string("one_u8");
        w.write_u32(0);  // product_type_ref = 0
        w.write_u32(0);  // primary_key: empty vec
        w.write_u32(0);  // indexes: empty vec  
        w.write_u32(0);  // constraints: empty vec
        w.write_u32(0);  // sequences: empty vec
        w.write_u8(0);   // schedule: None
        w.write_u8(0);   // table_type: User
        w.write_u8(0);   // table_access: Public
        
        // === Reducers ===
        w.write_u32(1);  // 1 reducer
        
        // Reducer: insert_one_u8
        w.write_string("insert_one_u8");
        w.write_u32(1);  // 1 arg
        
        // Arg: n
        w.write_option_some();
        w.write_string("n");
        w.write_u8(7);  // AlgebraicType::U8
        
        // === Types (named types) ===
        w.write_u32(0);  // empty for now
        
        // === MiscExports ===
        w.write_u32(0);  // empty
        
        // === RowLevelSecurity ===  
        w.write_u32(0);  // empty
        
        // Write buffer
        const auto& data = w.data();
        size_t len = data.size();
        bytes_sink_write(sink, data.data(), &len);
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
        // TODO: Actually implement reducer logic
        return 0;
    }
}