// minimal_if_constexpr_test.cpp
#include <cstdint>
#include <optional> // For testing the problematic branches
#include <vector>   // For testing the problematic branches
#include <type_traits> // For std::is_assignable_v, std::is_same_v

// --- Minimal Mock BSATN Reader & Writer ---
namespace bsatn {
    class Reader {
    public:
        // Method for plain type
        bool read_u8(uint8_t& val) { val = 1; return true; }

        // Methods that might be called by IS_OPTIONAL branches (even if not taken for uint8_t)
        template<typename T>
        bool read_optional_u8(std::optional<T>& opt_val) {
            opt_val = static_cast<T>(2); return true;
        }
    };

    class Writer {
    public:
        // Method for plain type
        bool write_u8(uint8_t val) { (void)val; return true; }

        // Methods that might be called by IS_OPTIONAL branches
        template<typename T>
        bool write_optional_u8(const std::optional<T>& opt_val) {
            (void)opt_val; return true;
        }
    };
} // namespace bsatn

// --- Test Struct ---
struct ByteStruct {
    uint8_t value;
};

// --- Simplified (De)serialization logic mimicking macros.h ---
// IS_OPTIONAL and IS_VECTOR would be const bool template args in real macros

// Deserialize
template<typename FieldType, bool IS_OPTIONAL, bool IS_VECTOR>
void deserialize_field_test(FieldType& field_member, ::bsatn::Reader& reader) {
    if constexpr (IS_OPTIONAL) {
        // This is the kind of branch that might cause issues if FieldType = uint8_t
        // The error was "assigning to 'uint8_t' from incompatible type 'std::optional<unsigned char>'"
        // So, the problematic assignment would be something like:
        // field_member = reader.read_optional_u8(...); // This line is problematic if FieldType is not std::optional
        // Let's try to make it more direct to the error:
        std::optional<uint8_t> temp_opt; // Correct type for the read_optional_u8
        reader.read_optional_u8(temp_opt);
        if constexpr (std::is_same_v<FieldType, std::optional<uint8_t>>) {
             field_member = temp_opt; // This is fine
        } else if (!IS_VECTOR) { // Added to ensure this branch is taken for non-optional, non-vector
             // field_member = temp_opt.value_or(0); // This would be a runtime error if temp_opt is nullopt, and still type mismatch if field_member is not uint8_t
             // The error implies direct assignment or construction:
             // field_member = temp_opt; // THIS IS THE PROBLEMATIC LINE if FieldType is uint8_t
                                      // Let's simulate the error more closely.
                                      // If FieldType is uint8_t, this attempts uint8_t = std::optional<uint8_t>
             // To reproduce "assigning to 'uint8_t' from 'std::optional<unsigned char>'"
             // where unsigned char might be uint8_t:
             if constexpr (!std::is_same_v<FieldType, std::optional<uint8_t>>) { // Ensure this is for the non-optional FieldType path
                // This line should only be "active" if IS_OPTIONAL is true.
                // The problem is it might be *parsed* and cause errors even if IS_OPTIONAL is false for FieldType=uint8_t.
                // Let's try to force the problematic assignment:
                // field_member = std::optional<uint8_t>(3); // This would show the error.
             }
        }
        // The actual macros are more complex. Let's simplify to the core issue:
        // The compiler sees a path inside `if constexpr(IS_OPTIONAL)` that would be invalid
        // if `FieldType` is not `std::optional`.
        // The error implies an assignment like `field_member = some_optional_value;`
    } else if constexpr (IS_VECTOR) {
        // Similar logic for vectors
    } else {
        // Plain type
        // reader.read_u8(field_member); // Corrected: needs to be compatible with FieldType
        if constexpr (std::is_same_v<FieldType, uint8_t>) {
             reader.read_u8(field_member);
        }
    }
}

