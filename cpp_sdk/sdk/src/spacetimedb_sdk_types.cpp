#include "spacetimedb/sdk/spacetimedb_sdk_types.h"
#include "spacetimedb/bsatn/reader.h"
#include "spacetimedb/bsatn/writer.h"
#include <array>
#include <stdexcept>
#include <string> 
#include <iomanip> 
#include <sstream> 
#include <chrono> 

namespace SpacetimeDb {
    namespace sdk {

        // Identity methods (assuming they are defined elsewhere or will be added, as they were not inline)
        // Identity::Identity() { value.fill(0); }
        // Identity::Identity(const std::array<uint8_t, IDENTITY_SIZE>& bytes) : value(bytes) {}
        // const std::array<uint8_t, IDENTITY_SIZE>& Identity::get_bytes() const { return value; }
        // std::string Identity::to_hex_string() const { /* ... */ }
        // bool Identity::operator==(const Identity& other) const { return value == other.value; }
        // bool Identity::operator!=(const Identity& other) const { return !(*this == other); }
        // bool Identity::operator<(const Identity& other) const { return value < other.value; }
        // void Identity::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const { writer.write_bytes(value.data(), IDENTITY_SIZE); }
        // void Identity::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) { reader.read_bytes(value.data(), IDENTITY_SIZE); }

        // Timestamp methods (assuming they are defined elsewhere or will be added, as they were not inline)
        // Timestamp::Timestamp() : ms_since_epoch(0) {}
        // Timestamp::Timestamp(uint64_t milliseconds_since_epoch) : ms_since_epoch(milliseconds_since_epoch) {}
        // uint64_t Timestamp::as_milliseconds() const { return ms_since_epoch; }
        // Timestamp Timestamp::current() { /* ... */ }
        // bool Timestamp::operator==(const Timestamp& other) const { return ms_since_epoch == other.ms_since_epoch; }
        // bool Timestamp::operator!=(const Timestamp& other) const { return !(*this == other); }
        // bool Timestamp::operator<(const Timestamp& other) const { return ms_since_epoch < other.ms_since_epoch; }
        // bool Timestamp::operator<=(const Timestamp& other) const { return ms_since_epoch <= other.ms_since_epoch; }
        // bool Timestamp::operator>(const Timestamp& other) const { return ms_since_epoch > other.ms_since_epoch; }
        // bool Timestamp::operator>=(const Timestamp& other) const { return ms_since_epoch >= other.ms_since_epoch; }
        // void Timestamp::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const { writer.write_u64_le(ms_since_epoch); }
        // void Timestamp::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) { ms_since_epoch = reader.read_u64_le(); }

        void ScheduleAt::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_u64_le(timestamp_micros);
        }
        void ScheduleAt::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
            timestamp_micros = reader.read_u64_le();
        }

        void ConnectionId::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_u64_le(id);
        }
        void ConnectionId::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
            id = reader.read_u64_le();
        }

        void TimeDuration::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
            writer.write_i64_le(nanoseconds);
        }
        void TimeDuration::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
            nanoseconds = reader.read_i64_le();
        }

        // Definitions for u256_placeholder and i256_placeholder bsatn methods
        // These need the full Reader/Writer definitions.
        void u256_placeholder::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const { writer.write_u256_le(*this); }
        void u256_placeholder::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) { *this = reader.read_u256_le(); }

        void i256_placeholder::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const { writer.write_i256_le(*this); }
        void i256_placeholder::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) { *this = reader.read_i256_le(); }

    } // namespace sdk
} // namespace SpacetimeDb
