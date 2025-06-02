// Minimal C++ module for SpacetimeDB
#include <cstdint>
#include <cstring>

// ABI imports from host
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void console_log(uint8_t level, const uint8_t* target, size_t target_len,
                    const uint8_t* filename, size_t filename_len, uint32_t line_number,
                    const uint8_t* text, size_t text_len);
    
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t bytes_sink_write(uint32_t sink, const uint8_t* buffer_ptr, size_t* buffer_len_ptr);
}

// Module definition buffer
alignas(8) static const uint8_t MODULE_DEF[] = {
    // Minimal module definition with just a simple table
    0x01, 0x00, 0x00, 0x00, // Version 1
    0x01, 0x00, 0x00, 0x00, // 1 table
    0x05, 0x00, 0x00, 0x00, // table name length
    'u', 's', 'e', 'r', 's', // table name
    0x01, // public = true
    0x00, // no scheduled reducer
    0x01, 0x00, 0x00, 0x00, // 1 field
    0x02, 0x00, 0x00, 0x00, // field name length
    'i', 'd', // field name
    0x05, // CoreType::U32
    0x00, // not optional
    0x01, // unique
    0x01, // auto increment
    0x01, 0x00, 0x00, 0x00, // 1 reducer
    0x04, 0x00, 0x00, 0x00, // reducer name length
    'i', 'n', 'i', 't', // reducer name
    0x00, // Kind::None
    0x00, 0x00, 0x00, 0x00  // 0 parameters
};

// Export module definition
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // RawModuleDef::V9 structure
        uint8_t data[] = {
            1,              // RawModuleDef enum: variant V9 = 1
            0, 0, 0, 0,     // typespace (empty vector)
            0, 0, 0, 0,     // tables (empty vector)  
            1, 0, 0, 0,     // reducers (1 reducer)
            // Reducer 0: init
            4, 0, 0, 0,     // name length
            'i', 'n', 'i', 't', // name
            0, 0, 0, 0,     // params (empty vector)
            0,              // lifecycle flags
            0, 0, 0, 0,     // types (empty vector)
            0, 0, 0, 0,     // misc_exports (empty vector)
            0, 0, 0, 0      // row_level_security (empty vector)
        };
        
        size_t len = sizeof(data);
        bytes_sink_write(sink, data, &len);
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
        if (id == 0) { // init reducer
            const char* msg = "Minimal module initialized!";
            console_log(0, nullptr, 0, nullptr, 0, 0, 
                       reinterpret_cast<const uint8_t*>(msg), strlen(msg));
        }
        return 0; // success
    }
}