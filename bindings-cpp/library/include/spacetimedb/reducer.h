#ifndef SPACETIMEDB_LIBRARY_REDUCER_H
#define SPACETIMEDB_LIBRARY_REDUCER_H

#include "spacetimedb/types.h"
#include <spacetimedb/reducer_context.h>
#include <spacetimedb/database.h>
#include <spacetimedb/bsatn_all.h>
#include <spacetimedb/abi/spacetimedb_abi.h>

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <memory>
#include <cstring> // For std::strlen (used in macro for error messages)

namespace SpacetimeDb {
namespace library {

// Global database instance for reducers
// Needs to be initialized by the host calling _spacetimedb_library_init
// This is defined as static in the header for simplicity, meaning each TU including this
// might get its own instance if not careful, or it might work due to inline nature
// and single WASM module. A .cpp definition would be safer for a global.
// For now, per prior regeneration, it's here.
static std::unique_ptr<Database> global_db_instance_ptr_for_reducers;

inline void initialize_reducer_database_instance() {
    if (!global_db_instance_ptr_for_reducers) {
        global_db_instance_ptr_for_reducers = std::make_unique<Database>();
    }
}

// Exported init function for host to call
extern "C" __attribute__((export_name("_spacetimedb_library_init")))
void _spacetimedb_library_init() {
    initialize_reducer_database_instance();
}


// Template helper to deserialize a single argument
template<typename T>
T deserialize_reducer_arg(SpacetimeDb::bsatn::Reader& reader) {
    T arg; // Requires T to be default-constructible for non-primitive cases
    if constexpr (std::is_same_v<T, bool>) return reader.read_bool();
    else if constexpr (std::is_same_v<T, uint8_t>) return reader.read_u8();
    else if constexpr (std::is_same_v<T, uint16_t>) return reader.read_u16_le();
    else if constexpr (std::is_same_v<T, uint32_t>) return reader.read_u32_le();
    else if constexpr (std::is_same_v<T, uint64_t>) return reader.read_u64_le();
    else if constexpr (std::is_same_v<T, int8_t>) return reader.read_i8();
    else if constexpr (std::is_same_v<T, int16_t>) return reader.read_i16_le();
    else if constexpr (std::is_same_v<T, int32_t>) return reader.read_i32_le();
    else if constexpr (std::is_same_v<T, int64_t>) return reader.read_i64_le();
    else if constexpr (std::is_same_v<T, float>) return reader.read_f32_le();
    else if constexpr (std::is_same_v<T, double>) return reader.read_f64_le();
    else if constexpr (std::is_same_v<T, std::string>) return reader.read_string();
    else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
        auto bytes_vec = reader.read_bytes();
        return std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(bytes_vec.data()),
                                   reinterpret_cast<const uint8_t*>(bytes_vec.data()) + bytes_vec.size());
    }
    else if constexpr (std::is_same_v<T, SpacetimeDb::Identity>) {
        arg.bsatn_deserialize(reader); return arg;
    } else if constexpr (std::is_same_v<T, SpacetimeDb::Timestamp>) {
        arg.bsatn_deserialize(reader); return arg;
    }
    else if constexpr (requires(T& t, SpacetimeDb::bsatn::Reader& r) { t.bsatn_deserialize(r); }) {
        arg.bsatn_deserialize(reader);
        return arg;
    } else {
        static_assert(std::is_void_v<T>, "Unsupported reducer argument type for BSATN deserialization. Must be primitive, string, bytes, Identity, Timestamp, or implement BsatnSerializable/bsatn_deserialize.");
        return T{};
    }
}

// Helper to deserialize all arguments into a tuple
// Ensure types are cleaned (remove ref and cv-qualifiers) before tuple_element_t
template<typename... Args, std::size_t... Is>
std::tuple<Args...> deserialize_all_args_impl(SpacetimeDb::bsatn::Reader& reader, std::index_sequence<Is...>) {
    return std::make_tuple(deserialize_reducer_arg<std::remove_cv_t<std::remove_reference_t<std::tuple_element_t<Is, std::tuple<Args...>>>>>(reader)...);
}

template<typename... Args>
std::tuple<Args...> deserialize_all_args(SpacetimeDb::bsatn::Reader& reader) {
    return deserialize_all_args_impl<Args...>(reader, std::index_sequence_for<Args...>{});
}

