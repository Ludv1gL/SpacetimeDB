#ifndef SPACETIMEDB_BSATN_COMPAT_H
#define SPACETIMEDB_BSATN_COMPAT_H

/**
 * Compatibility header to provide namespace aliases for the BSATN library.
 * This ensures consistent access patterns across the SDK.
 */

// Namespace aliases to handle inconsistencies
namespace spacetimedb {
    namespace bsatn = ::SpacetimeDb::bsatn;
}

#endif // SPACETIMEDB_BSATN_COMPAT_H