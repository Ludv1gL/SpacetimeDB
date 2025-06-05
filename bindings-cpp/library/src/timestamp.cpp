#include "spacetimedb/timestamp.h"
#include "spacetimedb/bsatn/reader.h"
#include "spacetimedb/bsatn/writer.h"

namespace SpacetimeDb {

void Timestamp::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
    writer.write_u64_le(micros_since_epoch_);
}

Timestamp Timestamp::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
    uint64_t micros = reader.read_u64_le();
    return Timestamp(micros);
}

} // namespace SpacetimeDb