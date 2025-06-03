// Debug string serialization
#define SPACETIMEDB_TABLES_LIST \
    X(DebugTable, debug_table, true)

#include <spacetimedb/spacetimedb.h>
#include <cstdio>

using namespace spacetimedb;

struct DebugTable {
    uint8_t n;
};

SPACETIMEDB_REDUCER(debug_reducer, ReducerContext ctx, uint8_t val) {
    DebugTable row{val};
    ctx.db.debug_table().insert(row);
}

// Override describe_module to debug
extern "C" {
    __attribute__((export_name("__describe_module_debug__")))
    void __describe_module_debug__(uint32_t sink) {
        std::vector<uint8_t> buf;
        
        // Test string serialization
        std::string test_str = "test";
        write_string(buf, test_str);
        
        printf("String '%s' serialized as:\n", test_str.c_str());
        printf("Length bytes: ");
        for (int i = 0; i < 4; i++) {
            printf("0x%02x ", buf[i]);
        }
        printf("\n");
        
        // The length should be 4 (0x04 0x00 0x00 0x00 in little-endian)
        printf("Length value: %u\n", (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | 
               ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24));
        
        printf("String bytes: ");
        for (size_t i = 4; i < buf.size(); i++) {
            printf("0x%02x('%c') ", buf[i], buf[i]);
        }
        printf("\n");
        
        // Call the real describe_module
        spacetimedb::initialize_module();
        spacetimedb::spacetimedb_write_module_def(sink);
    }
}