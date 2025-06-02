#include <cstdint>
#include <cstring>
#include <vector>
#include <cstdio>

// Iteration 8: Fix row encoding as ProductValue for BSATN

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
    
    void log_msg(const char* msg) {
        const char* filename = "sdk_test_iter8.cpp";
        console_log(1, nullptr, 0, (const uint8_t*)filename, strlen(filename), 
                   __LINE__, (const uint8_t*)msg, strlen(msg));
    }
    
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
        
        const std::vector<uint8_t>& data() const { return buffer; }
    };
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        BsatnWriter w;
        
        // RawModuleDef::V9
        w.write_u8(1);
        
        // === Typespace ===
        w.write_u32(1);  // 1 type
        
        // Type 0: Product type for OneU8
        w.write_u8(2);   // AlgebraicType::Product
        w.write_u32(1);  // 1 field
        
        // Field: n  
        w.write_u8(0);   // Option::Some
        w.write_string("n");
        w.write_u8(7);   // AlgebraicType::U8
        
        // === Tables ===
        w.write_u32(1);  // 1 table
        
        // Table: one_u8
        w.write_string("one_u8");
        w.write_u32(0);  // product_type_ref = 0
        w.write_u32(0);  // primary_key: empty
        w.write_u32(0);  // indexes: empty
        w.write_u32(0);  // constraints: empty
        w.write_u32(0);  // sequences: empty
        w.write_u8(1);   // Option::None for schedule
        w.write_u8(1);   // StTableType::User = 1
        w.write_u8(0);   // StAccess::Public = 0
        
        // === Reducers ===
        w.write_u32(1);  // 1 reducer
        
        // Reducer: insert_one_u8
        w.write_string("insert_one_u8");
        
        // params: ProductType
        w.write_u32(1);  // elements.len() = 1
        
        // ProductTypeElement for arg n
        w.write_u8(0);   // Option::Some for name
        w.write_string("n");
        w.write_u8(7);   // AlgebraicType::U8
        
        // lifecycle: Option<Lifecycle>
        w.write_u8(1);   // Option::None
        
        // === Types ===
        w.write_u32(0);  // empty
        
        // === MiscExports ===
        w.write_u32(0);  // empty
        
        // === RowLevelSecurity ===
        w.write_u32(0);  // empty
        
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
            log_msg("Called insert_one_u8 reducer");
            
            // Read all arguments into a buffer
            std::vector<uint8_t> args_buffer;
            args_buffer.reserve(256);
            
            // For invalid source (0), args is empty
            if (args_source == 0) {
                log_msg("Args source is invalid (0), using empty args");
            } else {
                while (true) {
                    size_t spare_capacity = args_buffer.capacity() - args_buffer.size();
                    args_buffer.resize(args_buffer.capacity());  // Temporarily resize to full capacity
                    uint8_t* write_ptr = args_buffer.data() + args_buffer.size() - spare_capacity;
                    size_t buf_len = spare_capacity;
                    
                    int16_t ret = bytes_source_read(args_source, write_ptr, &buf_len);
                    
                    char msg[128];
                    snprintf(msg, sizeof(msg), "bytes_source_read returned %d, buf_len=%zu", ret, buf_len);
                    log_msg(msg);
                    
                    if (ret <= 0) {
                        // Host side source exhausted (ret == -1) or wrote some bytes (ret == 0)
                        args_buffer.resize(args_buffer.size() - spare_capacity + buf_len);
                        
                        if (ret == -1) {
                            // Source exhausted, we're done
                            break;
                        }
                        
                        // If we filled the entire capacity, reserve more
                        if (buf_len == spare_capacity && spare_capacity > 0) {
                            args_buffer.reserve(args_buffer.capacity() + 256);
                        }
                    } else {
                        // Error case
                        args_buffer.resize(args_buffer.size() - spare_capacity);  // Reset size
                        char err_msg[128];
                        snprintf(err_msg, sizeof(err_msg), "Error reading args: ret=%d", ret);
                        log_msg(err_msg);
                        return -1;
                    }
                }
            }
            
            // Log the args buffer
            char msg[256];
            snprintf(msg, sizeof(msg), "Args buffer size: %zu", args_buffer.size());
            log_msg(msg);
            
            if (args_buffer.size() > 0) {
                snprintf(msg, sizeof(msg), "Args data: %02x", (unsigned)args_buffer[0]);
                log_msg(msg);
            }
            
            // For a single u8 argument, the BSATN encoding should be just the byte
            if (args_buffer.size() != 1) {
                log_msg("Error: Expected exactly 1 byte for u8 arg");
                return -1;
            }
            
            uint8_t n = args_buffer[0];
            
            // Create row data as ProductValue with single field
            // ProductValue is encoded as:
            // - element count (u32)
            // - for each element: the BSATN encoding of the value
            BsatnWriter row_writer;
            row_writer.write_u32(1);  // 1 element in the product
            row_writer.write_u8(n);   // the u8 value
            
            const auto& row_data_vec = row_writer.data();
            std::vector<uint8_t> row_data_copy = row_data_vec;  // Make a mutable copy
            size_t row_len = row_data_copy.size();
            
            // Insert into table
            // Table IDs are assigned by the system, not by the order in the module
            // For now, let's try table_id = 0 which is likely the first (and only) table
            snprintf(msg, sizeof(msg), "Inserting value %u into table_id 0", n);
            log_msg(msg);
            
            uint16_t result = datastore_insert_bsatn(0, row_data_copy.data(), &row_len);
            if (result != 0) {
                snprintf(msg, sizeof(msg), "Failed to insert: error code %u", result);
                log_msg(msg);
                
                // Try other table IDs just in case
                for (uint32_t tid = 1; tid < 10; tid++) {
                    row_len = row_data_copy.size();  // Reset length
                    snprintf(msg, sizeof(msg), "Trying table_id %u", tid);
                    log_msg(msg);
                    
                    result = datastore_insert_bsatn(tid, row_data_copy.data(), &row_len);
                    if (result == 0) {
                        snprintf(msg, sizeof(msg), "Success with table_id %u!", tid);
                        log_msg(msg);
                        return 0;
                    }
                }
                
                return -1;
            }
            
            log_msg("Successfully inserted row");
            return 0;
        }
        
        log_msg("Unknown reducer id");
        return -1;
    }
}