// Macro to define and register a reducer
#define SPACETIMEDB_REDUCER(REDUCER_FUNC_NAME, ...) \
    extern void REDUCER_FUNC_NAME(SpacetimeDb::ReducerContext& ctx, ##__VA_ARGS__); \
    extern "C" __attribute__((export_name(#REDUCER_FUNC_NAME))) \
    uint16_t _spacetimedb_reducer_wrapper_##REDUCER_FUNC_NAME(const uint8_t* args_data, size_t args_len) { \
        if (!SpacetimeDb::global_db_instance_ptr_for_reducers) { \
            const char* err_msg = "Critical Error: Module Library Database not initialized before calling reducer " #REDUCER_FUNC_NAME ". Host must call _spacetimedb_library_init."; \
            _console_log(0 /*FATAL like level*/, nullptr, 0, nullptr, 0, 0, reinterpret_cast<const uint8_t*>(err_msg), std::strlen(err_msg)); \
            return 100; /* Distinct error code for uninitialized Module Library */ \
        } \
        try { \
            SpacetimeDb::bsatn::Reader reader(reinterpret_cast<const uint8_t*>(args_data), args_len); \
            SpacetimeDb::Identity sender; \
            sender.bsatn_deserialize(reader); \
            SpacetimeDb::Timestamp timestamp; \
            timestamp.bsatn_deserialize(reader); \
            SpacetimeDb::ReducerContext ctx(sender, timestamp, *SpacetimeDb::global_db_instance_ptr_for_reducers); \
            using UserArgsTuple = std::tuple<__VA_ARGS__>; \
            if constexpr (std::tuple_size_v<UserArgsTuple> > 0) { \
                auto deserialized_args_tuple = SpacetimeDb::deserialize_all_args<__VA_ARGS__>(reader); \
                std::apply([&](auto&&... args) { \
                    REDUCER_FUNC_NAME(ctx, std::forward<decltype(args)>(args)...); \
                }, std::move(deserialized_args_tuple)); \
            } else { \
                 REDUCER_FUNC_NAME(ctx); \
            } \
            return 0; /* Success */ \
        } catch (const std::exception& e) { \
            std::string error_message = "Reducer '" #REDUCER_FUNC_NAME "' C++ exception: "; \
            error_message += e.what(); \
            if (error_message.length() > 250) { error_message.resize(250); error_message += "..."; } \
            _console_log(1 /*ERROR level in ABI, corresponds to LOG_LEVEL_ERROR=3 in example kv_store.h */, nullptr, 0, nullptr, 0, 0, reinterpret_cast<const uint8_t*>(error_message.c_str()), error_message.length()); \
            return 1; /* General error code for C++ exception */ \
        } catch (...) { \
            std::string error_message = "Reducer '" #REDUCER_FUNC_NAME "' unknown C++ exception."; \
             _console_log(1 /*ERROR level*/, nullptr, 0, nullptr, 0, 0, reinterpret_cast<const uint8_t*>(error_message.c_str()), error_message.length()); \
            return 2; /* Specific error code for unknown C++ exception */ \
        } \
    }

#define SPACETIMEDB_REDUCER_NO_ARGS(REDUCER_FUNC_NAME) \
    extern void REDUCER_FUNC_NAME(SpacetimeDb::ReducerContext& ctx); \
    extern "C" __attribute__((export_name(#REDUCER_FUNC_NAME))) \
    uint16_t _spacetimedb_reducer_wrapper_##REDUCER_FUNC_NAME(const uint8_t* args_data, size_t args_len) { \
        if (!SpacetimeDb::global_db_instance_ptr_for_reducers) { \
             const char* err_msg = "Critical Error: Module Library Database not initialized before calling reducer " #REDUCER_FUNC_NAME ". Host must call _spacetimedb_library_init."; \
            _console_log(0 /*FATAL like level*/, nullptr, 0, nullptr, 0, 0, reinterpret_cast<const uint8_t*>(err_msg), std::strlen(err_msg)); \
            return 100; \
        } \
        try { \
            SpacetimeDb::bsatn::bsatn_reader reader(args_data, args_len); \
            SpacetimeDb::sdk::Identity sender; \
            sender.bsatn_deserialize(reader); \
            SpacetimeDb::sdk::Timestamp timestamp; \
            timestamp.bsatn_deserialize(reader); \
            SpacetimeDb::ReducerContext ctx(sender, timestamp, *SpacetimeDb::global_db_instance_ptr_for_reducers); \
            REDUCER_FUNC_NAME(ctx); \
            return 0; /* Success */ \
        } catch (const std::exception& e) { \
            std::string error_message = "Reducer '" #REDUCER_FUNC_NAME "' C++ exception: "; \
            error_message += e.what(); \
            if (error_message.length() > 250) { error_message.resize(250); error_message += "..."; } \
            _console_log(1 /*ERROR level*/, nullptr, 0, nullptr, 0, 0, reinterpret_cast<const uint8_t*>(error_message.c_str()), error_message.length()); \
            return 1; \
        } catch (...) { \
            std::string error_message = "Reducer '" #REDUCER_FUNC_NAME "' unknown C++ exception."; \
            _console_log(1 /*ERROR level*/, nullptr, 0, nullptr, 0, 0, reinterpret_cast<const uint8_t*>(error_message.c_str()), error_message.length()); \
            return 2; \
        } \
    }

} // namespace library
} // namespace SpacetimeDb

#endif // SPACETIMEDB_LIBRARY_REDUCER_H
