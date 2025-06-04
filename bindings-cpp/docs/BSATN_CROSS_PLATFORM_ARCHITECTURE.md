# BSATN Cross-Platform Architecture: Supporting Standard C++ and Unreal Engine

## Overview

This document outlines an architecture for using the SpacetimeDB BSATN library in both standard C++ modules and Unreal Engine clients, which use different type systems (`std::` vs Unreal types like `TArray`, `FString`, etc.).

## Challenge

The current BSATN implementation is tightly coupled to standard C++ types:
- `std::string` for text
- `std::vector<T>` for arrays
- `std::optional<T>` for optional values

Unreal Engine uses its own type system:
- `FString` for text (UTF-16 internally)
- `TArray<T>` for dynamic arrays
- `TOptional<T>` for optional values
- Different memory management and allocation strategies

## Recommended Architecture: Type Adapter System

### 1. Core BSATN Layer (Type-Agnostic)

Create a core BSATN engine that's generic and independent of specific container types:

```cpp
// bsatn_core.h - Core interfaces that don't depend on std:: or Unreal types
namespace spacetimedb::bsatn::core {
    
    // Generic traits that adapters will specialize
    template<typename T>
    struct container_traits {
        // How to get size, iterate, push_back, etc.
    };
    
    template<typename T>
    struct string_traits {
        // How to get c_str, length, construct from char*, etc.
    };
    
    template<typename T>
    struct optional_traits {
        // How to check has_value, get value, construct none/some
    };
    
    template<typename T>
    struct allocator_traits {
        // How to allocate/deallocate memory in the target environment
    };
}
```

### 2. Standard C++ Adapter

Implement specializations for standard C++ types:

```cpp
// bsatn_std.h - Standard C++ specializations
namespace spacetimedb::bsatn::std_adapter {
    
    template<typename T>
    struct container_traits<std::vector<T>> {
        using value_type = T;
        using size_type = size_t;
        
        static size_t size(const std::vector<T>& v) { return v.size(); }
        static void reserve(std::vector<T>& v, size_t n) { v.reserve(n); }
        static void push_back(std::vector<T>& v, T&& item) { 
            v.push_back(std::move(item)); 
        }
        
        // Iterator support
        static auto begin(std::vector<T>& v) { return v.begin(); }
        static auto end(std::vector<T>& v) { return v.end(); }
    };
    
    template<>
    struct string_traits<std::string> {
        static const char* c_str(const std::string& s) { return s.c_str(); }
        static size_t length(const std::string& s) { return s.length(); }
        static std::string from_buffer(const char* data, size_t len) {
            return std::string(data, len);
        }
        static void append(std::string& s, const char* data, size_t len) {
            s.append(data, len);
        }
    };
    
    template<typename T>
    struct optional_traits<std::optional<T>> {
        static bool has_value(const std::optional<T>& opt) { 
            return opt.has_value(); 
        }
        static const T& value(const std::optional<T>& opt) { 
            return *opt; 
        }
        static std::optional<T> make_none() { 
            return std::nullopt; 
        }
        static std::optional<T> make_some(T&& val) { 
            return std::optional<T>(std::move(val)); 
        }
    };
}
```

### 3. Unreal Engine Adapter

Implement specializations for Unreal Engine types:

```cpp
// bsatn_unreal.h - Unreal Engine specializations
namespace spacetimedb::bsatn::unreal_adapter {
    
    template<typename T>
    struct container_traits<TArray<T>> {
        using value_type = T;
        using size_type = int32;
        
        static int32 size(const TArray<T>& arr) { return arr.Num(); }
        static void reserve(TArray<T>& arr, int32 n) { arr.Reserve(n); }
        static void push_back(TArray<T>& arr, T&& item) { 
            arr.Add(MoveTemp(item)); 
        }
        
        // Iterator support
        static auto begin(TArray<T>& arr) { return arr.begin(); }
        static auto end(TArray<T>& arr) { return arr.end(); }
    };
    
    template<>
    struct string_traits<FString> {
        static const TCHAR* c_str(const FString& s) { return *s; }
        static int32 length(const FString& s) { return s.Len(); }
        static FString from_buffer(const char* data, size_t len) {
            // Convert UTF-8 to Unreal's UTF-16
            return FString(len, UTF8_TO_TCHAR(data));
        }
        static TArray<uint8> to_utf8(const FString& s) {
            FTCHARToUTF8 converter(*s);
            return TArray<uint8>((uint8*)converter.Get(), converter.Length());
        }
    };
    
    template<typename T>
    struct optional_traits<TOptional<T>> {
        static bool has_value(const TOptional<T>& opt) { 
            return opt.IsSet(); 
        }
        static const T& value(const TOptional<T>& opt) { 
            return opt.GetValue(); 
        }
        static TOptional<T> make_none() { 
            return TOptional<T>(); 
        }
        static TOptional<T> make_some(T&& val) { 
            return TOptional<T>(MoveTemp(val)); 
        }
    };
}
```

