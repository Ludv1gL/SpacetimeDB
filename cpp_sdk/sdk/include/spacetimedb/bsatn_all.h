/**
 * @file bsatn_all.h
 * @brief Single include point for all BSATN functionality
 * 
 * This header includes all necessary BSATN headers in the correct order
 * and provides a consistent interface for BSATN serialization.
 */

#pragma once

// Core BSATN headers
#include "spacetimedb/bsatn/reader.h"
#include "spacetimedb/bsatn/writer.h"
#include "spacetimedb/bsatn/traits.h"
#include "spacetimedb/bsatn/algebraic_type.h"
#include "spacetimedb/bsatn/sum_type.h"

// Convenience namespace alias
namespace spacetimedb {
    namespace bsatn = ::SpacetimeDb::bsatn;
}

// Make commonly used types available
using SpacetimeDb::bsatn::Reader;
using SpacetimeDb::bsatn::Writer;
using SpacetimeDb::bsatn::Option;
using SpacetimeDb::bsatn::Sum;
using SpacetimeDb::bsatn::serialize;
using SpacetimeDb::bsatn::deserialize;
using SpacetimeDb::bsatn::bsatn_traits;
using SpacetimeDb::bsatn::AlgebraicType;


// The SPACETIMEDB_BSATN_STRUCT macro from traits.h
// is already available after including traits.h