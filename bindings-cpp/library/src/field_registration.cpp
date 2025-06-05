#include "spacetimedb/field_registration.h"

namespace SpacetimeDb {

// Serialization implementations
template<>
void serialize_value(std::vector<uint8_t>& buf, const bool& val) {
    buf.push_back(val ? 1 : 0);
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const uint8_t& val) {
    buf.push_back(val);
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const uint16_t& val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const uint32_t& val) {
    buf.push_back(val & 0xFF);
    buf.push_back((val >> 8) & 0xFF);
    buf.push_back((val >> 16) & 0xFF);
    buf.push_back((val >> 24) & 0xFF);
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const uint64_t& val) {
    for (int i = 0; i < 8; i++) {
        buf.push_back((val >> (i * 8)) & 0xFF);
    }
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const int8_t& val) {
    buf.push_back(static_cast<uint8_t>(val));
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const int16_t& val) {
    serialize_value(buf, static_cast<uint16_t>(val));
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const int32_t& val) {
    serialize_value(buf, static_cast<uint32_t>(val));
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const int64_t& val) {
    serialize_value(buf, static_cast<uint64_t>(val));
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const float& val) {
    uint32_t bits;
    memcpy(&bits, &val, sizeof(float));
    serialize_value(buf, bits);
}

template<>
void serialize_value(std::vector<uint8_t>& buf, const double& val) {
    uint64_t bits;
    memcpy(&bits, &val, sizeof(double));
    serialize_value(buf, bits);
}

void serialize_value(std::vector<uint8_t>& buf, const std::string& val) {
    write_u32(buf, val.length());
    for (char c : val) {
        buf.push_back(static_cast<uint8_t>(c));
    }
}

void serialize_value(std::vector<uint8_t>& buf, const std::vector<uint8_t>& val) {
    write_u32(buf, val.size());
    for (uint8_t b : val) {
        buf.push_back(b);
    }
}

// Explicit instantiations for common types
template void serialize_value<bool>(std::vector<uint8_t>&, const bool&);
template void serialize_value<uint8_t>(std::vector<uint8_t>&, const uint8_t&);
template void serialize_value<uint16_t>(std::vector<uint8_t>&, const uint16_t&);
template void serialize_value<uint32_t>(std::vector<uint8_t>&, const uint32_t&);
template void serialize_value<uint64_t>(std::vector<uint8_t>&, const uint64_t&);
template void serialize_value<int8_t>(std::vector<uint8_t>&, const int8_t&);
template void serialize_value<int16_t>(std::vector<uint8_t>&, const int16_t&);
template void serialize_value<int32_t>(std::vector<uint8_t>&, const int32_t&);
template void serialize_value<int64_t>(std::vector<uint8_t>&, const int64_t&);
template void serialize_value<float>(std::vector<uint8_t>&, const float&);
template void serialize_value<double>(std::vector<uint8_t>&, const double&);

} // namespace SpacetimeDb