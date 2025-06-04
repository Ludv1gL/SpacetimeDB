#ifndef SPACETIMEDB_BSATN_MAIN_H // Changed guard to avoid conflict if old bsatn.h was somehow included
#define SPACETIMEDB_BSATN_MAIN_H

/**
 * @file bsatn.h
 * @brief Main include file for the BSATN (Binary SpacetimeDB Abstract Type Notation) library components.
 * @details This file includes the core components for BSATN serialization and deserialization:
 *          - bsatn::Reader
 *          - bsatn::Writer
 *          - Placeholder types for 128-bit integers.
 *          - Generic bsatn::serialize and bsatn::deserialize<T> free function templates and their overloads/specializations.
 *          - Algebraic type system for type metadata
 *          - Serialization traits and interfaces
 *          - Type registry for managing types
 *          - Sum type support for discriminated unions
 */

#include "reader.h"              // Defines Reader for deserialization
#include "writer.h"              // Defines Writer for serialization
#include "uint128_placeholder.h" // Defines 128-bit integer placeholders
#include "algebraic_type.h"      // AlgebraicType system (matches Rust/C#)
#include "traits.h"              // Serialization traits and interfaces
#include "type_registry.h"       // Type registry for metadata
#include "sum_type.h"            // SumType and Option (renamed from Sum)
#include "visitor.h"             // Visitor pattern for deserialization
#include "size_calculator.h"     // Size calculation without serialization
#include "special_types.h"       // Special type handling (Identity, Timestamp, etc.)
#include "bsatn_compat.h"        // Backward compatibility layer

// BSATN (Binary SpacetimeDB Algebraic Type Notation) C++ Implementation
//
// This provides a complete serialization system compatible with Rust and C#:
//
// 🔹 Core Types:
//   - AlgebraicType: Type metadata system
//   - ProductType: Structs/tuples (renamed elements for consistency)
//   - SumType<T...>: Discriminated unions (renamed from Sum)
//   - Option<T>: Optional values
//
// 🔹 Usage:
//   serialize(writer, value);           // Serialize any supported type
//   auto obj = deserialize<T>(reader);  // Deserialize to specific type
//   SPACETIMEDB_BSATN_STRUCT(MyType, field1, field2);  // Enable struct serialization
//
// 🔹 Features:
//   ✅ All primitive types (bool, integers, floats, string)
//   ✅ Containers (vector, optional)
//   ✅ User-defined structs via macro
//   ✅ Sum types and discriminated unions
//   ✅ Special SpacetimeDB types (Identity, Timestamp, etc.)
//   ✅ Type registry for metadata
//   ✅ Cross-language compatibility
//
// Note: Legacy SpacetimeDb::bsatn namespace is available for backward compatibility.

#endif // SPACETIMEDB_BSATN_MAIN_H