// To really trigger the error "assigning to 'T' from 'std::optional<U>'"
// we need the branch to be parsed.
// Let's make a version that is closer to what the error message implies.
template<typename FieldType>
void deserialize_field_test_v2(FieldType& field_member, ::bsatn::Reader& reader, bool is_optional_runtime) {
    if (is_optional_runtime) { // This is runtime, so both branches are parsed
        std::optional<uint8_t> temp_opt_val;
        reader.read_optional_u8(temp_opt_val); // reads std::optional<uint8_t>
        // field_member = temp_opt_val; // This would error if FieldType is uint8_t. This is what we want to test for if constexpr.
                                    // This is the line that the error message implies is happening.
    } else {
        // reader.read_u8(field_member); // This is fine if FieldType is uint8_t
    }
}

// Let's use if constexpr as originally intended for the test.
// The issue is that even if IS_OPTIONAL is false, the std::optional branch is problematic.
template<typename FieldType, const bool IS_OPTIONAL_FLAG, const bool IS_VECTOR_FLAG>
void deserialize_field_test_v3(FieldType& field_member, ::bsatn::Reader& reader) {
    if constexpr (IS_OPTIONAL_FLAG) {
        std::optional<uint8_t> temp_val; // This is fine
        reader.read_optional_u8(temp_val); // This is fine
        // field_member = temp_val; // This is the problematic line for FieldType = uint8_t
                                // This causes "cannot convert std::optional<uint8_t> to uint8_t"
                                // For the test, let's make this assignment happen to see if if constexpr prevents it
                                // when IS_OPTIONAL_FLAG is false.
        if constexpr (std::is_assignable_v<FieldType&, std::optional<uint8_t>>) {
             field_member = temp_val;
        } else {
            // This is where the error would be if not for the above check.
            // The problem is the compiler might still instantiate code that *would* lead to an error.
            // Let's try to provoke the error directly if this branch is taken.
            // field_member = std::optional<uint8_t>{0}; // This exact line.
        }
    } else if constexpr (IS_VECTOR_FLAG) {
        // ...
    } else {
        // This is the path for ByteStruct's value field
        // Assuming FieldType is uint8_t for ByteStruct.value
        if constexpr (std::is_assignable_v<FieldType&, uint8_t>) { // Should be true for uint8_t
             // Corrected: was reader.read_u8(field_member); but field_member is FieldType&
             // Ensure field_member is actually uint8_t for this call.
             if constexpr (std::is_same_v<FieldType, uint8_t>) {
                reader.read_u8(field_member);
             }
        }
    }
}


// Serialize (simplified)
template<typename FieldType, bool IS_OPTIONAL, bool IS_VECTOR>
void serialize_field_test(const FieldType& field_member, ::bsatn::Writer& writer) {
    if constexpr (IS_OPTIONAL) {
        // writer.write_optional_u8(field_member); // This would be an error if FieldType is uint8_t
    } else if constexpr (IS_VECTOR) {
        // ...
    } else {
        // writer.write_u8(field_member); // Corrected
        if constexpr (std::is_same_v<FieldType, uint8_t>) {
            writer.write_u8(field_member);
        }
    }
}


int main() {
    ByteStruct s;
    s.value = 0;

    ::bsatn::Reader reader_instance;
    ::bsatn::Writer writer_instance;

    // For ByteStruct.value: FieldType=uint8_t, IS_OPTIONAL=false, IS_VECTOR=false
    deserialize_field_test<uint8_t, false, false>(s.value, reader_instance);
    serialize_field_test<uint8_t, false, false>(s.value, writer_instance);

    // Attempt with v3 which tries to provoke the error inside the IS_OPTIONAL=true branch
    // We call it with IS_OPTIONAL_FLAG = false
    deserialize_field_test_v3<uint8_t, false, false>(s.value, reader_instance);
    // If the above compiles, then if constexpr is working as expected for this simplified case.
    // The error "assigning to 'uint8_t' from 'std::optional<unsigned char>'"
    // must come from a more complex interaction in the original macros.

    // Let's add a call that *would* be problematic if not for if constexpr
    std::optional<uint8_t> opt_val;
    deserialize_field_test_v3<std::optional<uint8_t>, true, false>(opt_val, reader_instance);


    if (s.value == 1 && opt_val.has_value() && opt_val.value() == 2) { // Updated expected value for opt_val
         return 0; // Success
    }
    return 1; // Failure
}
