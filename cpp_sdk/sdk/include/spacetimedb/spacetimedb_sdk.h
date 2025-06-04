#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <tuple>
#include <type_traits>
#include <memory>
#include <optional>
#include <unordered_map>
#include <cstring>
#include <array>
#include <chrono>

// FFI function declarations
extern "C" {
    // Core database operations
    uint16_t datastore_insert_bsatn(uint32_t table_id, uint8_t* row, size_t* row_len);
    uint16_t table_id_from_name(const uint8_t* name, size_t name_len, uint32_t* table_id);
    uint16_t datastore_table_scan_bsatn(uint32_t table_id, uint8_t* buffer, size_t* buffer_len);
    uint16_t datastore_delete_all_by_eq_bsatn(uint32_t table_id, uint8_t* value, size_t value_len, uint32_t* num_deleted);
    uint16_t table_row_count(uint32_t table_id, uint64_t* count);
    
    // Index operations
    uint16_t datastore_btree_scan_bsatn(uint32_t index_id, uint8_t* prefix, size_t prefix_len, 
                                        uint8_t* rstart, size_t rstart_len, uint8_t* rend, size_t rend_len,
                                        uint8_t* buffer, size_t* buffer_len);
    
    // Logging
    void console_log(uint8_t level, const uint8_t* target, size_t target_len,
                     const uint8_t* filename, size_t filename_len, uint32_t line_number,
                     const uint8_t* text, size_t text_len);
    
    // Random number generation
    void volatile_nonatomic_schedule_immediate(const uint8_t* name, size_t name_len, 
                                               const uint8_t* args, size_t args_len);
    uint64_t get_pseudorandom_u64();
    void get_pseudorandom_bytes(uint8_t* buffer, size_t len);
    
    // Timing
    uint64_t timestamp_now();
}

namespace spacetimedb {

// Forward declarations
template<typename T> class TableHandle;
class ModuleDatabase;
struct ReducerContext;
class LogStopwatch;

// Basic types
struct Address {
    std::array<uint8_t, 16> bytes{};
    
    bool operator==(const Address& other) const {
        return bytes == other.bytes;
    }
};

struct Identity {
    std::array<uint8_t, 32> bytes{};
    
    bool operator==(const Identity& other) const {
        return bytes == other.bytes;
    }
};

struct Timestamp {
    uint64_t microseconds_since_epoch;
    
    Timestamp(uint64_t micros = 0) : microseconds_since_epoch(micros) {}
    
    static Timestamp now() {
        return Timestamp(timestamp_now());
    }
    
    uint64_t as_milliseconds() const {
        return microseconds_since_epoch / 1000;
    }
};

// BSATN Writer
class Writer {
    std::vector<uint8_t>& buffer;
public:
    explicit Writer(std::vector<uint8_t>& buf) : buffer(buf) {}
    
    void write_u8(uint8_t v) { 
        buffer.push_back(v); 
    }
    
    void write_u16(uint16_t v) {
        buffer.push_back(v & 0xFF);
        buffer.push_back((v >> 8) & 0xFF);
    }
    
    void write_u32(uint32_t v) {
        for (int i = 0; i < 4; i++) {
            buffer.push_back((v >> (i * 8)) & 0xFF);
        }
    }
    
    void write_u64(uint64_t v) {
        for (int i = 0; i < 8; i++) {
            buffer.push_back((v >> (i * 8)) & 0xFF);
        }
    }
    
    void write_i8(int8_t v) { write_u8(static_cast<uint8_t>(v)); }
    void write_i16(int16_t v) { write_u16(static_cast<uint16_t>(v)); }
    void write_i32(int32_t v) { write_u32(static_cast<uint32_t>(v)); }
    void write_i64(int64_t v) { write_u64(static_cast<uint64_t>(v)); }
    
    void write_f32(float v) {
        union { float f; uint32_t u; } converter;
        converter.f = v;
        write_u32(converter.u);
    }
    
    void write_f64(double v) {
        union { double f; uint64_t u; } converter;
        converter.f = v;
        write_u64(converter.u);
    }
    
    void write_bool(bool v) { 
        write_u8(v ? 1 : 0); 
    }
    
