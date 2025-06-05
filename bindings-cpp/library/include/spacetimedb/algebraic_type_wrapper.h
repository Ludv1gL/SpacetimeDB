#pragma once

/**
 * Wrapper to provide compatibility for AlgebraicType usage in spacetimedb namespace
 * This allows us to use the internal/autogen AlgebraicType system
 */

#include "internal/autogen/AlgebraicType.g.h"

namespace SpacetimeDb {

// Note: AlgebraicType types are already defined in bsatn/algebraic_type.h
// This file just includes the internal/autogen types that might be needed

// Import additional types that might not be in bsatn
using AlgebraicTypeRef = SpacetimeDb::Internal::AlgebraicTypeRef;

} // namespace SpacetimeDb