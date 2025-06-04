# C++ Module Library Credential Management

This document describes the credential management functionality in the SpacetimeDB C++ Module Library.

## Overview

SpacetimeDB uses a JWT-based authentication system with 256-bit identities. The C++ Module Library provides utilities for:

- Working with SpacetimeDB identities
- Managing authentication tokens
- Implementing role-based access control
- Tracking permissions and credentials

## Core Components

### Identity Class

The `Identity` class represents a 256-bit identifier for actors in SpacetimeDB:

```cpp
#include <spacetimedb/credentials.h>

// Create identity from byte array
spacetimedb::Identity::ByteArray bytes = { /* 32 bytes */ };
auto identity = spacetimedb::Identity::from_byte_array(bytes);

// Create from hex string
auto identity_opt = spacetimedb::Identity::from_hex("0xc200...");
if (identity_opt) {
    auto identity = *identity_opt;
}

// Convert to hex string
std::string hex = identity.to_hex();  // "0xc200..."
std::string abbreviated = identity.to_abbreviated_hex();  // "0xc200..." (first 16 chars)

// Special identities
auto zero = spacetimedb::Identity::ZERO;
auto one = spacetimedb::Identity::ONE;
```

### Authentication Tokens

The `AuthToken` class represents JWT tokens used for authentication:

```cpp
// Create token from JWT string
spacetimedb::AuthToken token("eyJ...");

// Check if expired
if (token.is_expired()) {
    // Handle expired token
}

// Parse claims (without signature verification)
auto claims_opt = token.parse_claims();
if (claims_opt) {
    auto& claims = *claims_opt;
    // Access claims.identity, claims.subject, etc.
}
```

### Credentials Utility Class

The `Credentials` class provides static utility methods:

```cpp
// Create identity from issuer and subject (same algorithm as Rust)
auto identity = spacetimedb::Credentials::create_identity("issuer", "subject");

// Extract identity from token
auto identity_opt = spacetimedb::Credentials::extract_identity(token);

// Validate token format
bool valid = spacetimedb::Credentials::validate_token_format(token);
```

### Module Identity

Within a reducer, you can get the current module's identity:

```cpp
SPACETIMEDB_REDUCER(my_reducer, spacetimedb::ReducerContext ctx) {
    // Get module identity
    auto module_id = spacetimedb::get_module_identity();
    
    // Compare with sender
    if (ctx.sender == module_id) {
        // Called by the module itself (e.g., scheduled reducer)
    }
}
```

## Implementing Credential Management

### Basic Credential Table

```cpp
struct UserCredential {
    spacetimedb::Identity identity;
    std::string username;
    std::string role;
    uint64_t created_at;
    std::optional<uint64_t> revoked_at;
};

SPACETIMEDB_REGISTER_FIELDS(UserCredential,
    SPACETIMEDB_FIELD(UserCredential, identity, spacetimedb::Identity);
    SPACETIMEDB_FIELD(UserCredential, username, std::string);
    SPACETIMEDB_FIELD(UserCredential, role, std::string);
    SPACETIMEDB_FIELD(UserCredential, created_at, uint64_t);
    SPACETIMEDB_FIELD(UserCredential, revoked_at, std::optional<uint64_t>);
)

SPACETIMEDB_TABLE(UserCredential, user_credentials, true)
```

### Role-Based Access Control

```cpp
bool has_role(spacetimedb::ReducerContext& ctx, 
              const spacetimedb::Identity& identity, 
              const std::string& required_role) {
    auto credentials = ctx.db.table<UserCredential>("user_credentials");
    
    for (const auto& cred : credentials.iter()) {
        if (cred.identity == identity && !cred.revoked_at.has_value()) {
            return cred.role == required_role || cred.role == "admin";
        }
    }
    
    return false;
}

SPACETIMEDB_REDUCER(admin_only_action, spacetimedb::ReducerContext ctx) {
    if (!has_role(ctx, ctx.sender, "admin")) {
        throw std::runtime_error("Admin access required");
    }
    
    // Perform admin action
}
```

### Creating Credentials

```cpp
SPACETIMEDB_REDUCER(create_user, spacetimedb::ReducerContext ctx, 
                   std::string username, std::string role) {
    // Create identity from username
    auto identity = spacetimedb::Credentials::create_identity(
        "my-app",  // issuer
        username   // subject
    );
    
    UserCredential cred{
        identity,
        username,
        role,
        ctx.timestamp.microseconds(),
        std::nullopt
    };
    
    ctx.db.table<UserCredential>("user_credentials").insert(cred);
}
```

### Revoking Credentials

```cpp
SPACETIMEDB_REDUCER(revoke_user, spacetimedb::ReducerContext ctx, 
                   std::string username) {
    auto credentials = ctx.db.table<UserCredential>("user_credentials");
    
    for (auto& cred : credentials.iter()) {
        if (cred.username == username && !cred.revoked_at.has_value()) {
            cred.revoked_at = ctx.timestamp.microseconds();
            credentials.update(cred);
            break;
        }
    }
}
```

## Security Best Practices

1. **Always validate permissions** in reducers that perform sensitive operations
2. **Log permission checks** for audit trails
3. **Use time-based expiration** for credentials when appropriate
4. **Implement least privilege** - give users only the permissions they need
5. **Revoke rather than delete** credentials to maintain audit history

## Identity Format

SpacetimeDB identities are 256-bit values with specific formatting:

- **Memory representation**: 32-byte little-endian array
- **JSON representation**: Hexadecimal string "0x[64 hex chars]"
- **BSATN representation**: 32-byte little-endian array

Identities created from claims (issuer/subject) have a special format:
- First 2 bytes: `0xc2 0x00` (when displayed as hex)
- Next 4 bytes: Checksum
- Remaining 26 bytes: Hash of issuer|subject

## Limitations

### Current Implementation

- JWT parsing is simplified (production code should use a proper JWT library)
- Blake3 hashing is simulated with SHA256 (use actual Blake3 in production)
- Token generation is not implemented (requires server-side signing keys)
- Signature verification is not performed (client-side limitation)

### Future Enhancements

- Integration with OpenID Connect providers
- Automated token refresh
- Session management
- Multi-factor authentication support

## Example Usage

See `credential_example.cpp` for a complete example implementing:
- User credential management
- Role-based access control
- Permission logging
- Admin-only operations
- Credential revocation

## Migration from Other Systems

When migrating from traditional authentication systems:

1. Map existing user IDs to SpacetimeDB identities
2. Create credential records for existing users
3. Implement role mapping from your existing system
4. Add permission checks to sensitive reducers
5. Set up audit logging for compliance

## Testing

For testing credential management:

```cpp
// Create test identities
auto alice = spacetimedb::Credentials::create_identity("test", "alice");
auto bob = spacetimedb::Credentials::create_identity("test", "bob");

// Mock reducer context with specific sender
spacetimedb::ReducerContext mock_ctx;
mock_ctx.sender = alice;

// Test permission checks
assert(has_role(mock_ctx, alice, "user") == true);
assert(has_role(mock_ctx, bob, "admin") == false);
```