    void write_string(const std::string& s) {
        write_u32(s.length());
        for (char c : s) {
            buffer.push_back(static_cast<uint8_t>(c));
        }
    }
    
    void write_bytes(const uint8_t* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            buffer.push_back(data[i]);
        }
    }
    
    template<typename T>
    void write_option(const std::optional<T>& opt);
    
    template<typename T>
    void write_vector(const std::vector<T>& vec);
};

// BSATN Reader
class Reader {
    const uint8_t* data;
    size_t pos;
    size_t len;
public:
    Reader(const uint8_t* d, size_t l) : data(d), pos(0), len(l) {}
    
    bool has_remaining() const { return pos < len; }
    size_t remaining() const { return len - pos; }
    
    uint8_t read_u8() {
        if (pos >= len) return 0;
        return data[pos++];
    }
    
    uint16_t read_u16() {
        if (pos + 2 > len) return 0;
        uint16_t result = 0;
        for (int i = 0; i < 2; i++) {
            result |= (uint16_t(data[pos++]) << (i * 8));
        }
        return result;
    }
    
    uint32_t read_u32() {
        if (pos + 4 > len) return 0;
        uint32_t result = 0;
        for (int i = 0; i < 4; i++) {
            result |= (uint32_t(data[pos++]) << (i * 8));
        }
        return result;
    }
    
    uint64_t read_u64() {
        if (pos + 8 > len) return 0;
        uint64_t result = 0;
        for (int i = 0; i < 8; i++) {
            result |= (uint64_t(data[pos++]) << (i * 8));
        }
        return result;
    }
    
    int8_t read_i8() { return static_cast<int8_t>(read_u8()); }
    int16_t read_i16() { return static_cast<int16_t>(read_u16()); }
    int32_t read_i32() { return static_cast<int32_t>(read_u32()); }
    int64_t read_i64() { return static_cast<int64_t>(read_u64()); }
    
    float read_f32() {
        union { float f; uint32_t u; } converter;
        converter.u = read_u32();
        return converter.f;
    }
    
    double read_f64() {
        union { double f; uint64_t u; } converter;
        converter.u = read_u64();
        return converter.f;
    }
    
    bool read_bool() { 
        return read_u8() != 0; 
    }
    
    std::string read_string() {
        uint32_t str_len = read_u32();
        if (pos + str_len > len) return "";
        std::string result(reinterpret_cast<const char*>(data + pos), str_len);
        pos += str_len;
        return result;
    }
    
    void read_bytes(uint8_t* out, size_t out_len) {
        size_t to_read = std::min(out_len, len - pos);
        std::memcpy(out, data + pos, to_read);
        pos += to_read;
    }
    
    template<typename T>
    std::optional<T> read_option();
    
    template<typename T>
    std::vector<T> read_vector();
};

// Serialization traits
template<typename T>
struct has_custom_serialize : std::false_type {};

