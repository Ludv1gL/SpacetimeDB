/**
 * SpacetimeDB C++ SDK Test Module - Simple version without X-Macro
 * 
 * This module tests basic primitive types manually
 */

#include <spacetimedb/abi/spacetimedb_abi.h>
#include <spacetimedb/bsatn_all.h>
#include <cstdint>
#include <vector>
#include <string>

using namespace SpacetimeDb;

// Module definition structures
struct ModuleTables {
    struct TableDef {
        const char* name;
        bool is_public;
        uint32_t table_id;
        void (*write_schema)(bsatn::Writer&);
    };
    
    static std::vector<TableDef> tables;
    static std::vector<void(*)()> reducers;
};

std::vector<ModuleTables::TableDef> ModuleTables::tables;
std::vector<void(*)()> ModuleTables::reducers;

// Helper to write module definition
void write_module_def(uint32_t sink) {
    bsatn::Writer writer;
    
    // RawModuleDef::V9 (variant 1)
    writer.write_u8(1);
    
    // Typespace - write number of types
    writer.write_u32_le(ModuleTables::tables.size());
    
    // Write each table's type definition
    for (const auto& table : ModuleTables::tables) {
        table.write_schema(writer);
    }
    
    // Tables - write number of tables
    writer.write_u32_le(ModuleTables::tables.size());
    
    // Write each table definition
    for (size_t i = 0; i < ModuleTables::tables.size(); i++) {
        const auto& table = ModuleTables::tables[i];
        
        // Table name
        writer.write_string(table.name);
        
        // Product type ref (index into typespace)
        writer.write_u32_le(i);
        
        // Primary key - None (1)
        writer.write_u8(1);
        
        // Constraints - empty vector
        writer.write_u32_le(0);
        
        // Sequences - empty vector
        writer.write_u32_le(0);
        
        // Schedules - empty vector
        writer.write_u32_le(0);
        
        // Indexes - empty vector
        writer.write_u32_le(0);
        
        // Is public
        writer.write_bool(table.is_public);
        
        // Table type
        writer.write_u8(0); // User = 0
        
        // Row level security
        writer.write_u8(1); // None
    }
    
    // Reducers
    writer.write_u32_le(ModuleTables::reducers.size());
    for (size_t i = 0; i < ModuleTables::reducers.size(); i++) {
        // This would write reducer definitions
        // For now, keeping it simple
    }
    
    // Misc exports - empty
    writer.write_u32_le(0);
    
    // Write to sink
    auto buffer = writer.take_buffer();
    size_t len = buffer.size();
    bytes_sink_write(sink, buffer.data(), &len);
}

// Table: one_i8
void write_one_i8_schema(bsatn::Writer& writer) {
    // Product type (2)
    writer.write_u8(2);
    
    // Number of fields (1)
    writer.write_u32_le(1);
    
    // Field: n (i8)
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(6); // I8 = 6
}

void insert_one_i8(int8_t n) {
    bsatn::Writer writer;
    writer.write_i8(n);
    
    auto buffer = writer.take_buffer();
    size_t len = buffer.size();
    
    // Get table ID for one_i8
    uint32_t table_id;
    const char* table_name = "one_i8";
    table_id_from_name(reinterpret_cast<const uint8_t*>(table_name), strlen(table_name), &table_id);
    
    // Insert the row
    datastore_insert_bsatn(table_id, buffer.data(), &len);
}

// Module exports
extern "C" {
    __attribute__((export_name("__describe_module__")))
    void __describe_module__(uint32_t sink) {
        // Register tables
        ModuleTables::tables.clear();
        ModuleTables::tables.push_back({"one_i8", true, 0, write_one_i8_schema});
        
        write_module_def(sink);
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
        if (id == 0) {
            // Read i8 argument
            size_t len = 1;
            uint8_t buf[1];
            bytes_source_read(args_source, buf, &len);
            
            insert_one_i8(static_cast<int8_t>(buf[0]));
            return 0;
        }
        return -1;
    }
}