#include "spacetimedb/time_duration.h"
#include "spacetimedb/bsatn/reader.h"
#include "spacetimedb/bsatn/writer.h"

namespace SpacetimeDb {

void TimeDuration::bsatn_serialize(SpacetimeDb::bsatn::Writer& writer) const {
    writer.write_i64_le(micros_);
}

TimeDuration TimeDuration::bsatn_deserialize(SpacetimeDb::bsatn::Reader& reader) {
    int64_t micros = reader.read_i64_le();
    return TimeDuration(micros);
}

} // namespace SpacetimeDb