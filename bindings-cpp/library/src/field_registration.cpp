#include "spacetimedb/field_registration.h"

namespace SpacetimeDb {

// Serialization implementations - These are not template specializations since they're already declared in header
void serialize_value(std::vector<uint8_t>& buf, const bool& val) {
    buf.push_back(val ? 1 : 0);
}

void serialize_value(std::vector<uint8_t>& buf, const uint8_t& val) {
    buf.push_back(val);
}

void serialize_value(std::vector<uint8_t>& buf, const uint16_t& val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
}

void serialize_value(std::vector<uint8_t>& buf, const uint32_t& val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

void serialize_value(std::vector<uint8_t>& buf, const uint64_t& val) {
    for (int i = 0; i < 8; i++) {
        buf.push_back((val >> (i * 8)) & 0xFF);
    }
}

void serialize_value(std::vector<uint8_t>& buf, const int8_t& val) {
    buf.push_back(static_cast<uint8_t>(val));
}

void serialize_value(std::vector<uint8_t>& buf, const int16_t& val) {
    serialize_value(buf, static_cast<uint16_t>(val));
}

void serialize_value(std::vector<uint8_t>& buf, const int32_t& val) {
    serialize_value(buf, static_cast<uint32_t>(val));
}

void serialize_value(std::vector<uint8_t>& buf, const int64_t& val) {
    serialize_value(buf, static_cast<uint64_t>(val));
}

void serialize_value(std::vector<uint8_t>& buf, const float& val) {
    uint32_t bits;
    memcpy(&bits, &val, sizeof(float));
    serialize_value(buf, bits);
}

void serialize_value(std::vector<uint8_t>& buf, const double& val) {
    uint64_t bits;
    memcpy(&bits, &val, sizeof(double));
    serialize_value(buf, bits);
}

// std::string and std::vector<uint8_t> serialization are already defined inline in the header

} // namespace SpacetimeDb