#ifndef SPACETIMEDB_MODULE_H
#define SPACETIMEDB_MODULE_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

// Forward declarations
namespace SpacetimeDb {
namespace sdk {
    class ReducerContext;
}
namespace Internal {
namespace FFI {
    enum class Errno;
    typedef Errno (*ReducerFn)(sdk::ReducerContext, const uint8_t*, size_t);
}
}
}

namespace SpacetimeDb {
namespace Internal {

// Table access levels
enum class TableAccess {
    Public,
    Private
};

// Module registration class - singleton pattern
class Module {
public:
    static Module& instance() {
        static Module inst;
        return inst;
    }

    // Table registration
    static void RegisterTable(
        const char* table_name,
        TableAccess access,
        const char* scheduled_reducer,
        const char* scheduled_at,
        std::function<std::vector<uint8_t>()> type_generator
    ) {
        instance()._registerTable(table_name, access, scheduled_reducer, scheduled_at, type_generator);
    }

    // Reducer registration with kind
    static void RegisterReducer(
        const char* reducer_name,
        FFI::ReducerFn reducer_fn,
        int kind
    ) {
        instance()._registerReducer(reducer_name, reducer_fn, kind);
    }

    // Index registration methods
    static void RegisterBTreeIndex(
        const char* index_name,
        const char* table_name,
        const std::vector<std::string>& columns
    ) {
        instance()._registerBTreeIndex(index_name, table_name, columns);
    }

    static void RegisterHashIndex(
        const char* index_name,
        const char* table_name,
        const char* column_name
    ) {
        instance()._registerHashIndex(index_name, table_name, column_name);
    }

    static void RegisterUniqueIndex(
        const char* index_name,
        const char* table_name,
        const char* column_name
    ) {
        instance()._registerUniqueIndex(index_name, table_name, column_name);
    }

    static void RegisterPrimaryKeyIndex(
        const char* table_name,
        const char* column_name
    ) {
        instance()._registerPrimaryKeyIndex(table_name, column_name);
    }

    // Client visibility filter
    static void RegisterClientVisibilityFilter(
        const char* filter_name,
        const char* sql_query
    ) {
        instance()._registerClientVisibilityFilter(filter_name, sql_query);
    }

    // Module metadata
    static void SetVersion(int major, int minor, int patch) {
        instance()._setVersion(major, minor, patch);
    }

    static void SetMetadata(
        const char* name,
        const char* author,
        const char* description,
        const char* license
    ) {
        instance()._setMetadata(name, author, description, license);
    }

private:
    Module() = default;
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    // Internal implementation methods
    void _registerTable(
        const char* table_name,
        TableAccess access,
        const char* scheduled_reducer,
        const char* scheduled_at,
        std::function<std::vector<uint8_t>()> type_generator
    );

    void _registerReducer(
        const char* reducer_name,
        FFI::ReducerFn reducer_fn,
        int kind
    );

    void _registerBTreeIndex(
        const char* index_name,
        const char* table_name,
        const std::vector<std::string>& columns
    );

    void _registerHashIndex(
        const char* index_name,
        const char* table_name,
        const char* column_name
    );

    void _registerUniqueIndex(
        const char* index_name,
        const char* table_name,
        const char* column_name
    );

    void _registerPrimaryKeyIndex(
        const char* table_name,
        const char* column_name
    );

    void _registerClientVisibilityFilter(
        const char* filter_name,
        const char* sql_query
    );

    void _setVersion(int major, int minor, int patch);

    void _setMetadata(
        const char* name,
        const char* author,
        const char* description,
        const char* license
    );

    // Storage for registered items
    struct TableInfo {
        std::string name;
        TableAccess access;
        std::string scheduled_reducer;
        std::string scheduled_at;
        std::function<std::vector<uint8_t>()> type_generator;
    };

    struct ReducerInfo {
        std::string name;
        FFI::ReducerFn function;
        int kind;
    };

    struct IndexInfo {
        std::string name;
        std::string table_name;
        std::vector<std::string> columns;
        enum Type { BTree, Hash, Unique, PrimaryKey } type;
    };

    std::vector<TableInfo> tables_;
    std::vector<ReducerInfo> reducers_;
    std::vector<IndexInfo> indexes_;
    std::unordered_map<std::string, std::string> visibility_filters_;
    
    struct {
        int major = 0;
        int minor = 0;
        int patch = 0;
    } version_;
    
    struct {
        std::string name;
        std::string author;
        std::string description;
        std::string license;
    } metadata_;

    // Friend functions for FFI exports
    friend extern "C" const uint8_t* __describe_module__();
    friend extern "C" FFI::Errno __call_reducer__(
        const uint8_t* reducer_name,
        size_t reducer_name_len,
        sdk::ReducerContext ctx,
        const uint8_t* args,
        size_t args_len
    );
};

} // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_MODULE_H