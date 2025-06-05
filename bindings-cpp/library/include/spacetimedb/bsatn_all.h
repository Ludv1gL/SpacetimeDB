/**
 * @file bsatn_all.h
 * @brief Single unified include point for all BSATN functionality
 * 
 * This header provides the complete BSATN serialization system with
 * consistent naming aligned to Rust/C# conventions.
 * 
 * @deprecated Use spacetimedb/bsatn/bsatn.h instead for new code
 */

#pragma once

// Include the main BSATN header
#include "spacetimedb/bsatn/bsatn.h"

// Make commonly used types available in global scope
using SpacetimeDb::bsatn::Reader;
using SpacetimeDb::bsatn::Writer;
using SpacetimeDb::bsatn::Option;
using SpacetimeDb::bsatn::SumType;
using SpacetimeDb::bsatn::AlgebraicType;
using SpacetimeDb::bsatn::ProductType;
using SpacetimeDb::bsatn::serialize;
using SpacetimeDb::bsatn::deserialize;

// Backward compatibility aliases
using SpacetimeDb::bsatn::Sum;  // Legacy name for SumType