// minimal_field_test.cpp
#include "spacetimedb/sdk/spacetimedb_sdk_types.h"
#include "spacetimedb/macros.h"

// Global Wrapper Function
inline ::SpacetimeDb::FieldDefinition global_field_internal_wrapper(
    const char* name,
    ::SpacetimeDb::CoreType type,
    const char* user_defined_name,
    bool is_optional,
    bool is_unique_field,
    bool is_auto_inc_field
)
{
    // Call the actual function from macros.h using its fully qualified name
    return SPACETIMEDB_FIELD_INTERNAL(name, type, user_defined_name, is_optional, is_unique_field, is_auto_inc_field);
}

namespace TestNamespace {
    #define MY_TEST_FIELD(fieldName, coreType, userDefinedNameStr) \
        /* The result of SPACETIMEDB_FIELD_INTERNAL is FieldDefinition, not directly constexpr assignable to bool. */ \
        /* We just want to check if the call resolves. */ \
        const ::SpacetimeDb::FieldDefinition generated_field_##fieldName = global_field_internal_wrapper(#fieldName, coreType, userDefinedNameStr, false, false, false)

    // Note: userDefinedNameStr for CoreType::I32 should be nullptr or a valid custom type name if applicable
    MY_TEST_FIELD(exampleField, ::SpacetimeDb::CoreType::I32, nullptr);
}

int main() {
    // To ensure usage and prevent optimizing away.
    // Access a member of the struct to ensure it's a complete type.
    // const char* name = TestNamespace::generated_field_exampleField.name;
    // (void)name;
    // The above might not compile if FieldDefinition is opaque or complex.
    // For now, just having the definition of generated_field_exampleField is enough for the test.
    return 0;
}