// Default serialization functions
template<typename T>
void serialize(Writer& writer, const T& value) {
    if constexpr (has_custom_serialize<T>::value) {
        value.serialize(writer);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        writer.write_u8(value);
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        writer.write_u16(value);
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        writer.write_u32(value);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        writer.write_u64(value);
    } else if constexpr (std::is_same_v<T, int8_t>) {
        writer.write_i8(value);
    } else if constexpr (std::is_same_v<T, int16_t>) {
        writer.write_i16(value);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        writer.write_i32(value);
    } else if constexpr (std::is_same_v<T, int64_t>) {
        writer.write_i64(value);
    } else if constexpr (std::is_same_v<T, float>) {
        writer.write_f32(value);
    } else if constexpr (std::is_same_v<T, double>) {
        writer.write_f64(value);
    } else if constexpr (std::is_same_v<T, bool>) {
        writer.write_bool(value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        writer.write_string(value);
    } else if constexpr (std::is_same_v<T, Address>) {
        writer.write_bytes(value.bytes.data(), 16);
    } else if constexpr (std::is_same_v<T, Identity>) {
        writer.write_bytes(value.bytes.data(), 32);
    } else if constexpr (std::is_same_v<T, Timestamp>) {
        writer.write_u64(value.microseconds_since_epoch);
    } else {
        static_assert(has_custom_serialize<T>::value, "Type must have custom serialization");
    }
}

template<typename T>
void deserialize(Reader& reader, T& value) {
    if constexpr (has_custom_serialize<T>::value) {
        value.deserialize(reader);
    } else if constexpr (std::is_same_v<T, uint8_t>) {
        value = reader.read_u8();
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        value = reader.read_u16();
    } else if constexpr (std::is_same_v<T, uint32_t>) {
        value = reader.read_u32();
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        value = reader.read_u64();
    } else if constexpr (std::is_same_v<T, int8_t>) {
        value = reader.read_i8();
    } else if constexpr (std::is_same_v<T, int16_t>) {
        value = reader.read_i16();
    } else if constexpr (std::is_same_v<T, int32_t>) {
        value = reader.read_i32();
    } else if constexpr (std::is_same_v<T, int64_t>) {
        value = reader.read_i64();
    } else if constexpr (std::is_same_v<T, float>) {
        value = reader.read_f32();
    } else if constexpr (std::is_same_v<T, double>) {
        value = reader.read_f64();
    } else if constexpr (std::is_same_v<T, bool>) {
        value = reader.read_bool();
    } else if constexpr (std::is_same_v<T, std::string>) {
        value = reader.read_string();
    } else if constexpr (std::is_same_v<T, Address>) {
        reader.read_bytes(value.bytes.data(), 16);
    } else if constexpr (std::is_same_v<T, Identity>) {
        reader.read_bytes(value.bytes.data(), 32);
    } else if constexpr (std::is_same_v<T, Timestamp>) {
        value.microseconds_since_epoch = reader.read_u64();
    } else {
        static_assert(has_custom_serialize<T>::value, "Type must have custom deserialization");
    }
}

// Option serialization
template<typename T>
void Writer::write_option(const std::optional<T>& opt) {
    if (opt.has_value()) {
        write_u8(0); // Some
        serialize(*this, opt.value());
    } else {
        write_u8(1); // None
    }
}

template<typename T>
std::optional<T> Reader::read_option() {
    uint8_t tag = read_u8();
    if (tag == 0) { // Some
        T value;
        deserialize(*this, value);
        return value;
    }
    return std::nullopt; // None
}

// Vector serialization
template<typename T>
void Writer::write_vector(const std::vector<T>& vec) {
    write_u32(vec.size());
    for (const auto& elem : vec) {
        serialize(*this, elem);
    }
}

template<typename T>
std::vector<T> Reader::read_vector() {
    uint32_t size = read_u32();
    std::vector<T> result;
    result.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        T elem;
        deserialize(*this, elem);
        result.push_back(std::move(elem));
    }
    return result;
}

// Calculate size helper
template<typename T>
size_t calculate_size(const T& value) {
    std::vector<uint8_t> temp;
    Writer writer(temp);
    serialize(writer, value);
    return temp.size();
}

// Error result type
template<typename T>
class Result {
    bool success;
    T value;
    std::string error;
    
public:
    Result(T val) : success(true), value(std::move(val)) {}
    Result(const char* err) : success(false), error(err) {}
    Result(std::string err) : success(false), error(std::move(err)) {}
    
    bool is_ok() const { return success; }
    bool is_err() const { return !success; }
    
    const T& unwrap() const { 
        if (!success) throw std::runtime_error("Called unwrap on error Result");
        return value; 
    }
    T& unwrap() { 
        if (!success) throw std::runtime_error("Called unwrap on error Result");
        return value; 
    }
    
    const T& unwrap_or(const T& default_value) const {
        return success ? value : default_value;
    }
    
    const std::string& error_message() const { return error; }
};

// Table handle implementation
template<typename T>
class TableHandle {
    std::string table_name;
    mutable uint32_t table_id = 0;
    mutable bool id_resolved = false;
    
