#include "../../../sdk/include/spacetimedb/spacetimedb_core.h"
#include <cstdio>
#include <type_traits>

// =============================================================================
// SIMPLE TEST TABLES
// =============================================================================

struct OneU8 {
    uint8_t n;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneU8& value) {
        buffer.push_back(value.n);
    }
};
SPACETIMEDB_TABLE(OneU8, one_u8, true)

struct OneU32 {
    uint32_t n;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneU32& value) {
        buffer.push_back(value.n & 0xFF);
        buffer.push_back((value.n >> 8) & 0xFF);
        buffer.push_back((value.n >> 16) & 0xFF);
        buffer.push_back((value.n >> 24) & 0xFF);
    }
};
SPACETIMEDB_TABLE(OneU32, one_u32, true)

struct OneString {
    std::string s;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneString& value) {
        uint32_t len = value.s.length();
        buffer.push_back(len & 0xFF);
        buffer.push_back((len >> 8) & 0xFF);
        buffer.push_back((len >> 16) & 0xFF);
        buffer.push_back((len >> 24) & 0xFF);
        buffer.insert(buffer.end(), value.s.begin(), value.s.end());
    }
};
SPACETIMEDB_TABLE(OneString, one_string, true)

struct OneIdentity {
    SpacetimeDb::Identity i;
    
    static void spacetimedb_serialize(std::vector<uint8_t>& buffer, const OneIdentity& value) {
        buffer.insert(buffer.end(), value.i.data, value.i.data + 32);
    }
};
SPACETIMEDB_TABLE(OneIdentity, one_identity, true)

// =============================================================================
// SIMPLE REDUCERS
// =============================================================================

SPACETIMEDB_REDUCER(insert_one_u8, SpacetimeDb::ReducerContext ctx) {
    OneU8 row{42};
    
    auto table_id = ctx.db->table<OneU8>("one_u8").get_table_id();
    if (table_id != 0) {
        std::vector<uint8_t> buffer;
        OneU8::spacetimedb_serialize(buffer, row);
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
        if (result == 0) {
            LOG_INFO("Successfully inserted u8 value 42");
        } else {
            LOG_ERROR("Failed to insert u8 value");
        }
    }
}

SPACETIMEDB_REDUCER(insert_one_u32, SpacetimeDb::ReducerContext ctx) {
    OneU32 row{123456};
    
    auto table_id = ctx.db->table<OneU32>("one_u32").get_table_id();
    if (table_id != 0) {
        std::vector<uint8_t> buffer;
        OneU32::spacetimedb_serialize(buffer, row);
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
        if (result == 0) {
            LOG_INFO("Successfully inserted u32 value 123456");
        } else {
            LOG_ERROR("Failed to insert u32 value");
        }
    }
}

SPACETIMEDB_REDUCER(insert_one_string, SpacetimeDb::ReducerContext ctx) {
    OneString row{"Hello from C++ SDK test!"};
    
    auto table_id = ctx.db->table<OneString>("one_string").get_table_id();
    if (table_id != 0) {
        std::vector<uint8_t> buffer;
        OneString::spacetimedb_serialize(buffer, row);
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
        if (result == 0) {
            LOG_INFO("Successfully inserted string value");
        } else {
            LOG_ERROR("Failed to insert string value");
        }
    }
}

SPACETIMEDB_REDUCER(insert_caller_identity, SpacetimeDb::ReducerContext ctx) {
    OneIdentity row{ctx.sender};
    
    auto table_id = ctx.db->table<OneIdentity>("one_identity").get_table_id();
    if (table_id != 0) {
        std::vector<uint8_t> buffer;
        OneIdentity::spacetimedb_serialize(buffer, row);
        size_t buffer_size = buffer.size();
        uint16_t result = datastore_insert_bsatn(table_id, buffer.data(), &buffer_size);
        if (result == 0) {
            LOG_INFO("Successfully inserted caller identity");
        } else {
            LOG_ERROR("Failed to insert caller identity");
        }
    }
}

SPACETIMEDB_REDUCER(no_op_succeeds, SpacetimeDb::ReducerContext ctx) {
    LOG_INFO("No-op reducer succeeded");
}

SPACETIMEDB_INIT(init) {
    LOG_INFO("C++ SDK simple test module initialized");
    
    // Test timestamp
    char ts_buf[64];
    snprintf(ts_buf, sizeof(ts_buf), "Init timestamp: %llu", (unsigned long long)ctx.timestamp);
    LOG_INFO(ts_buf);
    
    // Test module identity
    auto module_id = ctx.identity();
    char id_buf[128];
    snprintf(id_buf, sizeof(id_buf), "Module identity starts with: %02x%02x%02x%02x",
             module_id.data[0], module_id.data[1], module_id.data[2], module_id.data[3]);
    LOG_INFO(id_buf);
}

// =============================================================================
// MODULE EXPORTS
// =============================================================================

