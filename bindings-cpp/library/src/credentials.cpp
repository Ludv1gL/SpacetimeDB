#include "spacetimedb/credentials.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cstring>

// For Blake3 hashing (simplified implementation for demonstration)
// In production, you would use a proper Blake3 library
// #include <openssl/sha.h>

namespace SpacetimeDb {

// Static constant definitions for identity extensions
namespace identity_extensions {
    const Identity ZERO = Identity(Identity::ByteArray{});
    const Identity ONE = []() {
        Identity::ByteArray bytes{};
        bytes[0] = 1;
        return Identity(bytes);
    }();
}

// Helper function to convert hex character to value
static uint8_t hex_char_to_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::invalid_argument("Invalid hex character");
}

// Helper function to convert byte to hex string
static std::string byte_to_hex(uint8_t byte) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return ss.str();
}

namespace identity_extensions {

Identity from_be_byte_array(const Identity::ByteArray& bytes) {
    Identity::ByteArray le_bytes;
    for (size_t i = 0; i < 32; ++i) {
        le_bytes[i] = bytes[31 - i];
    }
    return Identity(le_bytes);
}

std::optional<Identity> from_hex(const std::string& hex) {
    std::string cleaned = hex;
    
    // Remove "0x" prefix if present
    if (cleaned.size() >= 2 && cleaned[0] == '0' && (cleaned[1] == 'x' || cleaned[1] == 'X')) {
        cleaned = cleaned.substr(2);
    }
    
    // Must be exactly 64 hex characters (32 bytes)
    if (cleaned.size() != 64) {
        return std::nullopt;
    }
    
    ByteArray bytes;
    try {
        for (size_t i = 0; i < 32; ++i) {
            uint8_t high = hex_char_to_value(cleaned[i * 2]);
            uint8_t low = hex_char_to_value(cleaned[i * 2 + 1]);
            bytes[i] = (high << 4) | low;
        }
    } catch (const std::invalid_argument&) {
        return std::nullopt;
    }
    
    // The hex string is big-endian, so use from_be_byte_array
    return from_be_byte_array(bytes);
}

std::string to_hex(const Identity& id) {
    std::stringstream ss;
    ss << "0x";
    
    // Convert to big-endian for display
    auto be_bytes = to_be_byte_array(id);
    for (uint8_t byte : be_bytes) {
        ss << byte_to_hex(byte);
    }
    
    return ss.str();
}

Identity::ByteArray to_be_byte_array(const Identity& id) {
    Identity::ByteArray be_bytes;
    auto le_bytes = id.to_byte_array();
    for (size_t i = 0; i < 32; ++i) {
        be_bytes[i] = le_bytes[31 - i];
    }
    return be_bytes;
}

std::string to_abbreviated_hex(const Identity& id) {
    std::stringstream ss;
    ss << "0x";
    
    // Get first 8 bytes in big-endian
    auto be_bytes = to_be_byte_array(id);
    for (size_t i = 0; i < 8; ++i) {
        ss << byte_to_hex(be_bytes[i]);
    }
    
    return ss.str();
}

} // namespace identity_extensions

// Simple Blake3 hash simulation (for demonstration)
// In production, use actual Blake3 implementation
static std::array<uint8_t, 32> blake3_hash(const uint8_t* data, size_t len) {
    std::array<uint8_t, 32> result;
    // Placeholder - would use actual Blake3 or SHA256
    // SHA256(data, len, result.data());
    // For now, just do a simple hash simulation
    for (size_t i = 0; i < 32; ++i) {
        result[i] = static_cast<uint8_t>((i + len) ^ data[i % len]);
    }
    return result;
}

Identity Credentials::create_identity(const std::string& issuer, const std::string& subject) {
    // Following the same algorithm as Rust's Identity::from_claims
    std::string input = issuer + "|" + subject;
    
    // First hash
    auto first_hash = blake3_hash(reinterpret_cast<const uint8_t*>(input.data()), input.size());
    
    // Take first 26 bytes as id_hash
    std::array<uint8_t, 28> checksum_input{};
    checksum_input[0] = 0xc2;
    checksum_input[1] = 0x00;
    std::memcpy(&checksum_input[2], first_hash.data(), 26);
    
    // Checksum hash
    auto checksum_hash = blake3_hash(checksum_input.data(), checksum_input.size());
    
    // Build final identity bytes
    Identity::ByteArray final_bytes{};
    final_bytes[0] = 0xc2;
    final_bytes[1] = 0x00;
    std::memcpy(&final_bytes[2], checksum_hash.data(), 4);
    std::memcpy(&final_bytes[6], first_hash.data(), 26);
    
    // The bytes are formatted in big-endian for display
    return identity_extensions::from_be_byte_array(final_bytes);
}

// Simple JWT parsing (for demonstration - use a proper JWT library in production)
static std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Base64 URL decode (simplified - use proper implementation in production)
static std::string base64_url_decode(const std::string& input) {
    // This is a placeholder - implement proper base64url decoding
    return input;
}

std::optional<SpacetimeIdentityClaims> AuthToken::parse_claims() const {
    // Simple JWT parsing - splits by '.' and decodes the payload
    auto parts = split_string(jwt_, '.');
    if (parts.size() != 3) {
        return std::nullopt;
    }
    
    // Decode payload (parts[1])
    std::string payload = base64_url_decode(parts[1]);
    
    // This is a placeholder for actual JSON parsing
    // In production, use a proper JSON library like nlohmann/json
    SpacetimeIdentityClaims claims;
    // ... parse JSON and populate claims ...
    
    return claims;
}

bool AuthToken::is_expired() const {
    auto claims = parse_claims();
    if (!claims || !claims->expires_at) {
        return false;  // No expiration
    }
    
    return std::chrono::system_clock::now() > *claims->expires_at;
}

std::optional<AuthToken> Credentials::generate_test_token(
    const Identity& identity,
    const std::string& subject,
    const std::string& issuer,
    std::chrono::seconds validity_duration
) {
    // This is a placeholder - actual token generation requires
    // server-side signing keys and proper JWT library
    return std::nullopt;
}

bool Credentials::validate_token_format(const AuthToken& token) {
    auto parts = split_string(token.get_jwt(), '.');
    return parts.size() == 3 && !token.is_expired();
}

std::optional<Identity> Credentials::extract_identity(const AuthToken& token) {
    auto claims = token.parse_claims();
    if (!claims) {
        return std::nullopt;
    }
    return claims->identity;
}

} // namespace SpacetimeDb

// FFI implementation
extern "C" {

__attribute__((weak))
void identity(uint8_t* out) {
    // Weak symbol - will be overridden by runtime
    // For testing, return zero identity
    std::memset(out, 0, 32);
}

} // extern "C"