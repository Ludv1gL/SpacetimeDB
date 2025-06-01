#ifndef SPACETIMEDB_ABI_UTILS_H
#define SPACETIMEDB_ABI_UTILS_H

#include "spacetimedb_abi.h" // For the extern "C" ABI function declarations
#include "common_defs.h"     // For Status, BytesSink, BytesSource

#include <vector>
#include <string>
#include <stdexcept> // For std::runtime_error
#include <cstddef>   // For std::byte (C++17)

namespace SpacetimeDB {
namespace Abi {
namespace Utils {

// Helper to write data to a BytesSink.
// Throws std::runtime_error on host error.
inline void write_bytes_to_sink(::BytesSink sink_handle, const unsigned char* data, uint32_t len) {
    ::Status status = ::_bytes_sink_write(sink_handle, data, len);
    if (status.inner != 0) { // Assuming 0 is OK from common_defs.h convention
        throw std::runtime_error("Host failed to write to BytesSink, status: " + std::to_string(status.inner));
    }
}

// Convenience overload for std::vector<std::byte>.
inline void write_vector_to_sink(::BytesSink sink_handle, const std::vector<std::byte>& data) {
    write_bytes_to_sink(sink_handle, reinterpret_cast<const unsigned char*>(data.data()), static_cast<uint32_t>(data.size()));
}

// Convenience overload for std::string.
inline void write_string_to_sink(::BytesSink sink_handle, const std::string& str_data) {
    write_bytes_to_sink(sink_handle, reinterpret_cast<const unsigned char*>(str_data.data()), static_cast<uint32_t>(str_data.length()));
}


// Helper to read all remaining data from a BytesSource.
// Throws std::runtime_error on host error or if read count doesn't match expected.
inline std::vector<std::byte> read_all_from_source(::BytesSource source_handle) {
    uint32_t remaining_count = ::_bytes_source_get_remaining_count(source_handle);
    std::vector<std::byte> buffer(remaining_count);

    if (remaining_count == 0) {
        return buffer;
    }

    uint32_t bytes_read = ::_bytes_source_read(source_handle, reinterpret_cast<uint8_t*>(buffer.data()), remaining_count);

    if (bytes_read != remaining_count) {
        // This could indicate an issue or simply that the source was exhausted unexpectedly.
        // Depending on strictness, one might throw or just resize.
        // For now, assume it should read all that was reported by get_remaining_count.
        throw std::runtime_error("Failed to read all expected data from BytesSource. Expected: " +
                                 std::to_string(remaining_count) + ", Got: " + std::to_string(bytes_read));
    }
    // buffer.resize(bytes_read); // If bytes_read could be less and that's acceptable.
    return buffer;
}

// RAII wrappers for BytesSink and BytesSource to ensure _done is called.
class ManagedBytesSink {
public:
    ManagedBytesSink() : handle(::_bytes_sink_create()), valid(true) {}
    ~ManagedBytesSink() {
        if (valid) {
            ::_bytes_sink_done(handle);
        }
    }
    // Disable copy
    ManagedBytesSink(const ManagedBytesSink&) = delete;
    ManagedBytesSink& operator=(const ManagedBytesSink&) = delete;
    // Allow move
    ManagedBytesSink(ManagedBytesSink&& other) noexcept : handle(other.handle), valid(other.valid) {
        other.valid = false;
    }
    ManagedBytesSink& operator=(ManagedBytesSink&& other) noexcept {
        if (this != &other) {
            if (valid) {
                ::_bytes_sink_done(handle);
            }
            handle = other.handle;
            valid = other.valid;
            other.valid = false;
        }
        return *this;
    }

    ::BytesSink get_handle() const { return handle; }
    bool is_valid() const { return valid; }

private:
    ::BytesSink handle;
    bool valid; // To prevent double_done on move
};

class ManagedBytesSource {
public:
    // Create from existing bytes
    ManagedBytesSource(const unsigned char* data_ptr, uint32_t data_len)
        : handle(::_bytes_source_create_from_bytes(data_ptr, data_len)), valid(true) {}

    // Create from sink bytes (consumes sink)
    explicit ManagedBytesSource(ManagedBytesSink& sink)
        : handle(::_bytes_source_create_from_sink_bytes(sink.get_handle())), valid(true) {
        // Note: The ownership/lifetime of the sink's bytes might be relevant here.
        // Assuming _bytes_source_create_from_sink_bytes makes a copy or takes ownership appropriately.
    }
     explicit ManagedBytesSource(BytesSink raw_sink_handle)
        : handle(::_bytes_source_create_from_sink_bytes(raw_sink_handle)), valid(true) {}


    ~ManagedBytesSource() {
        if (valid) {
            ::_bytes_source_done(handle);
        }
    }
    // Disable copy
    ManagedBytesSource(const ManagedBytesSource&) = delete;
    ManagedBytesSource& operator=(const ManagedBytesSource&) = delete;
    // Allow move
    ManagedBytesSource(ManagedBytesSource&& other) noexcept : handle(other.handle), valid(other.valid) {
        other.valid = false;
    }
    ManagedBytesSource& operator=(ManagedBytesSource&& other) noexcept {
        if (this != &other) {
            if (valid) {
                ::_bytes_source_done(handle);
            }
            handle = other.handle;
            valid = other.valid;
            other.valid = false;
        }
        return *this;
    }

    ::BytesSource get_handle() const { return handle; }
    bool is_valid() const { return valid; }

private:
    ::BytesSource handle;
    bool valid;
};


} // namespace Utils
} // namespace Abi
} // namespace SpacetimeDB


#endif // SPACETIMEDB_ABI_UTILS_H
