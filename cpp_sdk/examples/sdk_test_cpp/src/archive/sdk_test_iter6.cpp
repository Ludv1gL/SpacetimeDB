#include <cstdint>
#include <cstring>
#include <vector>
#include <cstdio>

// Iteration 6: Build one table step by step

extern "C" {
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
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_source_read")))
    uint16_t bytes_source_read(uint32_t source, uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    __attribute__((import_module("spacetime_10.0"), import_name("datastore_insert_bsatn")))
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row_ptr, size_t* row_len_ptr);
    
    class BsatnWriter {
        std::vector<uint8_t> buffer;
        
        void log(const char* what) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Writing %s at offset %zu", what, buffer.size());
            const char* filename = "sdk_test_iter6.cpp";
            console_log(1, nullptr, 0, (const uint8_t*)filename, strlen(filename), 
                       __LINE__, (const uint8_t*)msg, strlen(msg));
        }
        
    public:
        void write_u8(uint8_t val, const char* desc = nullptr) {
            if (desc) log(desc);
            buffer.push_back(val);
        }
        
        void write_u32(uint32_t val, const char* desc = nullptr) {
            if (desc) log(desc);
            buffer.push_back(val & 0xFF);
            buffer.push_back((val >> 8) & 0xFF);
            buffer.push_back((val >> 16) & 0xFF);
            buffer.push_back((val >> 24) & 0xFF);
        }
        
        void write_bytes(const uint8_t* data, size_t len, const char* desc = nullptr) {
            if (desc) log(desc);
            for (size_t i = 0; i < len; i++) {
                buffer.push_back(data[i]);
            }
        }
        
        void write_string(const char* str, const char* desc = nullptr) {
            if (desc) log(desc);
            uint32_t len = strlen(str);
            write_u32(len);
            write_bytes((const uint8_t*)str, len);
        }
        
        const std::vector<uint8_t>& data() const { return buffer; }
    };
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        BsatnWriter w;
        
        // RawModuleDef::V9
        w.write_u8(1, "RawModuleDef::V9");
        
        // === Typespace ===
        // Based on the Rust code, Typespace serializes as just a Vec<AlgebraicType>
        w.write_u32(1, "typespace length = 1");
        
        // Type 0: Product type for OneU8
        // AlgebraicType::Product variant
        w.write_u8(2, "AlgebraicType::Product");
        
        // ProductType serialization
        // Based on SpacetimeType derive, it should serialize its fields in order
        // ProductType has: elements: Box<[ProductTypeElement]>
        // Box<[T]> serializes the same as Vec<T>
        w.write_u32(1, "ProductType.elements length = 1");
        
        // ProductTypeElement 0
        // ProductTypeElement has: name: Option<String>, ty: AlgebraicType
        // Option<String> for name "n"
        w.write_u8(0, "Option::Some");
        w.write_string("n", "field name");
        
        // AlgebraicType::U8
        w.write_u8(7, "AlgebraicType::U8");
        
        // === Tables ===
        w.write_u32(1, "tables length = 1");
        
        // RawTableDefV9 has many fields, let's write them in order
        w.write_string("one_u8", "table name");
        w.write_u32(0, "product_type_ref");
        
        // primary_key: Vec<ColId> (empty)
        w.write_u32(0, "primary_key length");
        
        // indexes: Vec<RawIndexDefV9> (empty)
        w.write_u32(0, "indexes length");
        
        // constraints: Vec<RawConstraintDefV9> (empty)
        w.write_u32(0, "constraints length");
        
        // sequences: Vec<RawSequenceDefV9> (empty)
        w.write_u32(0, "sequences length");
        
        // schedule: Option<RawScheduleDefV9>
        w.write_u8(1, "Option::None for schedule");
        
        // table_type: StTableType
        w.write_u8(1, "StTableType::User");  // User = 1 (not 0!)
        
        // table_access: StAccess
        w.write_u8(0, "StAccess::Public");
        
        // === Reducers ===
        w.write_u32(1, "reducers length = 1");
        
        // Reducer: insert_one_u8
        w.write_string("insert_one_u8", "reducer name");
        w.write_u32(1, "number of args");
        
        // Arg: n
        w.write_u8(0, "Option::Some");
        w.write_string("n", "arg name");
        w.write_u8(7, "AlgebraicType::U8");
        
        // === Types ===
        w.write_u32(0, "types length = 0");
        
        // === MiscExports ===
        w.write_u32(0, "misc_exports length = 0");
        
        // === RowLevelSecurity ===
        w.write_u32(0, "row_level_security length = 0");
        
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
        if (id == 0) {  // insert_one_u8 reducer
            // Read arguments
            uint8_t args_buffer[256];
            size_t args_len = sizeof(args_buffer);
            uint16_t result = bytes_source_read(args_source, args_buffer, &args_len);
            if (result != 0) {
                return -1;
            }
            
            // Extract the u8 value (should be just 1 byte)
            if (args_len < 1) {
                return -1;
            }
            uint8_t n = args_buffer[0];
            
            // Create row data (just the single u8 value)
            uint8_t row_data[1] = {n};
            
            // Insert into table (table_id = 0 for one_u8)
            size_t row_len = 1;
            result = datastore_insert_bsatn(0, row_data, &row_len);
            if (result != 0) {
                return -1;
            }
            
            return 0;
        }
        
        return -1;  // Unknown reducer
    }
}