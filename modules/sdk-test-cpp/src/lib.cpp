// SpacetimeDB C++ Module using minimal SDK
#include "spacetimedb_minimal.h"

// Define a simple table structure
struct OneU8 {
    uint8_t n;
    
    // Serialize method for BSATN
    bool serialize(SimpleBsatnWriter& writer) const {
        return writer.write_u8(n);
    }
};

// Module exports
extern "C" {

// Describe module - start simple and build up
STDB_EXPORT(__describe_module__)
void __describe_module__(uint32_t sink) {
    uint8_t buffer[1024];
    SimpleBsatnWriter writer(buffer, sizeof(buffer));
    
    // RawModuleDef::V9
    writer.write_u8(1);
    
    // typespace: 1 type for our table
    writer.write_u32_le(1);
    
    // Type 0: Product type for OneU8
    writer.write_u8(2); // AlgebraicType::Product
    writer.write_u32_le(1); // 1 field in product
    // Field 0: "n" : U8
    writer.write_u8(1); // Option::Some
    writer.write_string("n");
    writer.write_u8(12); // AlgebraicType::U8
    
    // tables: 1 table
    writer.write_u32_le(1);
    
    // Table 0: "one_u8"
    writer.write_string("one_u8");
    writer.write_u32_le(0); // product_type_ref = type 0
    writer.write_u32_le(0); // primary_key = empty array
    writer.write_u32_le(0); // indexes = empty array
    writer.write_u32_le(0); // constraints = empty array
    writer.write_u32_le(0); // sequences = empty array
    writer.write_u8(0); // schedule = None
    writer.write_u8(0); // table_type = User (variant 0)
    writer.write_u8(0); // table_access = Public (variant 0)
    
    // reducers: 1 reducer
    writer.write_u32_le(1);
    
    // Reducer: no_op
    writer.write_string("no_op");
    writer.write_u32_le(0); // params: 0 elements (empty ProductType)
    writer.write_u8(1); // lifecycle = Some
    writer.write_u8(0); // UserDefined
    
    // Empty arrays
    writer.write_u32_le(0); // types
    writer.write_u32_le(0); // misc_exports
    writer.write_u32_le(0); // row_level_security
    
    size_t len = writer.get_position();
    bytes_sink_write(sink, buffer, &len);
}

// Handle reducer calls
STDB_EXPORT(__call_reducer__)
int16_t __call_reducer__(
    uint32_t id,
    uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
    uint64_t conn_id_0, uint64_t conn_id_1,
    uint64_t timestamp, 
    uint32_t args, 
    uint32_t error
) {
    if (id == 0) {
        // no_op reducer - just log and succeed
        log_info("no_op reducer called");
        return 0;
    }
    
    return -999; // No such reducer
}

} // extern "C"