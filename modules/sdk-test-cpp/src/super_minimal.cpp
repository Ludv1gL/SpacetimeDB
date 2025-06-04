/**
 * Super minimal SpacetimeDB module
 * Just the bare minimum exports
 */

#include <cstdint>
#include <cstring>

// FFI imports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("console_log")))
    void _console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
}

void log_info(const char* msg) {
    const char* filename = "super_minimal.cpp";
    _console_log(2, nullptr, 0, (const uint8_t*)filename, strlen(filename), __LINE__, 
                (const uint8_t*)msg, strlen(msg));
}

// Required exports
extern "C" {
    __attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
    uint16_t _bytes_sink_write(uint32_t sink, const uint8_t* buffer, size_t* buffer_len);
    
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Minimal valid module description
        // RawModuleDef::V9 with empty everything
        uint8_t data[] = {
            1,  // RawModuleDef::V9 tag
            0, 0, 0, 0,  // typespace: empty vec
            0, 0, 0, 0,  // names: empty vec  
            0, 0, 0, 0,  // tables: empty vec
            0, 0, 0, 0,  // reducers: empty vec
            0, 0, 0, 0,  // types: empty vec
            0, 0, 0, 0,  // misc_exports: empty vec
            0, 0, 0, 0   // row_level_security: empty vec
        };
        size_t len = sizeof(data);
        _bytes_sink_write(sink, data, &len);
    }
    
    __attribute__((export_name("__call_reducer__")))
    int16_t __call_reducer__(
        uint32_t reducer_id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        uint64_t timestamp_us,
        uint32_t args_source,
        uint32_t error_sink
    ) {
        log_info("__call_reducer__ called");
        return -1; // NO_SUCH_REDUCER
    }
}