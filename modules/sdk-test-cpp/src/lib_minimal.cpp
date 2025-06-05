// Ultra-minimal SpacetimeDB C++ module to test compilation and publishing
// Avoiding all C++ standard library features that might cause WASI imports

#ifdef __cplusplus
extern "C" {
#endif

// Minimal SpacetimeDB module exports - just enough to be recognized as a valid module

// Import the SpacetimeDB function we need
// Signature: (sink: u32, data: *const u8, len: *mut usize) -> u16 (error code)
__attribute__((import_module("spacetime_10.0"), import_name("bytes_sink_write")))
extern unsigned short bytes_sink_write(unsigned int sink, const unsigned char* data, unsigned int* len);

// Minimal module description - just write an empty module def
__attribute__((export_name("__describe_module__")))
void __describe_module__(unsigned int sink) {
    // Write minimal empty module definition
    // RawModuleDef::V9 variant = 1, then all empty lists
    unsigned char data[] = {
        1,             // RawModuleDef::V9 (variant 1)
        0, 0, 0, 0,    // typespace: 0 types (empty Vec)
        0, 0, 0, 0,    // tables: 0 tables (empty Vec)
        0, 0, 0, 0,    // reducers: 0 reducers (empty Vec)
        0, 0, 0, 0,    // types: 0 types (empty Vec)  
        0, 0, 0, 0,    // misc_exports: 0 exports (empty Vec)
        0, 0, 0, 0     // row_level_security: 0 entries (empty Vec)
    };
    unsigned int len = sizeof(data);
    bytes_sink_write(sink, data, &len);
}

// Minimal reducer call handler  
__attribute__((export_name("__call_reducer__")))
short __call_reducer__(
    unsigned int id,
    unsigned long long sender_0, unsigned long long sender_1, 
    unsigned long long sender_2, unsigned long long sender_3,
    unsigned long long conn_id_0, unsigned long long conn_id_1,
    unsigned long long timestamp, 
    unsigned int args_source, 
    unsigned int error_sink
) {
    // No reducers defined, so just return error for any call
    return -1;
}

#ifdef __cplusplus
}
#endif