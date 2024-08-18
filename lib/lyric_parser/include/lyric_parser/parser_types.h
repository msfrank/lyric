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

    enum class AccessType {
        Public,
        Protected,
        Private,
    };

    enum class BoundType {
        None,
        Extends,
        Super,
    };

    enum class VarianceType {
        Invariant,
        Covariant,
        Contravariant,
    };

    enum class BaseType {
        Binary,
        Octal,
        Decimal,
        Hex,
    };

    enum class NotationType {
        Fixed,
        Scientific,
    };

    enum class ArchetypeDescriptorType {
        Invalid,
        Namespace,
        Node,
        Attr,
    };

    class ArchetypeId {
    public:
        bool isValid() const;
        ArchetypeDescriptorType getType() const;
        tu_uint32 getId() const;
        tu_uint32 getOffset() const;

    private:
        ArchetypeDescriptorType m_type;
        tu_uint32 m_id;
        tu_uint32 m_offset;

        friend class ArchetypeState;
        ArchetypeId(ArchetypeDescriptorType type, tu_uint32 id, tu_uint32 offset);
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

    struct ParseLocation {
        tu_int64 lineNumber;
        tu_int64 columnNumber;
        tu_int64 fileOffset;
        tu_int64 textSpan;
        ParseLocation();
        ParseLocation(
            tu_int64 lineNumber,
            tu_int64 columnNumber,
            tu_int64 fileOffset,
            tu_int64 textSpan);
        bool isValid() const;
    };

    // forward declarations
    namespace internal {
        class ArchetypeReader;
    }
}

#endif // LYRIC_PARSER_PARSER_TYPES_H
