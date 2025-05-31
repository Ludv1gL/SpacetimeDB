#include "spacetime_module_abi.h"
#include "spacetime_module_def.h" // For SpacetimeDB::Internal::get_serialized_module_definition_bytes
#include <vector>
#include <cstddef> // For std::byte

// EMSCRIPTEN_KEEPALIVE can be used here if <emscripten.h> is available and targeted.
// For example:
// #ifdef __EMSCRIPTEN__
// #include <emscripten.h>
// #define SPACETIMEDB_KEEPALIVE EMSCRIPTEN_KEEPALIVE
// #else
// #define SPACETIMEDB_KEEPALIVE
// #endif
// And then apply SPACETIMEDB_KEEPALIVE to the functions.
// For this file, we'll assume export is handled by linker flags or that the user
// will add EMSCRIPTEN_KEEPALIVE if they prefer that method.

namespace {
// Static storage for the serialized module definition.
// This ensures the data pointer remains valid for the lifetime of the Wasm module.
std::vector<std::byte> g_serialized_module_def_buffer;
bool g_buffer_initialized = false;

void initialize_buffer_if_needed() {
    if (!g_buffer_initialized) {
        g_serialized_module_def_buffer = SpacetimeDB::Internal::get_serialized_module_definition_bytes();
        g_buffer_initialized = true;
    }
}

} // anonymous namespace

extern "C" {

const unsigned char* get_spacetimedb_module_def_data() {
    initialize_buffer_if_needed();
    return reinterpret_cast<const unsigned char*>(g_serialized_module_def_buffer.data());
}

int get_spacetimedb_module_def_size() {
    initialize_buffer_if_needed();
    return static_cast<int>(g_serialized_module_def_buffer.size());
}

} // extern "C"
