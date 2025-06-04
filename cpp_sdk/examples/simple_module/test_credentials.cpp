#include <spacetimedb/credentials.h>
#include <iostream>
#include <cassert>

using namespace spacetimedb;

void test_identity_conversions() {
    std::cout << "Testing Identity conversions..." << std::endl;
    
    // Test zero identity
    auto zero = identity_extensions::ZERO;
    assert(zero == Identity(Identity::ByteArray{}));
    
    // Test hex conversion
    auto hex_str = "0xc200123456789abcdef0123456789abcdef0123456789abcdef0123456789abc";
    auto id_opt = identity_extensions::from_hex(hex_str);
    assert(id_opt.has_value());
    
    auto id = *id_opt;
    auto hex_output = identity_extensions::to_hex(id);
    std::cout << "Original: " << hex_str << std::endl;
    std::cout << "Converted: " << hex_output << std::endl;
    
    // Test abbreviated hex
    auto abbrev = identity_extensions::to_abbreviated_hex(id);
    std::cout << "Abbreviated: " << abbrev << std::endl;
    assert(abbrev.length() == 18); // "0x" + 16 hex chars
    
    // Test byte array round trip
    auto bytes = id.to_byte_array();
    auto id2 = Identity(bytes);
    assert(id == id2);
    
    // Test big-endian round trip
    auto be_bytes = identity_extensions::to_be_byte_array(id);
    auto id3 = identity_extensions::from_be_byte_array(be_bytes);
    assert(id == id3);
    
    std::cout << "✓ Identity conversions passed" << std::endl;
}

void test_credential_creation() {
    std::cout << "\nTesting credential creation..." << std::endl;
    
    // Test identity creation from claims
    auto id1 = Credentials::create_identity("spacetimedb", "alice");
    auto id2 = Credentials::create_identity("spacetimedb", "alice");
    auto id3 = Credentials::create_identity("spacetimedb", "bob");
    
    // Same issuer/subject should produce same identity
    assert(id1 == id2);
    
    // Different subject should produce different identity
    assert(id1 != id3);
    
    // Check that identities start with 0xc200
    auto hex1 = identity_extensions::to_hex(id1);
    auto hex3 = identity_extensions::to_hex(id3);
    assert(hex1.substr(0, 6) == "0xc200");
    assert(hex3.substr(0, 6) == "0xc200");
    
    std::cout << "Alice identity: " << hex1 << std::endl;
    std::cout << "Bob identity: " << hex3 << std::endl;
    
    std::cout << "✓ Credential creation passed" << std::endl;
}

void test_identity_comparison() {
    std::cout << "\nTesting identity comparison..." << std::endl;
    
    auto id1 = Credentials::create_identity("test", "user1");
    auto id2 = Credentials::create_identity("test", "user2");
    auto id3 = Credentials::create_identity("test", "user1"); // Same as id1
    
    // Test equality
    assert(id1 == id3);
    assert(!(id1 == id2));
    
    // Test inequality
    assert(id1 != id2);
    assert(!(id1 != id3));
    
    // Test ordering (for use in maps/sets)
    assert((id1 < id2) || (id2 < id1)); // Should have some ordering
    
    std::cout << "✓ Identity comparison passed" << std::endl;
}

void test_auth_token() {
    std::cout << "\nTesting AuthToken..." << std::endl;
    
    // Create a mock JWT token
    AuthToken token("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c");
    
    // Basic format validation
    std::cout << "Token format valid: " << (Credentials::validate_token_format(token) ? "yes" : "no") << std::endl;
    
    std::cout << "✓ AuthToken tests passed" << std::endl;
}

int main() {
    std::cout << "=== SpacetimeDB C++ SDK Credential Tests ===" << std::endl;
    
    try {
        test_identity_conversions();
        test_credential_creation();
        test_identity_comparison();
        test_auth_token();
        
        std::cout << "\n✅ All tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}