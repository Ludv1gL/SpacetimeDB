#ifndef SPACETIMEDB_INTERNAL_MODULE_H
#define SPACETIMEDB_INTERNAL_MODULE_H

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <typeinfo>

#include "spacetimedb/bsatn/bsatn.h"
#include "spacetimedb/internal/autogen/RawModuleDefV9.g.h"
#include "spacetimedb/internal/autogen/RawModuleDef.g.h"
#include "spacetimedb/internal/autogen/RawTableDefV9.g.h"
#include "spacetimedb/internal/autogen/RawReducerDefV9.g.h"
#include "spacetimedb/internal/autogen/RawRowLevelSecurityDefV9.g.h"
#include "spacetimedb/internal/autogen/RawTypeDefV9.g.h"
#include "spacetimedb/internal/autogen/RawScopedTypeNameV9.g.h"
#include "spacetimedb/internal/autogen/AlgebraicType.g.h"
#include "spacetimedb/internal/IReducer.h"
#include "spacetimedb/internal/ITable.h"
#include "spacetimedb/internal/FFI.h"

namespace SpacetimeDb {
namespace Internal {

// Forward declarations
class IReducerContext;
class TypeRegistrar;

// Type reference for the algebraic type system
struct AlgebraicTypeRef {
    uint32_t idx;
    
    explicit AlgebraicTypeRef(uint32_t idx) : idx(idx) {}
};

// Interface for type registration
class ITypeRegistrar {
public:
    virtual ~ITypeRegistrar() = default;
    
    // Register a type and get its reference
    template<typename T>
    AlgebraicTypeRef RegisterType(std::function<std::vector<uint8_t>(AlgebraicTypeRef)> makeType) {
        // Delegate to implementation
        return RegisterTypeImpl(typeid(T).name(), makeType);
    }
    
protected:
    // Implementation method for derived classes
    virtual AlgebraicTypeRef RegisterTypeImpl(const std::string& typeName, 
                                               std::function<std::vector<uint8_t>(AlgebraicTypeRef)> makeType) = 0;
};

// Reducer function type - matches Rust pattern
using ReducerFn = std::function<FFI::Errno(sdk::ReducerContext, const uint8_t*, size_t)>;

// Module class - manages module definition and registration
class Module {
private:
    // Singleton instance
    static Module& Instance();
    
    // Module definition
    RawModuleDefV9 moduleDef;
    
    // Registered reducers - stored as functions for direct dispatch
    std::vector<ReducerFn> reducerFns;
    
    // Reducer metadata for error reporting
    std::vector<std::string> reducerNames;
    
    // Type registrar
    std::unique_ptr<TypeRegistrar> typeRegistrar;
    
    // Private constructor for singleton
    Module();
    
public:
    // Delete copy constructor and assignment
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
    
    // Set the reducer context constructor
    static void SetReducerContextConstructor(
        std::function<std::unique_ptr<IReducerContext>(
            sdk::Identity, std::optional<sdk::ConnectionId>, uint64_t, sdk::Timestamp
        )> ctor
    );
    
    // Register a reducer with a simpler pattern
    template<typename R>
    static void RegisterReducer() {
        static_assert(std::is_base_of_v<IReducer, R>, "R must derive from IReducer");
        auto reducer = std::make_unique<R>();
        Instance().RegisterReducerImpl(std::move(reducer));
    }
    
    // Direct reducer registration (Rust-like)
    static void RegisterReducerDirect(const std::string& name, ReducerFn fn) {
        Instance().RegisterReducerDirectImpl(name, fn);
    }
    
    // Direct table registration (Rust-like)
    static void RegisterTableDirect(const std::string& name, 
                                   TableAccess access,
                                   std::function<std::vector<uint8_t>()> typeGen) {
        Instance().RegisterTableDirectImpl(name, access, typeGen);
    }
    
    // Register a table
    template<typename T, typename View>
    static void RegisterTable() {
        static_assert(std::is_base_of_v<ITableView<View, T>, View>, 
                      "View must derive from ITableView<View, T>");
        Instance().RegisterTableImpl(View::MakeTableDesc(*Instance().typeRegistrar));
    }
    
    // Register client visibility filter (row level security)
    static void RegisterClientVisibilityFilter(const std::string& sql);
    
    // FFI exports
    static void __describe_module__(FFI::BytesSink description);
    static FFI::Errno __call_reducer__(
        uint32_t id,
        uint64_t sender_0, uint64_t sender_1, uint64_t sender_2, uint64_t sender_3,
        uint64_t conn_id_0, uint64_t conn_id_1,
        sdk::Timestamp timestamp,
        FFI::BytesSource args,
        FFI::BytesSink error
    );
    
private:
    void RegisterReducerImpl(std::unique_ptr<IReducer> reducer);
    void RegisterReducerDirectImpl(const std::string& name, ReducerFn fn);
    void RegisterTableImpl(const RawTableDefV9& table);
    void RegisterTableDirectImpl(const std::string& name, TableAccess access, std::function<std::vector<uint8_t>()> typeGen);
    
    // Add type to typespace and register its name
    AlgebraicTypeRef RegisterTypeGeneric(const std::string& typeName,
                                         std::function<std::vector<uint8_t>(AlgebraicTypeRef)> makeType) {
        auto& types = moduleDef.typespace.types;
        AlgebraicTypeRef typeRef(static_cast<uint32_t>(types.size()));
        
        // Reserve slot with empty type
        types.push_back(AlgebraicType{});
        
        // Generate the actual type
        auto typeBytes = makeType(typeRef);
        types[typeRef.idx] = AlgebraicType(std::move(typeBytes));
        
        // Register type name
        RawScopedTypeNameV9 scopedName;
        scopedName.name = typeName; // Use provided type name
        
        RawTypeDefV9 typeDef;
        typeDef.name = scopedName;
        typeDef.ty = typeRef.idx;
        typeDef.custom_ordering = true;
        
        moduleDef.types.push_back(std::move(typeDef));
        
        return typeRef;
    }
};

// Type registrar implementation
class TypeRegistrar : public ITypeRegistrar {
private:
    Module& module;
    std::unordered_map<std::string, AlgebraicTypeRef> types;
    
public:
    explicit TypeRegistrar(Module& module) : module(module) {}
    
protected:
    AlgebraicTypeRef RegisterTypeImpl(const std::string& typeName, 
                                      std::function<std::vector<uint8_t>(AlgebraicTypeRef)> makeType) override {
        // Check if type already registered
        auto it = types.find(typeName);
        if (it != types.end()) {
            return it->second;
        }
        
        // Register new type
        auto typeRef = module.RegisterTypeGeneric(typeName, makeType);
        types[typeName] = typeRef;
        return typeRef;
    }
};

// Helper to consume bytes from BytesSource
std::vector<uint8_t> ConsumeBytes(FFI::BytesSource source);

// Helper to write bytes to BytesSink
void WriteBytes(FFI::BytesSink sink, const std::vector<uint8_t>& bytes);

// Legacy support functions removed - use Module API instead

} // namespace Internal
} // namespace SpacetimeDb

#endif // SPACETIMEDB_INTERNAL_MODULE_H