### 4. Policy-Based Serialization

Create a policy-based serialization system that can work with either type system:

```cpp
// bsatn_serialize.h - Policy-based serialization
template<typename Policy>
class BsatnSerializer {
public:
    using string_type = typename Policy::string_type;
    using byte_array_type = typename Policy::byte_array_type;
    template<typename T>
    using array_type = typename Policy::template array_type<T>;
    template<typename T>
    using optional_type = typename Policy::template optional_type<T>;
    
    // Serialization methods that use the policy types
    void serialize_string(Writer& w, const string_type& str) {
        auto len = Policy::string_traits::length(str);
        w.write_u32_le(len);
        // Handle encoding conversion if needed
        if constexpr (Policy::needs_encoding_conversion) {
            auto utf8 = Policy::string_traits::to_utf8(str);
            w.write_bytes_raw(utf8.data(), utf8.size());
        } else {
            w.write_bytes_raw(Policy::string_traits::c_str(str), len);
        }
    }
    
    template<typename T>
    void serialize_array(Writer& w, const array_type<T>& arr) {
        auto size = Policy::container_traits::size(arr);
        w.write_u32_le(size);
        for (const auto& item : arr) {
            serialize(w, item);
        }
    }
    
    template<typename T>
    optional_type<T> deserialize_optional(Reader& r) {
        uint8_t tag = r.read_u8();
        if (tag == 0) {
            return Policy::optional_traits::make_none();
        } else {
            return Policy::optional_traits::make_some(deserialize<T>(r));
        }
    }
};

// Define policies
struct StdPolicy {
    using string_type = std::string;
    using byte_array_type = std::vector<uint8_t>;
    template<typename T>
    using array_type = std::vector<T>;
    template<typename T>
    using optional_type = std::optional<T>;
    
    using string_traits = std_adapter::string_traits<std::string>;
    template<typename T>
    using container_traits = std_adapter::container_traits<std::vector<T>>;
    template<typename T>
    using optional_traits = std_adapter::optional_traits<std::optional<T>>;
    
    static constexpr bool needs_encoding_conversion = false;
};

struct UnrealPolicy {
    using string_type = FString;
    using byte_array_type = TArray<uint8>;
    template<typename T>
    using array_type = TArray<T>;
    template<typename T>
    using optional_type = TOptional<T>;
    
    using string_traits = unreal_adapter::string_traits<FString>;
    template<typename T>
    using container_traits = unreal_adapter::container_traits<TArray<T>>;
    template<typename T>
    using optional_traits = unreal_adapter::optional_traits<TOptional<T>>;
    
    static constexpr bool needs_encoding_conversion = true;
};
```

### 5. Macro System for Each Environment

#### Standard C++ Macros

```cpp
// For standard C++ modules
#define SPACETIMEDB_BSATN_STRUCT(Type, ...) \
    template<> \
    struct spacetimedb::bsatn::bsatn_traits<Type> { \
        using Serializer = BsatnSerializer<StdPolicy>; \
        /* ... implementation ... */ \
    };
```

#### Unreal Engine Macros

```cpp
// For Unreal Engine
#define SPACETIMEDB_UNREAL_STRUCT(Type, ...) \
    template<> \
    struct spacetimedb::bsatn::bsatn_traits<Type> { \
        using Serializer = BsatnSerializer<UnrealPolicy>; \
        /* ... implementation ... */ \
    }; \
    /* Also generate Unreal reflection data */
```

### 6. Usage Examples

#### Standard C++ Module

```cpp
// my_module.cpp
#include <spacetimedb/bsatn_std.h>

struct Player {
    std::string name;
    std::vector<uint32_t> scores;
    std::optional<std::string> clan;
};

SPACETIMEDB_BSATN_STRUCT(Player, name, scores, clan)

void my_reducer(ReducerContext ctx, Player player) {
    // Uses std:: types naturally
    DB.players().insert(player);
}
```

#### Unreal Engine Client

```cpp
// MyGameClient.cpp
#include <spacetimedb/bsatn_unreal.h>

USTRUCT(BlueprintType)
struct FPlayer {
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(BlueprintReadWrite)
    TArray<uint32> Scores;
    
    UPROPERTY(BlueprintReadWrite)
    TOptional<FString> Clan;
};

// Macro that generates Unreal-compatible serialization
SPACETIMEDB_UNREAL_STRUCT(FPlayer, Name, Scores, Clan)

void UMyGameClient::HandlePlayerData(const TArray<uint8>& Data) {
    BsatnSerializer<UnrealPolicy> Serializer;
    FPlayer Player = Serializer.Deserialize<FPlayer>(Data);
    
    // Use in Unreal context
    UE_LOG(LogGame, Display, TEXT("Player: %s"), *Player.Name);
}
```