    Result<uint32_t> resolve_table_id() const {
        if (id_resolved) return table_id;
        
        uint16_t err = table_id_from_name(
            reinterpret_cast<const uint8_t*>(table_name.c_str()), 
            table_name.length(), 
            &table_id);
            
        if (err == 0) {
            id_resolved = true;
            return table_id;
        }
        
        return Result<uint32_t>("Failed to resolve table ID");
    }
    
public:
    explicit TableHandle(const std::string& name) : table_name(name) {}
    
    Result<T> insert(const T& row) {
        auto id_result = resolve_table_id();
        if (id_result.is_err()) {
            return Result<T>(id_result.error_message());
        }
        
        std::vector<uint8_t> buffer;
        Writer writer(buffer);
        serialize(writer, row);
        
        size_t len = buffer.size();
        uint16_t err = datastore_insert_bsatn(id_result.unwrap(), buffer.data(), &len);
        
        if (err == 0 && len > 0) {
            // Parse returned row (may have auto-inc fields)
            Reader reader(buffer.data(), len);
            T result;
            deserialize(reader, result);
            return result;
        }
        
        return row;
    }
    
    Result<T> try_insert(const T& row) {
        return insert(row);
    }
    
    Result<bool> delete_by_value(const T& value) {
        auto id_result = resolve_table_id();
        if (id_result.is_err()) {
            return Result<bool>(id_result.error_message());
        }
        
        std::vector<uint8_t> buffer;
        Writer writer(buffer);
        serialize(writer, value);
        
        uint32_t num_deleted = 0;
        uint16_t err = datastore_delete_all_by_eq_bsatn(
            id_result.unwrap(), buffer.data(), buffer.size(), &num_deleted);
        
        if (err != 0) {
            return Result<bool>("Delete operation failed");
        }
        
        return num_deleted > 0;
    }
    
    uint64_t count() {
        auto id_result = resolve_table_id();
        if (id_result.is_err()) return 0;
        
        uint64_t row_count = 0;
        table_row_count(id_result.unwrap(), &row_count);
        return row_count;
    }
    
    std::vector<T> scan() {
        auto id_result = resolve_table_id();
        if (id_result.is_err()) return {};
        
        std::vector<T> results;
        std::vector<uint8_t> buffer(65536); // 64KB buffer
        size_t buffer_len = buffer.size();
        
        uint16_t err = datastore_table_scan_bsatn(id_result.unwrap(), buffer.data(), &buffer_len);
        if (err != 0) return results;
        
        Reader reader(buffer.data(), buffer_len);
        while (reader.has_remaining()) {
            T row;
            deserialize(reader, row);
            results.push_back(std::move(row));
        }
        
        return results;
    }
    
    // Iterator support
    class iterator {
        // TODO: Implement proper iterator
    public:
        T operator*() const { return T{}; }
        iterator& operator++() { return *this; }
        bool operator!=(const iterator& other) const { return false; }
    };
    
    iterator begin() { return iterator{}; }
    iterator end() { return iterator{}; }
};

// Module database
class ModuleDatabase {
public:
    template<typename T>
    TableHandle<T> table(const std::string& name) {
        return TableHandle<T>(name);
    }
};

// Reducer context
struct ReducerContext {
    ModuleDatabase db;
    Identity sender;
    Timestamp timestamp;
    std::optional<Address> address;
    
    ReducerContext() : timestamp(Timestamp::now()) {}
};

// Logging utilities
enum class LogLevel : uint8_t {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Panic = 5
};

inline void log(LogLevel level, const std::string& msg, const char* file = __FILE__, uint32_t line = __LINE__) {
    console_log(static_cast<uint8_t>(level), nullptr, 0, 
                reinterpret_cast<const uint8_t*>(file), std::strlen(file), 
                line, 
                reinterpret_cast<const uint8_t*>(msg.c_str()), msg.length());
}

inline void log_trace(const std::string& msg, const char* file = __FILE__, uint32_t line = __LINE__) {
    log(LogLevel::Trace, msg, file, line);
}

inline void log_debug(const std::string& msg, const char* file = __FILE__, uint32_t line = __LINE__) {
    log(LogLevel::Debug, msg, file, line);
}

inline void log_info(const std::string& msg, const char* file = __FILE__, uint32_t line = __LINE__) {
    log(LogLevel::Info, msg, file, line);
}

