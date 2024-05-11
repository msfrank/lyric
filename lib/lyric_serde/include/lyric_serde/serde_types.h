#ifndef LYRIC_SERDE_SERDE_TYPES_H
#define LYRIC_SERDE_SERDE_TYPES_H

#include <absl/container/flat_hash_map.h>

#include <tempo_utils/integer_types.h>

namespace lyric_serde {

    constexpr uint32_t kInvalidOffsetU32      = 0xFFFFFFFF;

    enum class PatchsetVersion {
        Unknown,
        Version1,
    };

    enum class ValueType {
        Invalid,
        Nil,
        Bool,
        Int64,
        Float64,
        UInt64,
        UInt32,
        UInt16,
        UInt8,
        String,
        Attr,
        Element,
    };

    enum class ChangeOperation {
        Invalid,
        SyncOperation,
        AppendOperation,
        InsertOperation,
        UpdateOperation,
        ReplaceOperation,
        RemoveOperation,
        EmitOperation,
        InvokeOperation,
        AcceptOperation,
    };

    struct Resource {
        tu_int16 nsKey;
        tu_uint32 idValue;
    };

    struct NamespaceAddress {
    public:
        NamespaceAddress() : u32(kInvalidOffsetU32) {};
        explicit NamespaceAddress(tu_uint32 u32) : u32(u32) {};
        NamespaceAddress(const NamespaceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NamespaceAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct ValueAddress {
    public:
        ValueAddress() : u32(kInvalidOffsetU32) {};
        explicit ValueAddress(tu_uint32 u32) : u32(u32) {};
        ValueAddress(const ValueAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ValueAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    struct ChangeAddress {
    public:
        ChangeAddress() : u32(kInvalidOffsetU32) {};
        explicit ChangeAddress(tu_uint32 u32) : u32(u32) {};
        ChangeAddress(const ChangeAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != kInvalidOffsetU32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const ChangeAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = kInvalidOffsetU32;
    };

    // forward declarations
    namespace internal {
        class PatchsetReader;
    }
}

#endif // LYRIC_SERDE_SERDE_TYPES_H