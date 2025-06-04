#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include "spacetimedb/abi/spacetimedb_abi.h"
#include "spacetimedb/bsatn/writer.h"
#include "spacetimedb/bsatn/reader.h"
#include "spacetimedb/sdk/reducer_context.h"

namespace spacetimedb {

// AlgebraicType discriminants
enum class AlgebraicTypeTag : uint8_t {
    Ref = 0,
    Sum = 1,
    Product = 2,
    Array = 3,
    String = 4,
    Bool = 5,
    I8 = 6,
    U8 = 7,
    I16 = 8,
    U16 = 9,
    I32 = 10,
    U32 = 11,
    I64 = 12,
    U64 = 13,
    I128 = 14,
    U128 = 15,
    I256 = 16,
    U256 = 17,
    F32 = 18,
    F64 = 19
};

// Table registration
struct TableInfo {
    std::string name;
    uint32_t type_ref;
    bool is_public;
    std::function<void(SpacetimeDb::bsatn::Writer&)> write_schema;
};

// Reducer registration
struct ReducerInfo {
    std::string name;
    std::function<void(ReducerContext&, uint32_t, uint32_t)> handler;
    std::function<void(SpacetimeDb::bsatn::Writer&)> write_params;
};

// Module registry
class ModuleRegistry {
public:
    static ModuleRegistry& instance() {
        static ModuleRegistry registry;
        return registry;
    }
    
    void register_table(const TableInfo& table) {
        tables.push_back(table);
    }
    
    void register_reducer(const ReducerInfo& reducer) {
        reducers.push_back(reducer);
    }
    
    void register_type(uint32_t type_ref, std::function<void(SpacetimeDb::bsatn::Writer&)> write_type) {
        types[type_ref] = write_type;
    }
    
    const std::vector<TableInfo>& get_tables() const { return tables; }
    const std::vector<ReducerInfo>& get_reducers() const { return reducers; }
    const std::unordered_map<uint32_t, std::function<void(SpacetimeDb::bsatn::Writer&)>>& get_types() const { return types; }
    
private:
    std::vector<TableInfo> tables;
    std::vector<ReducerInfo> reducers;
    std::unordered_map<uint32_t, std::function<void(SpacetimeDb::bsatn::Writer&)>> types;
};

// Table handle for type-safe operations
template<typename T>
class TableHandle {
public:
    TableHandle(const std::string& name) : table_name(name) {}
    
    void insert(const T& row) {
        // Get table ID
        uint32_t table_id = 0;
        uint16_t err = spacetime_table_id_from_name(
            reinterpret_cast<const uint8_t*>(table_name.c_str()),
            table_name.length(),
            &table_id
        );
        
        if (err != 0) {
            throw std::runtime_error("Failed to get table ID for " + table_name);
        }
        
        // Serialize row
        SpacetimeDb::bsatn::Writer w;
        SpacetimeDb::bsatn::serialize(w, row);
        
        auto buffer = w.take_buffer();
        size_t len = buffer.size();
        
        // Insert row
        err = spacetime_datastore_insert_bsatn(table_id, buffer.data(), &len);
        if (err != 0) {
            throw std::runtime_error("Failed to insert row into " + table_name);
        }
    }
    
private:
    std::string table_name;
};

// Module database accessor
class ModuleDatabase {
public:
    template<typename T>
    TableHandle<T> table(const std::string& name) {
        return TableHandle<T>(name);
    }
};

// Logging utilities
inline void log_info(const std::string& msg) {
    spacetime_console_log(
        2, // Info level
        nullptr, 0,
        reinterpret_cast<const uint8_t*>(__FILE__), strlen(__FILE__),
        __LINE__,
        reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length()
    );
}

inline void log_error(const std::string& msg) {
    spacetime_console_log(
        4, // Error level
        nullptr, 0,
        reinterpret_cast<const uint8_t*>(__FILE__), strlen(__FILE__),
        __LINE__,
        reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length()
    );
}

} // namespace spacetimedb

// Macros for module definition
#define SPACETIMEDB_MODULE_INIT() \
    static struct ModuleInitializer { \
        ModuleInitializer() {

#define SPACETIMEDB_MODULE_END() \
        } \
    } module_initializer;

// Updated table macro
#define SPACETIMEDB_TABLE(Type, name, is_public) \
    namespace { \
        struct Register_##name { \
            Register_##name() { \
                spacetimedb::ModuleRegistry::instance().register_table({ \
                    #name, \
                    spacetimedb::ModuleRegistry::instance().get_types().size(), \
                    is_public, \
                    [](SpacetimeDb::bsatn::Writer& w) { \
                        /* Write Product type */ \
                        w.write_u8(static_cast<uint8_t>(spacetimedb::AlgebraicTypeTag::Product)); \
                        /* Write fields */ \
                        Type::write_schema(w); \
                    } \
                }); \
            } \
        }; \
        static Register_##name register_##name; \
    }

// Updated reducer macro
#define SPACETIMEDB_REDUCER(name, ...) \
    void name(__VA_ARGS__); \
    namespace { \
        struct Register_##name { \
            Register_##name() { \
                spacetimedb::ModuleRegistry::instance().register_reducer({ \
                    #name, \
                    [](spacetimedb::ReducerContext& ctx, uint32_t args, uint32_t error) { \
                        /* TODO: Deserialize args and call reducer */ \
                        name(ctx); \
                    }, \
                    [](SpacetimeDb::bsatn::Writer& w) { \
                        /* TODO: Write parameter types */ \
                    } \
                }); \
            } \
        }; \
        static Register_##name register_##name; \
    } \
    void name(__VA_ARGS__)