inline void log_warn(const std::string& msg, const char* file = __FILE__, uint32_t line = __LINE__) {
    log(LogLevel::Warn, msg, file, line);
}

inline void log_error(const std::string& msg, const char* file = __FILE__, uint32_t line = __LINE__) {
    log(LogLevel::Error, msg, file, line);
}

inline void log_panic(const std::string& msg, const char* file = __FILE__, uint32_t line = __LINE__) {
    log(LogLevel::Panic, msg, file, line);
}

// LogStopwatch for performance measurement
class LogStopwatch {
    std::string message;
    Timestamp start_time;
    const char* file;
    uint32_t line;
    
public:
    LogStopwatch(const std::string& msg, const char* f = __FILE__, uint32_t l = __LINE__) 
        : message(msg), start_time(Timestamp::now()), file(f), line(l) {}
    
    ~LogStopwatch() {
        auto end_time = Timestamp::now();
        auto duration_micros = end_time.microseconds_since_epoch - start_time.microseconds_since_epoch;
        auto duration_ms = duration_micros / 1000.0;
        
        std::string log_msg = message + " took " + std::to_string(duration_ms) + "ms";
        log_info(log_msg, file, line);
    }
};

// Random number utilities
inline uint64_t random_u64() {
    return get_pseudorandom_u64();
}

inline void random_bytes(uint8_t* buffer, size_t len) {
    get_pseudorandom_bytes(buffer, len);
}

inline double random_f64() {
    return static_cast<double>(random_u64()) / static_cast<double>(UINT64_MAX);
}

// Variadic argument parsing
namespace detail {
    template<typename... Args>
    struct ArgumentParser {
        static std::tuple<Args...> parse(const uint8_t* buffer, size_t size) {
            Reader reader(buffer, size);
            return parse_impl<Args...>(reader, std::index_sequence_for<Args...>{});
        }
        
    private:
        template<typename... Ts, size_t... Is>
        static std::tuple<Ts...> parse_impl(Reader& reader, std::index_sequence<Is...>) {
            return std::make_tuple(deserialize_single<Ts>(reader)...);
        }
        
        template<typename T>
        static T deserialize_single(Reader& reader) {
            T value;
            deserialize(reader, value);
            return value;
        }
    };
}

// Reducer registry
class ReducerRegistry {
public:
    using Handler = std::function<void(const uint8_t*, size_t)>;
    
    struct ReducerInfo {
        std::string name;
        Handler handler;
        bool is_init = false;
        bool is_client_connected = false;
        bool is_client_disconnected = false;
    };
    
    void register_reducer(const std::string& name, Handler handler, 
                         bool is_init = false, 
                         bool is_client_connected = false,
                         bool is_client_disconnected = false) {
        ReducerInfo info{name, handler, is_init, is_client_connected, is_client_disconnected};
        reducers[name] = info;
    }
    
    bool dispatch(const std::string& name, const uint8_t* args, size_t args_len) {
        auto it = reducers.find(name);
        if (it != reducers.end()) {
            it->second.handler(args, args_len);
            return true;
        }
        return false;
    }
    
    const std::unordered_map<std::string, ReducerInfo>& get_reducers() const {
        return reducers;
    }
    
    static ReducerRegistry& instance() {
        static ReducerRegistry registry;
        return registry;
    }
    
private:
    std::unordered_map<std::string, ReducerInfo> reducers;
};

// Reducer registration helper
template<typename... Args>
void register_reducer(const std::string& name, std::function<void(ReducerContext&, Args...)> fn) {
    ReducerRegistry::instance().register_reducer(name, 
        [fn](const uint8_t* buffer, size_t size) {
            auto args = detail::ArgumentParser<Args...>::parse(buffer, size);
            ReducerContext ctx;
            std::apply([&ctx, &fn](Args... parsed_args) {
                fn(ctx, parsed_args...);
            }, args);
        });
}

// Built-in reducer registration
inline void register_init_reducer(std::function<void(ReducerContext&)> fn) {
    ReducerRegistry::instance().register_reducer("__init__", 
        [fn](const uint8_t*, size_t) {
            ReducerContext ctx;
            fn(ctx);
        }, true);
}

