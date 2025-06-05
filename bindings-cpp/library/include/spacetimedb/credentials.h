#ifndef SPACETIMEDB_LIBRARY_CREDENTIALS_H
#define SPACETIMEDB_LIBRARY_CREDENTIALS_H

#include <array>
#include <chrono>
#include <optional>
#include <string>
#include <vector>
#include "builtin_reducers.h" // For Identity class

namespace SpacetimeDb {

// Extension methods for Identity class (defined in builtin_reducers.h)
// These are implemented in credentials.cpp
namespace identity_extensions {
    // Create from big-endian byte array (for hex string conversion)
    Identity from_be_byte_array(const Identity::ByteArray& bytes);
    
    // Create from hex string (e.g., "0xc200...")
    std::optional<Identity> from_hex(const std::string& hex);
    
    // Convert to hex string
    std::string to_hex(const Identity& id);
    
    // Get abbreviated hex (first 16 characters)
    std::string to_abbreviated_hex(const Identity& id);
    
    // Convert to big-endian byte array
    Identity::ByteArray to_be_byte_array(const Identity& id);
    
    // Special identities
    extern const Identity ZERO;
    extern const Identity ONE;
}

/**
 * JWT claims for SpacetimeDB authentication
 */
struct SpacetimeIdentityClaims {
    Identity identity;
    std::string subject;
    std::string issuer;
    std::vector<std::string> audience;
    std::chrono::system_clock::time_point issued_at;
    std::optional<std::chrono::system_clock::time_point> expires_at;
};

/**
 * Authentication token for SpacetimeDB
 */
class AuthToken {
public:
    // Create a new auth token from a JWT string
    explicit AuthToken(const std::string& jwt) : jwt_(jwt) {}
    
    // Get the raw JWT string
    const std::string& get_jwt() const { return jwt_; }
    
    // Parse and validate the token claims (without signature verification)
    std::optional<SpacetimeIdentityClaims> parse_claims() const;
    
    // Check if token is expired
    bool is_expired() const;
    
private:
    std::string jwt_;
};

/**
 * Credential management utilities
 */
class Credentials {
public:
    // Create an identity from issuer and subject claims
    // This uses the same algorithm as SpacetimeDB's Identity::from_claims
    static Identity create_identity(const std::string& issuer, const std::string& subject);
    
    // Generate a token for testing (module-side only)
    // Note: This is for testing only and requires server-side key
    static std::optional<AuthToken> generate_test_token(
        const Identity& identity,
        const std::string& subject,
        const std::string& issuer,
        std::chrono::seconds validity_duration = std::chrono::hours(24)
    );
    
    // Validate token format and expiration (basic validation without signature)
    static bool validate_token_format(const AuthToken& token);
    
    // Extract identity from a token
    static std::optional<Identity> extract_identity(const AuthToken& token);
};

// FFI function to get current module identity
extern "C" void identity(uint8_t* out);

/**
 * Get the current module's identity
 * This should only be called from within a reducer context
 */
inline Identity get_module_identity() {
    Identity::ByteArray bytes;
    identity(bytes.data());
    return Identity(bytes);
}

} // namespace SpacetimeDb

#endif // SPACETIMEDB_LIBRARY_CREDENTIALS_H