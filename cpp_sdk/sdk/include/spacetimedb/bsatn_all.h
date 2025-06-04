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
using spacetimedb::bsatn::Reader;
using spacetimedb::bsatn::Writer;
using spacetimedb::bsatn::Option;
using spacetimedb::bsatn::SumType;
using spacetimedb::bsatn::AlgebraicType;
using spacetimedb::bsatn::ProductType;
using spacetimedb::bsatn::serialize;
using spacetimedb::bsatn::deserialize;

// Backward compatibility aliases
using spacetimedb::bsatn::Sum;  // Legacy name for SumType