### 7. Type Mapping System

For seamless conversion between module and client types:

```cpp
// Type mapping traits
template<typename StdType, typename UnrealType>
struct TypeMapper {
    static UnrealType ToUnreal(const StdType& std_val);
    static StdType ToStd(const UnrealType& unreal_val);
};

// Specialization for string
template<>
struct TypeMapper<std::string, FString> {
    static FString ToUnreal(const std::string& str) {
        return FString(UTF8_TO_TCHAR(str.c_str()));
    }
    static std::string ToStd(const FString& str) {
        return TCHAR_TO_UTF8(*str);
    }
};

// Specialization for arrays
template<typename T>
struct TypeMapper<std::vector<T>, TArray<T>> {
    static TArray<T> ToUnreal(const std::vector<T>& vec) {
        TArray<T> arr;
        arr.Reserve(vec.size());
        for (const auto& item : vec) {
            arr.Add(TypeMapper<T, T>::ToUnreal(item));
        }
        return arr;
    }
    static std::vector<T> ToStd(const TArray<T>& arr) {
        std::vector<T> vec;
        vec.reserve(arr.Num());
        for (const auto& item : arr) {
            vec.push_back(TypeMapper<T, T>::ToStd(item));
        }
        return vec;
    }
};

// Automatic struct mapping using reflection
template<typename StdStruct, typename UnrealStruct>
class StructMapper {
    // Could use code generation or template metaprogramming
    // to automatically map fields between structs
};
```

### 8. Alternative Approach: Code Generation

For maximum performance and type safety, consider a code generation approach:

#### Schema Definition

```yaml
# player.schema.yaml
structs:
  Player:
    fields:
      - name: name
        type: string
      - name: scores
        type: array<uint32>
      - name: clan
        type: optional<string>
        
reducers:
  add_player:
    params:
      - player: Player
```

#### Generated Output

The code generator would produce:

**Player_Std.h** (for modules):
```cpp
struct Player {
    std::string name;
    std::vector<uint32_t> scores;
    std::optional<std::string> clan;
};
SPACETIMEDB_BSATN_STRUCT(Player, name, scores, clan)
```

**Player_Unreal.h** (for Unreal clients):
```cpp
USTRUCT(BlueprintType)
struct FPlayer {
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(BlueprintReadWrite)
    TArray<uint32> Scores;
    
    UPROPERTY(BlueprintReadWrite)
    TOptional<FString> Clan;
};
SPACETIMEDB_UNREAL_STRUCT(FPlayer, Name, Scores, Clan)

// Conversion functions
FPlayer ToUnreal(const Player& player);
Player ToStd(const FPlayer& player);
```

## Benefits of This Architecture

1. **Zero Overhead** - Template specialization is resolved at compile time
2. **Type Safety** - Each environment uses its native types with full type checking
3. **Code Reuse** - Core BSATN logic is shared between both implementations
4. **Maintainability** - Clear separation of concerns between core and adapters
5. **Extensibility** - Easy to add support for other type systems (Qt, custom engines)
6. **Performance** - No runtime type conversions unless explicitly needed
7. **Developer Experience** - Natural use of platform-specific types

## Implementation Considerations

### Memory Management

- Standard C++ uses standard allocators
- Unreal Engine has its own memory management (FMemory, UObject system)
- Adapters should respect platform-specific allocation strategies

### Thread Safety

- Standard C++ modules might use std::mutex
- Unreal Engine has FCriticalSection and other threading primitives
- Core BSATN should be thread-agnostic

### Error Handling

- Standard C++ can use exceptions
- Unreal Engine typically avoids exceptions
- Use a policy-based approach for error handling

### Build System Integration

- Standard C++ modules use CMake
- Unreal Engine uses UnrealBuildTool
- Provide appropriate build configurations for each

## Recommended Implementation Path

1. **Phase 1**: Extract type-agnostic core from current BSATN
2. **Phase 2**: Implement standard C++ adapter (minimal changes)
3. **Phase 3**: Implement Unreal Engine adapter
4. **Phase 4**: Create example projects for both environments
5. **Phase 5**: Optimize and add code generation support

## Conclusion

This architecture provides a clean separation between the core BSATN functionality and platform-specific type systems. It allows both standard C++ modules and Unreal Engine clients to use their native types while sharing the same serialization protocol, ensuring compatibility and optimal performance in both environments.