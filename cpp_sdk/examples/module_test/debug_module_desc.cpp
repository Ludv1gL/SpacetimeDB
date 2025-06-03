// Register one table
#define SPACETIMEDB_TABLES_LIST \
    X(TestTable, test_table, true)

#include <spacetimedb/spacetimedb.h>
#include <cstdio>
#include <vector>

// Minimal module to debug module description
struct TestTable {
    uint8_t value;
};

using namespace spacetimedb;

// Simple reducer
SPACETIMEDB_REDUCER(test_reducer, ReducerContext ctx, uint8_t val) {
    TestTable row{val};
    ctx.db.test_table().insert(row);
}

// Hook to capture module description
extern "C" {
    __attribute__((export_name("__describe_module_debug__")))
    void __describe_module_debug__(uint32_t sink) {
        spacetimedb::initialize_module();
        
        // Instead of writing to sink, let's write to a buffer and print it
        std::vector<uint8_t> debug_buffer;
        auto& module = spacetimedb::ModuleDef::instance();
        
        // RawModuleDef::V9 tag
        debug_buffer.push_back(1);
        
        // Typespace
        spacetimedb::write_u32(debug_buffer, module.tables.size());
        
        // Let's print the first few bytes to debug
        printf("First bytes of module description:\n");
        for (size_t i = 0; i < std::min<size_t>(debug_buffer.size(), 20); i++) {
            printf("0x%02x ", debug_buffer[i]);
        }
        printf("\n");
        
        // Continue writing to check where 0x69 appears
        for (const auto& table : module.tables) {
            table.write_schema(debug_buffer);
        }
        
        // Tables
        spacetimedb::write_u32(debug_buffer, module.tables.size());
        
        size_t offset_before_tables = debug_buffer.size();
        
        for (size_t i = 0; i < module.tables.size(); i++) {
            const auto& table = module.tables[i];
            
            spacetimedb::write_string(debug_buffer, table.name);
            spacetimedb::write_u32(debug_buffer, i);  // product_type_ref
            spacetimedb::write_u32(debug_buffer, 0);  // primary_key (empty)
            spacetimedb::write_u32(debug_buffer, 0);  // indexes (empty)
            spacetimedb::write_u32(debug_buffer, 0);  // constraints (empty)
            spacetimedb::write_u32(debug_buffer, 0);  // sequences (empty)
            
            size_t offset_before_option = debug_buffer.size();
            debug_buffer.push_back(1);  // schedule (Option::None = tag 1)
            
            printf("Offset %zu: Added schedule Option::None (0x%02x)\n", 
                   offset_before_option, debug_buffer[offset_before_option]);
            
            debug_buffer.push_back(1);  // table_type
            debug_buffer.push_back(table.is_public ? 0 : 1);  // access
        }
        
        // Find any 0x69 in the buffer
        printf("\nSearching for 0x69 in buffer of size %zu:\n", debug_buffer.size());
        for (size_t i = 0; i < debug_buffer.size(); i++) {
            if (debug_buffer[i] == 0x69) {
                printf("Found 0x69 at offset %zu\n", i);
                // Print context
                printf("Context: ");
                for (size_t j = std::max<size_t>(0, i - 5); j < std::min(i + 5, debug_buffer.size()); j++) {
                    printf("0x%02x ", debug_buffer[j]);
                }
                printf("\n");
            }
        }
        
        // Write the actual data to sink
        spacetimedb::spacetimedb_write_module_def(sink);
    }
}