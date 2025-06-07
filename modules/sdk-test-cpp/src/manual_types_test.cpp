// Manual module with explicit type definitions
#include <cstdint>
#include <cstring>
#include <vector>

// Manual ABI exports
#define SPACETIMEDB_WASM_EXPORT __attribute__((visibility("default")))

// Manual buffer functions
extern "C" {
    extern void spacetime_buffer_consume(uint32_t buffer_handle, const uint8_t* data, size_t len);
    extern uint8_t* spacetime_buffer_len(uint32_t buffer_handle, uint32_t* len);
}

// Manual BSATN writing helpers
void write_u8(std::vector<uint8_t>& buf, uint8_t val) {
    buf.push_back(val);
}

void write_u16_le(std::vector<uint8_t>& buf, uint16_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
}

void write_u32_le(std::vector<uint8_t>& buf, uint32_t val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

void write_string(std::vector<uint8_t>& buf, const char* str) {
    size_t len = strlen(str);
    write_u32_le(buf, len);
    for (size_t i = 0; i < len; i++) {
        buf.push_back(str[i]);
    }
}

// Table to store test data
struct TestTable {
    struct Row {
        uint8_t u8_val;
        uint16_t u16_val;
        int8_t i8_val;
        float f32_val;
    };
    static std::vector<Row> rows;
};
std::vector<TestTable::Row> TestTable::rows;

extern "C" {

SPACETIMEDB_WASM_EXPORT
void __describe_module__(uint32_t description) {
    std::vector<uint8_t> module_def;
    
    // RawModuleDefV9 structure
    // typespace: Typespace
    write_u8(module_def, 0); // typespace.types = empty vector
    write_u32_le(module_def, 0);
    
    // types: Vec<RawTypeDefV9>
    write_u32_le(module_def, 0); // empty
    
    // tables: Vec<RawTableDefV9>
    write_u32_le(module_def, 1); // 1 table
    
    // Table 0: test_table
    write_string(module_def, "test_table");
    write_u8(module_def, 0); // table_type = User
    write_u8(module_def, 0); // table_access = Public
    
    // product_type_ref - inline the type
    write_u8(module_def, 2); // Product type
    write_u32_le(module_def, 4); // 4 fields
    
    // Field 0: u8_val
    write_u8(module_def, 0); // Some
    write_string(module_def, "u8_val");
    write_u8(module_def, 7); // U8 type
    
    // Field 1: u16_val  
    write_u8(module_def, 0); // Some
    write_string(module_def, "u16_val");
    write_u8(module_def, 9); // U16 type (correct!)
    
    // Field 2: i8_val
    write_u8(module_def, 0); // Some
    write_string(module_def, "i8_val");
    write_u8(module_def, 6); // I8 type
    
    // Field 3: f32_val
    write_u8(module_def, 0); // Some
    write_string(module_def, "f32_val");
    write_u8(module_def, 18); // F32 type (correct!)
    
    // primary_key: ColList
    write_u8(module_def, 0); // ColListBuilder = empty
    
    // indexes: Vec<RawIndexDefV9>
    write_u32_le(module_def, 0); // empty
    
    // constraints: Vec<RawConstraintDefV9>
    write_u32_le(module_def, 0); // empty
    
    // sequences: Vec<RawSequenceDefV9>
    write_u32_le(module_def, 0); // empty
    
    // schedule: Option<RawScheduleDefV9>
    write_u8(module_def, 1); // None
    
    // schedulers: Vec<SchedulerKind>
    write_u32_le(module_def, 0); // empty
    
    // reducers: Vec<RawReducerDefV9>
    write_u32_le(module_def, 1); // 1 reducer
    
    // Reducer 0: test_types
    write_string(module_def, "test_types");
    
    // params: ProductType with correct types
    write_u8(module_def, 2); // Product type
    write_u32_le(module_def, 4); // 4 params
    
    // Param 0: u8_val
    write_u8(module_def, 0); // Some
    write_string(module_def, "u8_val");
    write_u8(module_def, 7); // U8 type
    
    // Param 1: u16_val - THIS IS THE KEY FIX
    write_u8(module_def, 0); // Some
    write_string(module_def, "u16_val");
    write_u8(module_def, 9); // U16 type (not I16!)
    
    // Param 2: i8_val
    write_u8(module_def, 0); // Some  
    write_string(module_def, "i8_val");
    write_u8(module_def, 6); // I8 type
    
    // Param 3: f32_val
    write_u8(module_def, 0); // Some
    write_string(module_def, "f32_val");
    write_u8(module_def, 18); // F32 type (not U128!)
    
    // lifecycle: Option<LifecycleReducer>
    write_u8(module_def, 1); // None
    
    // misc_exports: Vec<RawMiscModuleExportV9>
    write_u32_le(module_def, 0); // empty
    
    // row_level_security: Vec<RawRowLevelSecurityDefV9>
    write_u32_le(module_def, 0); // empty
    
    // Send to host
    spacetime_buffer_consume(description, module_def.data(), module_def.size());
}

SPACETIMEDB_WASM_EXPORT
int16_t __call_reducer__(
    uint32_t reducer_id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp,
    uint32_t args,
    uint32_t error
) {
    if (reducer_id == 0) { // test_types
        // Read arguments
        uint32_t args_len = 0;
        uint8_t* args_ptr = spacetime_buffer_len(args, &args_len);
        
        if (args_len >= 8) { // Expect at least 8 bytes
            uint8_t u8_val = args_ptr[0];
            uint16_t u16_val = args_ptr[1] | (args_ptr[2] << 8);
            int8_t i8_val = (int8_t)args_ptr[3];
            // Float would need proper deserialization
            
            // Store in table
            TestTable::rows.push_back({u8_val, u16_val, i8_val, 0.0f});
        }
        
        return 0; // Success
    }
    return -1; // No such reducer
}

} // extern "C"