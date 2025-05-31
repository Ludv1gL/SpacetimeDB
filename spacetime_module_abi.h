#ifndef SPACETIME_MODULE_ABI_H
#define SPACETIME_MODULE_ABI_H

// This header defines the C-style ABI functions that a SpacetimeDB C++ Wasm module
// must export for the SpacetimeDB runtime to retrieve the schema definition.

// The primary way to ensure these are exported is by using compiler/linker flags,
// or compiler-specific attributes. For example, with Emscripten:
//
// 1. Using EMSCRIPTEN_KEEPALIVE (preferred for specific functions):
//    In the .cpp file implementing these functions:
//    #include <emscripten.h> // For EMSCRIPTEN_KEEPALIVE
//    extern "C" {
//        EMSCRIPTEN_KEEPALIVE const unsigned char* get_spacetimedb_module_def_data() { /* ... */ }
//        EMSCRIPTEN_KEEPALIVE int get_spacetimedb_module_def_size() { /* ... */ }
//    }
//
// 2. Using EXPORTED_FUNCTIONS linker flag:
//    When linking with emcc:
//    emcc ... -sEXPORTED_FUNCTIONS="['_get_spacetimedb_module_def_data', '_get_spacetimedb_module_def_size']"
//    (Note the leading underscores which are common in C ABI names).
//
// Users of this SDK should ensure their build system correctly exports these symbols.
// Including this header is not strictly necessary for the module to export them,
// but it provides the declarations if other C++ code within the same module needs to
// call them (though that's unlikely for these specific ABI functions).

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define SPACETIMEDB_WASM_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define SPACETIMEDB_WASM_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves a pointer to the static buffer containing the BSATN-serialized ModuleDef.
 *
 * The lifetime of the returned buffer is static and tied to the Wasm module's lifetime.
 * The data should not be modified.
 *
 * @return A pointer to the constant byte data of the serialized ModuleDef.
 */
SPACETIMEDB_WASM_EXPORT const unsigned char* get_spacetimedb_module_def_data();

/**
 * @brief Retrieves the size of the BSATN-serialized ModuleDef buffer.
 *
 * @return The size in bytes of the data returned by get_spacetimedb_module_def_data().
 */
SPACETIMEDB_WASM_EXPORT int get_spacetimedb_module_def_size();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SPACETIME_MODULE_ABI_H
