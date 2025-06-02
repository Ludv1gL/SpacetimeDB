#include <cstdint>
#include <cstring>
#include <vector>

extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
    
    // BSATN encoding helpers
    void write_u32_le(std::vector<uint8_t>& buf, uint32_t val) {
        buf.push_back(val & 0xFF);
        buf.push_back((val >> 8) & 0xFF);
        buf.push_back((val >> 16) & 0xFF);
        buf.push_back((val >> 24) & 0xFF);
    }
    
    void write_string(std::vector<uint8_t>& buf, const std::string& str) {
        write_u32_le(buf, str.length());
        for (char c : str) {
            buf.push_back(c);
        }
    }
    
    void write_u8(std::vector<uint8_t>& buf, uint8_t val) {
        buf.push_back(val);
    }
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        std::vector<uint8_t> data;
        
        // RawModuleDef::V9 structure:
        write_u8(data, 1);  // variant V9 = 1
        
        // Typespace with one simple type
        write_u32_le(data, 1);  // typespace vector length = 1
        
        // AlgebraicType::Product for OneU8Row
        write_u8(data, 0);  // Product variant = 0
        write_u32_le(data, 1);  // elements count = 1
        
        // ProductTypeElement for field "n"
        write_u8(data, 1);  // Some(name)
        write_string(data, "n");  // field name
        write_u8(data, 1);  // AlgebraicType::U8 = 1
        
        // Tables vector with one table
        write_u32_le(data, 1);  // tables vector length = 1
        
        // TableSchema for "one_u8" 
        write_string(data, "one_u8");  // table name
        write_u32_le(data, 0);  // product_type_ref = 0 (index into typespace)
        write_u32_le(data, 0);  // primary_key vector length = 0
        write_u32_le(data, 0);  // indexes vector length = 0
        write_u32_le(data, 0);  // constraints vector length = 0
        write_u32_le(data, 0);  // sequences vector length = 0
        write_u8(data, 0);  // schedule: none
        write_u8(data, 0);  // table_type: User
        write_u8(data, 0);  // table_access: Public
        
        // Empty vectors for remaining fields
        write_u32_le(data, 0);  // reducers (empty)
        write_u32_le(data, 0);  // types (empty)
        write_u32_le(data, 0);  // misc_exports (empty)
        write_u32_le(data, 0);  // row_level_security (empty)
        
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
        return 0;
    }
}