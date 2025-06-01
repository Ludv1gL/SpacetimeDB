// minimal_deserialize_test.cpp
#include <cstdint> // For uint32_t

// --- Minimal Mock BSATN Reader ---
namespace bsatn {
    class Reader {
    public:
        bool read_u32(uint32_t& val) {
            val = 0;
            return true;
        }
    };

    // Forward declaration of the generic deserialize template
    template<typename T>
    T deserialize(Reader& r);
} // namespace bsatn

// --- Test Enum ---
enum class MyGlobalEnum {
    Val1,
    Val2
};

// --- Generated Deserialization Implementation ---
namespace bsatn {
    // Forward declaration of the specific deserializer implementation
    MyGlobalEnum deserialize_MyGlobalEnum(::bsatn::Reader& reader);

    MyGlobalEnum deserialize_MyGlobalEnum(::bsatn::Reader& reader) {
        uint32_t val;
        if (reader.read_u32(val)) {
            return static_cast<MyGlobalEnum>(val);
        }
        return MyGlobalEnum::Val1;
    }
} // namespace bsatn

// --- BSATN Deserialization Specialization ---
namespace bsatn {
    template<>
    inline MyGlobalEnum deserialize<MyGlobalEnum>(::bsatn::Reader& reader) {
        return ::bsatn::deserialize_MyGlobalEnum(reader);
    }
} // namespace bsatn

// --- Main Test Function ---
int main() {
    ::bsatn::Reader reader_instance;
    MyGlobalEnum result = ::bsatn::deserialize<MyGlobalEnum>(reader_instance);
    if (result == MyGlobalEnum::Val1) {
        return 0;
    }
    return 1;
}
