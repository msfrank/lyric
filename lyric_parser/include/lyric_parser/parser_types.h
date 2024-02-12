#ifndef LYRIC_PARSER_PARSER_TYPES_H
#define LYRIC_PARSER_PARSER_TYPES_H

#include <memory>

#include <tempo_utils/integer_types.h>

#define ARCHETYPE_INVALID_OFFSET_U32    ((tu_uint32) 0xffffffff)
#define ARCHETYPE_INVALID_OFFSET_U16    ((uint16_t) 0xffff)
#define ARCHETYPE_INVALID_OFFSET_U8     ((uint8_t)  0xff)

namespace lyric_parser {

    constexpr tu_uint32 INVALID_ADDRESS_U32     = 0xFFFFFFFF;

    enum class ArchetypeVersion {
        Unknown,
        Version1,
    };

    enum class BindingType {
        DESCRIPTOR,
        VALUE,
        VARIABLE,
    };

    enum class MutationType {
        MUTABLE,
        IMMUTABLE,
    };

    enum class AccessType {
        PUBLIC,
        PROTECTED,
        PRIVATE,
    };

    enum class BoundType {
        NONE,
        EXTENDS,
        SUPER,
    };

    enum class VarianceType {
        INVARIANT,
        COVARIANT,
        CONTRAVARIANT,
    };

    enum class BaseType {
        INVALID,
        BINARY,
        OCTAL,
        DECIMAL,
        HEX,
    };

    enum class NotationType {
        INVALID,
        FIXED,
        SCIENTIFIC,
    };

    struct NamespaceAddress {
    public:
        NamespaceAddress() : u32(INVALID_ADDRESS_U32) {};
        explicit NamespaceAddress(tu_uint32 u32) : u32(u32) {};
        NamespaceAddress(const NamespaceAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NamespaceAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = INVALID_ADDRESS_U32;
    };

    struct AttrAddress {
    public:
        AttrAddress() : u32(INVALID_ADDRESS_U32) {};
        explicit AttrAddress(tu_uint32 u32) : u32(u32) {};
        AttrAddress(const AttrAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const AttrAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = INVALID_ADDRESS_U32;
    };

    struct NodeAddress {
    public:
        NodeAddress() : u32(INVALID_ADDRESS_U32) {};
        explicit NodeAddress(tu_uint32 u32) : u32(u32) {};
        NodeAddress(const NodeAddress &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getAddress() const { return u32; };
        bool operator==(const NodeAddress &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32 = INVALID_ADDRESS_U32;
    };

    struct AttrId {
        AttrId();
        AttrId(const NamespaceAddress &address, tu_uint32 type);
        AttrId(const AttrId &other);

        NamespaceAddress getAddress() const;
        tu_uint32 getType() const;

        bool operator==(const AttrId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const AttrId &id) {
            return H::combine(std::move(h), id.m_address.getAddress(), id.m_type);
        }

    private:
        NamespaceAddress m_address;
        tu_uint32 m_type;
    };

    // forward declarations
    namespace internal {
        class ArchetypeReader;
    }
}

#endif // LYRIC_PARSER_PARSER_TYPES_H
