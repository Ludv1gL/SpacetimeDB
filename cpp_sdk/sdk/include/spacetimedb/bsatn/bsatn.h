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
#include "bsatn_compat.h"        // Backward compatibility layer

// The spacetimedb::bsatn namespace contains the unified BSATN system with consistent
// naming conventions aligned to Rust and C# implementations:
//
// Core Types (matching Rust/C#):
//   - AlgebraicType, ProductType, SumType (renamed from Sum)
//   - ProductTypeElement, SumTypeVariant
//
// Core Operations (matching Rust):
//   - serialize(writer, value)
//   - deserialize<T>(reader)
//
// Error Types (matching Rust):
//   - EncodeError, DecodeError (future)
//
// Usage:
//   spacetimedb::bsatn::serialize(writer, obj)
//   auto obj = spacetimedb::bsatn::deserialize<MyType>(reader)
//
// Legacy namespace SpacetimeDb::bsatn is provided for backward compatibility.

#endif // SPACETIMEDB_BSATN_MAIN_H