inline void register_client_connected_reducer(std::function<void(ReducerContext&)> fn) {
    ReducerRegistry::instance().register_reducer("__client_connected__", 
        [fn](const uint8_t*, size_t) {
            ReducerContext ctx;
            fn(ctx);
        }, false, true);
}

inline void register_client_disconnected_reducer(std::function<void(ReducerContext&)> fn) {
    ReducerRegistry::instance().register_reducer("__client_disconnected__", 
        [fn](const uint8_t*, size_t) {
            ReducerContext ctx;
            fn(ctx);
        }, false, false, true);
}

// Schedule reducer for immediate execution
inline void schedule_reducer(const std::string& name) {
    volatile_nonatomic_schedule_immediate(
        reinterpret_cast<const uint8_t*>(name.c_str()), name.length(), 
        nullptr, 0);
}

template<typename... Args>
void schedule_reducer(const std::string& name, Args... args) {
    std::vector<uint8_t> buffer;
    Writer writer(buffer);
    
    // Serialize all arguments
    (serialize(writer, args), ...);
    
    volatile_nonatomic_schedule_immediate(
        reinterpret_cast<const uint8_t*>(name.c_str()), name.length(), 
        buffer.data(), buffer.size());
}

} // namespace spacetimedb

// Macros for easy table and reducer declaration
#define SPACETIMEDB_TABLE(Type, name, is_public) \
    static_assert(true, "Table registered: " #name);

#define SPACETIMEDB_REDUCER(name, ctx, ...) \
    void name##_impl(spacetimedb::ReducerContext& ctx, ##__VA_ARGS__); \
    namespace { \
        struct name##_registrar { \
            name##_registrar() { \
                using namespace spacetimedb; \
                std::function<void(ReducerContext&, ##__VA_ARGS__)> fn = name##_impl; \
                register_reducer<__VA_ARGS__>(#name, fn); \
            } \
        }; \
        static name##_registrar name##_reg; \
    } \
    void name##_impl(spacetimedb::ReducerContext& ctx, ##__VA_ARGS__)

#define SPACETIMEDB_INIT(ctx) \
    void init_impl(spacetimedb::ReducerContext& ctx); \
    namespace { \
        struct init_registrar { \
            init_registrar() { \
                spacetimedb::register_init_reducer(init_impl); \
            } \
        }; \
        static init_registrar init_reg; \
    } \
    void init_impl(spacetimedb::ReducerContext& ctx)

#define SPACETIMEDB_CLIENT_CONNECTED(ctx) \
    void client_connected_impl(spacetimedb::ReducerContext& ctx); \
    namespace { \
        struct client_connected_registrar { \
            client_connected_registrar() { \
                spacetimedb::register_client_connected_reducer(client_connected_impl); \
            } \
        }; \
        static client_connected_registrar client_connected_reg; \
    } \
    void client_connected_impl(spacetimedb::ReducerContext& ctx)

#define SPACETIMEDB_CLIENT_DISCONNECTED(ctx) \
    void client_disconnected_impl(spacetimedb::ReducerContext& ctx); \
    namespace { \
        struct client_disconnected_registrar { \
            client_disconnected_registrar() { \
                spacetimedb::register_client_disconnected_reducer(client_disconnected_impl); \
            } \
        }; \
        static client_disconnected_registrar client_disconnected_reg; \
    } \
    void client_disconnected_impl(spacetimedb::ReducerContext& ctx)

// Field registration for structs
#define SPACETIMEDB_REGISTER_FIELDS(Type, ...) \
    namespace spacetimedb { \
        template<> struct has_custom_serialize<Type> : std::true_type {}; \
    } \
    __VA_ARGS__

#define SPACETIMEDB_FIELD(Type, field, field_type) \
    static_assert(std::is_same_v<decltype(std::declval<Type>().field), field_type>, \
                  "Field type mismatch for " #Type "::" #field)

// Module exports
extern "C" {
    void __describe_module__(uint8_t* buffer, size_t* len);
    void __call_reducer__(const char* name, size_t name_len, const uint8_t* args, size_t args_len);
}