std::vector<uint8_t> build_simple_module_definition() {
    std::vector<uint8_t> module_bytes;
    SpacetimeDb::BsatnWriter writer(module_bytes);
    
    // RawModuleDef::V9 tag
    writer.write_u8(1);
    
    // 1. typespace: 4 types (OneU8, OneU32, OneString, OneIdentity)
    writer.write_vec_len(4); // 4 types
    
    // Type 0: OneU8
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_vec_len(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(7); // U8
    
    // Type 1: OneU32
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_vec_len(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("n");
    writer.write_u8(9); // U32
    
    // Type 2: OneString
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_vec_len(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("s");
    writer.write_u8(15); // String
    
    // Type 3: OneIdentity
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_vec_len(1); // 1 field
    writer.write_u8(0); // Some
    writer.write_string("i");
    // Identity is actually U256
    writer.write_u8(10); // AlgebraicType::U256
    
    // names: empty
    writer.write_vec_len(0);
    
    // 2. tables: 4 tables
    writer.write_vec_len(4);
    
    // Table 0: one_u8
    writer.write_string("one_u8");
    writer.write_u32(0); // type ref 0
    writer.write_u8(1);  // primary_key: None
    writer.write_vec_len(0); // indexes
    writer.write_vec_len(0); // constraints
    writer.write_vec_len(0); // sequences
    writer.write_u8(1);  // schedule: None
    writer.write_u8(0);  // table_type: User
    writer.write_u8(0);  // table_access: Public
    
    // Table 1: one_u32
    writer.write_string("one_u32");
    writer.write_u32(1); // type ref 1
    writer.write_u8(1);  // primary_key: None
    writer.write_vec_len(0); // indexes
    writer.write_vec_len(0); // constraints
    writer.write_vec_len(0); // sequences
    writer.write_u8(1);  // schedule: None
    writer.write_u8(0);  // table_type: User
    writer.write_u8(0);  // table_access: Public
    
    // Table 2: one_string
    writer.write_string("one_string");
    writer.write_u32(2); // type ref 2
    writer.write_u8(1);  // primary_key: None
    writer.write_vec_len(0); // indexes
    writer.write_vec_len(0); // constraints
    writer.write_vec_len(0); // sequences
    writer.write_u8(1);  // schedule: None
    writer.write_u8(0);  // table_type: User
    writer.write_u8(0);  // table_access: Public
    
    // Table 3: one_identity
    writer.write_string("one_identity");
    writer.write_u32(3); // type ref 3
    writer.write_u8(1);  // primary_key: None
    writer.write_vec_len(0); // indexes
    writer.write_vec_len(0); // constraints
    writer.write_vec_len(0); // sequences
    writer.write_u8(1);  // schedule: None
    writer.write_u8(0);  // table_type: User
    writer.write_u8(0);  // table_access: Public
    
    // 3. reducers: 6 reducers
    writer.write_vec_len(6);
    
    // Reducer 0: init
    writer.write_string("init");
    writer.write_vec_len(0); // params
    writer.write_u8(0); // Some(Init)
    writer.write_u8(0); // Init lifecycle
    
    // Reducer 1: no_op_succeeds
    writer.write_string("no_op_succeeds");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // Reducer 2: insert_one_u8
    writer.write_string("insert_one_u8");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // Reducer 3: insert_one_u32
    writer.write_string("insert_one_u32");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // Reducer 4: insert_one_string
    writer.write_string("insert_one_string");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // Reducer 5: insert_caller_identity
    writer.write_string("insert_caller_identity");
    writer.write_vec_len(0); // params
    writer.write_u8(1); // None lifecycle
    
    // 4. types: empty
    writer.write_vec_len(0);
    
    // 5. misc_exports: empty
    writer.write_vec_len(0);
    
    // 6. row_level_security: empty
    writer.write_vec_len(0);
    
    return module_bytes;
}

extern "C" __attribute__((export_name("__describe_module__"))) 
void __describe_module__(uint32_t description) {
    auto module_def = build_simple_module_definition();
    
    size_t total_size = module_def.size();
    size_t written = 0;
    
    while (written < total_size) {
        size_t chunk_size = std::min(size_t(1024), total_size - written);
        size_t write_size = chunk_size;
        
        uint16_t result = bytes_sink_write(description, 
                                          module_def.data() + written, 
                                          &write_size);
        
        if (result != 0) {
            break;
        }
        
        written += write_size;
    }
}

extern "C" __attribute__((export_name("__call_reducer__"))) 
int16_t __call_reducer__(uint32_t id, 
                        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
                        uint64_t conn_id_0, uint64_t conn_id_1, 
                        uint64_t timestamp,
                        uint32_t args, uint32_t error) {
    try {
        // Construct identity from parameters
        auto sender_identity = identity_from_params(sender_0, sender_1, sender_2, sender_3);
        
        // Construct connection ID if valid
        std::optional<SpacetimeDb::ConnectionId> conn_id;
        if (conn_id_0 != 0 || conn_id_1 != 0) {
            conn_id = SpacetimeDb::ConnectionId(conn_id_0, conn_id_1);
        }
        
        // Create reducer context
        SpacetimeDb::ReducerContext ctx(SpacetimeDb::get_module_db(), 
                                       sender_identity, 
                                       timestamp, 
                                       conn_id);
        
        // Dispatch to reducers
        if (!SpacetimeDb::ReducerDispatcher::instance().call_reducer(id, ctx, args)) {
            return 1; // Unknown reducer
        }
        
        return 0; // Success
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("Error in __call_reducer__: ") + e.what());
        return 1;
    }
}