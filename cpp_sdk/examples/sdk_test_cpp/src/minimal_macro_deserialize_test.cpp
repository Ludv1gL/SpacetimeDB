// minimal_macro_deserialize_test.cpp
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

// Include the copied headers
#include "spacetimedb_sdk_types.h" // Defines SpacetimeDb::CoreType etc.
#include "bsatn.h"       // Might define bsatn::deserialize generic template & Reader/Writer
#include "reader.h"      // Actual bsatn::Reader
#include "writer.h"      // Actual bsatn::Writer (in case macros use it)
#include "macros.h"      // The macros we are testing

// Declare the enum class first
enum class MyTestEnum {
    ValA,
    ValB
};

// Test Enum & Macro Invocation
// Define the vector of variants first to avoid preprocessor issues with initializer lists
std::vector<::SpacetimeDb::EnumVariantDefinition> my_test_enum_variants =
    {::SpacetimeDb::EnumVariantDefinition{"ValA"}, ::SpacetimeDb::EnumVariantDefinition{"ValB"}};

SPACETIMEDB_TYPE_ENUM(MyTestEnum, MyTestEnum, "MyTestEnum", my_test_enum_variants);

int main() {
    // Instantiate the real bsatn::Reader.
    unsigned char buffer[1] = {0}; // Dummy buffer for the reader
    // The first element ValA (0) will be written as 0x00.
    // If the buffer was {1}, it would be ValB.
    ::bsatn::Reader reader_instance(reinterpret_cast<const std::byte*>(buffer), 1);

    MyTestEnum result = ::bsatn::deserialize<MyTestEnum>(reader_instance);

    // The SPACETIMEDB_TYPE_ENUM generates deserialize to read u8.
    // buffer[0] is 0, so ValA (0) should be deserialized.
    if (result == MyTestEnum::ValA) {
        return 0;
    }
    return 1;
}
