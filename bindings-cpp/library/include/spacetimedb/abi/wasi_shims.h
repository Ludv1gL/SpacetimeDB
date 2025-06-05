// WASI shims for SpacetimeDB C++ modules
// This header indicates that WASI shims are available when building modules
// The actual implementations are in wasi_shims.cpp

#ifndef SPACETIMEDB_WASI_SHIMS_H
#define SPACETIMEDB_WASI_SHIMS_H

// This header indicates that WASI shims are provided by the SpacetimeDB C++ Module Library
// When this is defined, modules can safely use the C++ standard library
#define SPACETIMEDB_HAS_WASI_SHIMS 1

// Modules that need C++ standard library support should ensure wasi_shims.cpp is linked
// This is typically done automatically when linking the SpacetimeDB module library

#endif // SPACETIMEDB_WASI_SHIMS_H