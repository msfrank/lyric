
#include <bit>

#include <boost/endian.hpp>

#include <lyric_runtime/tagged_value.h>

#include "lyric_runtime/bytes_ref.h"
#include "lyric_runtime/namespace_ref.h"
#include "lyric_runtime/protocol_ref.h"
#include "lyric_runtime/status_ref.h"
#include "lyric_runtime/string_ref.h"

/*
 * TODO:
 *  - add singleword enum for empty string
 *  - add singleword enum for empty bytes
 *  - add new tag in doubleword Num32 for tinystring, which can fit 6 ascii chars + length byte
 *  - add new tag in doubleword Num32 for tinybytes, which can fit 6 bytes + length byte
 */

/*
 * singleword value type tags:
 *   Short:     000  (0)
 *   I32:       001  (1)
 *   U32:       010  (2)
 */

/*
 * doubleword value type tags:
 *   Self1:     011  (3)
 *   Self2:     100  (4)
 *   Num32:     101  (5)
 *   Num64:     110  (6)
 *   Pointer:   111  (7)
 */

/*
 * Pointer highbits flags:
 *   Ref:       00000010 (2)
 *   Bytes:     00000100 (4)
 *   Namespace: 00000110 (6)
 *   Protocol:  00001000 (8)
 *   Rest:      00001010 (10)
 *   Status:    00001100 (12)
 *   String:    00001110 (14)
 *   Desc:      00010000 (16)
 *   Type:      00010010 (18)
 */

/*
 * Short byte tags:
 *   Enum:      00001000 (8)
 *   I8:        00010000 (16)
 *   U8:        00011000 (24)
 *   I16:       00100000 (32)
 *   U16:       00101000 (40)
 *   C21:       00110000 (48)
 */

/*
 * Num32 byte tags:
 *   Signed:    00001101 (13)
 *   Unsigned:  00010101 (21)
 *   Float:     00011101 (29)
 */

/*
 * Num64 type tags:
 *   Signed:    0110 (6)
 *   Unsigned:  1110 (14)
 */

/*
 * Enum singleword values:
 *   Invalid:   00 00 00 08
 *   Undef:     00 00 01 08
 *   Nil:       00 00 02 08
 *   True:      00 00 03 08
 *   False:     00 00 04 08
 *   F64Zero:   00 00 05 08
 *   F64NZero:  00 00 06 08
 */

constexpr tu_uint8 kTagMask                 = 0x07;
constexpr tu_uint8 kNum64Mask               = 0x0F;
constexpr tu_uint8 kPointerMask             = 0x7E;

constexpr tu_uint8 kSingleWordShortTag      = 0x00;
constexpr tu_uint8 kSingleWordI32Tag        = 0x01;
constexpr tu_uint8 kSingleWordU32Tag        = 0x02;

constexpr tu_uint8 kI32SignBit              = 0x08;
constexpr tu_uint8 kI64SignBit              = 0x10;

constexpr tu_uint8 kDoubleWordSelf1Tag      = 0x03;
constexpr tu_uint8 kDoubleWordSelf2Tag      = 0x04;
constexpr tu_uint8 kDoubleWordNum32Tag      = 0x05;
constexpr tu_uint8 kDoubleWordNum64Tag      = 0x06;
constexpr tu_uint8 kDoubleWordPointerTag    = 0x07;

constexpr tu_uint8 kShortEnumTag            = 0x08;
constexpr tu_uint8 kShortI8Tag              = 0x10;
constexpr tu_uint8 kShortU8Tag              = 0x18;
constexpr tu_uint8 kShortI16Tag             = 0x20;
constexpr tu_uint8 kShortU16Tag             = 0x28;
constexpr tu_uint8 kShortC21Tag             = 0x30;

constexpr tu_uint8 kNum32SignedTag          = 0x0D;
constexpr tu_uint8 kNum32UnsignedTag        = 0x15;
constexpr tu_uint8 kNum32FloatTag           = 0x1D;

constexpr tu_uint8 kNum64SignedTag          = 0x06;
constexpr tu_uint8 kNum64UnsignedTag        = 0x0E;

constexpr tu_uint8 kPointerRefTag           = 0x02;

constexpr tu_uint8 kPointerBytesTag         = 0x04;
constexpr tu_uint8 kPointerNamespaceTag     = 0x06;
constexpr tu_uint8 kPointerProtocolTag      = 0x08;
constexpr tu_uint8 kPointerRestTag          = 0x0A;
constexpr tu_uint8 kPointerStatusTag        = 0x0C;
constexpr tu_uint8 kPointerStringTag        = 0x0E;
constexpr tu_uint8 kPointerDescTag          = 0x10;
constexpr tu_uint8 kPointerTypeTag          = 0x12;

constexpr tu_uint32 kUndefEnum              = 0x01;
constexpr tu_uint32 kNilEnum                = 0x02;
constexpr tu_uint32 kTrueEnum               = 0x03;
constexpr tu_uint32 kFalseEnum              = 0x04;
constexpr tu_uint32 kF64ZeroEnum            = 0x05;
constexpr tu_uint32 kF64NZeroEnum           = 0x06;

lyric_runtime::TaggedValue::TaggedValue()
    : m_repr(TaggedRepresentation::Invalid)
{
}

lyric_runtime::TaggedValue::TaggedValue(TaggedRepresentation repr, std::array<tu_uint8,8> bytes)
    : m_repr(repr),
      m_bytes(bytes)
{
}

lyric_runtime::TaggedValue::TaggedValue(const TaggedValue &other)
    : m_repr(other.m_repr),
      m_bytes(other.m_bytes)
{
}

lyric_runtime::TaggedValue::TaggedValue(TaggedValue &&other) noexcept
    : m_repr(std::exchange(other.m_repr, TaggedRepresentation::Invalid)),
      m_bytes{std::exchange(other.m_bytes, {})}
{
}

lyric_runtime::TaggedValue&
lyric_runtime::TaggedValue::operator=(const TaggedValue &other)
{
    m_repr = other.m_repr;
    m_bytes = other.m_bytes;
    return *this;
}

lyric_runtime::TaggedValue&
lyric_runtime::TaggedValue::operator=(TaggedValue &&other) noexcept
{
    if (this != &other) {
        m_repr = std::exchange(other.m_repr, TaggedRepresentation::Invalid);
        m_bytes.swap(other.m_bytes);
    }
    return *this;
}

bool
lyric_runtime::TaggedValue::isValid() const
{
    return m_repr != TaggedRepresentation::Invalid;
}

lyric_runtime::TaggedRepresentation
lyric_runtime::TaggedValue::getRepresentation() const
{
    return m_repr;
}

std::span<const tu_uint8>
lyric_runtime::TaggedValue::rawValue() const
{
    return std::span(m_bytes.data(), m_bytes.size());
}

inline tu_uint8 get_info_byte(const std::array<tu_uint8,8> &bytes) { return bytes[7]; }

inline tu_uint8 get_info_tag(const std::array<tu_uint8,8> &bytes) { return bytes[7] & kTagMask; }

lyric_runtime::DataCellType
lyric_runtime::TaggedValue::getType() const
{
    tu_uint8 info = get_info_byte(m_bytes);
    switch (m_repr) {
        case TaggedRepresentation::SmallValue: {
            tu_uint8 tag = info & kTagMask;
            switch (tag) {
                case kSingleWordI32Tag:
                    return DataCellType::Int32;
                case kSingleWordU32Tag:
                    return DataCellType::UInt32;
                case kSingleWordShortTag: {
                    const tu_uint8 shorttag = info;
                    switch (shorttag) {
                        case kShortI8Tag:
                            return DataCellType::Int8;
                        case kShortU8Tag:
                            return DataCellType::UInt8;
                        case kShortI16Tag:
                            return DataCellType::Int16;
                        case kShortU16Tag:
                            return DataCellType::UInt16;
                        case kShortC21Tag:
                            return DataCellType::Char32;
                        case kShortEnumTag: {
                            tu_uint8 enumtag = m_bytes[6];
                            switch (enumtag) {
                                case kUndefEnum:
                                    return DataCellType::Undef;
                                case kNilEnum:
                                    return DataCellType::Nil;
                                case kTrueEnum:
                                case kFalseEnum:
                                    return DataCellType::Bool;
                                case kF64ZeroEnum:
                                case kF64NZeroEnum:
                                    return DataCellType::Float64;
                                default:
                                    return DataCellType::Invalid;
                            }
                        }
                        default:
                            return DataCellType::Invalid;
                    }
                }
                default:
                    return DataCellType::Invalid;
            }
        }

        case TaggedRepresentation::LargeValue: {
            tu_uint8 tag = info & kTagMask;
            switch (tag) {
                case kDoubleWordSelf1Tag:
                case kDoubleWordSelf2Tag:
                    return DataCellType::Float64;
                case kDoubleWordNum32Tag: {
                    tu_uint8 num32tag = info;
                    switch (num32tag) {
                        case kNum32SignedTag:
                            return DataCellType::Int32;
                        case kNum32UnsignedTag:
                            return DataCellType::UInt32;
                        case kNum32FloatTag:
                            return DataCellType::Float32;
                        default:
                            return DataCellType::Invalid;
                    }

                }
                case kDoubleWordNum64Tag: {
                    tu_uint8 num64tag = info & 0x0f;
                    switch (num64tag) {
                        case kNum64SignedTag:
                            return DataCellType::Int64;
                        case kNum64UnsignedTag:
                            return DataCellType::UInt64;
                        default:
                            return DataCellType::Invalid;
                    }
                }
                default:
                    return DataCellType::Invalid;
            }
        }

        case TaggedRepresentation::Pointer: {
            tu_uint8 tag = info & kTagMask;
            if (tag != kDoubleWordPointerTag)
                return DataCellType::Invalid;
            tu_uint8 pointertag = m_bytes[0] & kPointerMask;
            switch (pointertag) {
                case kPointerRefTag:
                    return DataCellType::Ref;
                case kPointerBytesTag:
                    return DataCellType::Bytes;
                case kPointerNamespaceTag:
                    return DataCellType::Namespace;
                case kPointerProtocolTag:
                    return DataCellType::Protocol;
                case kPointerRestTag:
                    return DataCellType::Rest;
                case kPointerStatusTag:
                    return DataCellType::Status;
                case kPointerStringTag:
                    return DataCellType::String;
                case kPointerDescTag:
                    return DataCellType::Descriptor;
                case kPointerTypeTag:
                    return DataCellType::Type;
                default:
                    return DataCellType::Invalid;
            }
        }

        default:
            return DataCellType::Invalid;
    }
}

inline tu_uint8 get_enum_byte(const std::array<tu_uint8,8> &bytes)
{
    if (get_info_byte(bytes) != kShortEnumTag)
        return 0;
    return bytes[6];
}

bool
lyric_runtime::TaggedValue::isUndef() const
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    return get_enum_byte(m_bytes) == kUndefEnum;
}

bool
lyric_runtime::TaggedValue::isNil() const
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    return get_enum_byte(m_bytes) == kNilEnum;
}

bool
lyric_runtime::TaggedValue::getBool(bool &b)
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    switch (get_enum_byte(m_bytes)) {
        case kTrueEnum:
            b = true;
            return true;
        case kFalseEnum:
            b = false;
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::TaggedValue::getI8(tu_int8 &i8)
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortI8Tag)
        return false;
    i8 = static_cast<tu_int8>(m_bytes[6]);
    return true;
}

bool
lyric_runtime::TaggedValue::getU8(tu_uint8 &u8)
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortU8Tag)
        return false;
    u8 = m_bytes[6];
    return true;
}

bool
lyric_runtime::TaggedValue::getI16(tu_int16 &i16)
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortI16Tag)
        return false;
    i16 = boost::endian::endian_load<tu_int16,2,boost::endian::order::native>(m_bytes.data() + 5);
    return true;
}

bool
lyric_runtime::TaggedValue::getU16(tu_uint16 &u16)
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortU16Tag)
        return false;
    u16 = boost::endian::endian_load<tu_uint16,2,boost::endian::order::native>(m_bytes.data() + 5);
    return true;
}

bool
lyric_runtime::TaggedValue::getI32(tu_int32 &i32)
{
    switch (m_repr) {
        case TaggedRepresentation::SmallValue: {
            tu_uint8 info = get_info_byte(m_bytes);
            if ((info & kTagMask) != kSingleWordI32Tag)
                return false;
            auto *ptr = (tu_uint32 *) &m_bytes[4];
            tu_uint32 u32 = boost::endian::big_to_native(*ptr);
            u32 >>= 4;
            i32 = static_cast<tu_int32>(u32);
            if (info & kI32SignBit) {
                i32 = -i32;
            }
            return true;
        }
        case TaggedRepresentation::LargeValue: {
            tu_uint8 info = get_info_byte(m_bytes);
            if (info != kNum32SignedTag)
                return false;
            i32 = boost::endian::endian_load<tu_int32,4,boost::endian::order::native>(m_bytes.data() + 3);
            return true;
        }
        default:
            return false;
    }
}

bool
lyric_runtime::TaggedValue::getU32(tu_uint32 &u32)
{
    switch (m_repr) {
        case TaggedRepresentation::SmallValue: {
            tu_uint8 tag = get_info_tag(m_bytes);
            if (tag != kSingleWordU32Tag)
                return false;
            auto *ptr = (tu_uint32 *) &m_bytes[4];
            u32 = *ptr;
            boost::endian::big_to_native_inplace(u32);
            u32 >>= 3;
            return true;
        }
        case TaggedRepresentation::LargeValue: {
            tu_uint8 info = get_info_byte(m_bytes);
            if (info != kNum32UnsignedTag)
                return false;
            u32 = boost::endian::endian_load<tu_uint32,4,boost::endian::order::native>(m_bytes.data() + 3);
            return true;
        }
        default:
            return false;
    }
}

bool
lyric_runtime::TaggedValue::getI64(tu_int64 &i64)
{
    tu_uint8 info = get_info_byte(m_bytes);
    if ((info & kNum64Mask) != kNum64SignedTag)
        return false;
    auto *ptr = (tu_uint64 *) m_bytes.data();
    tu_uint64 u64 = boost::endian::big_to_native(*ptr);
    u64 >>= 5;
    i64 = static_cast<tu_int64>(u64);
    if (info & kI64SignBit) {
        i64 = -i64;
    }
    return true;
}

bool
lyric_runtime::TaggedValue::getU64(tu_uint64 &u64)
{
    if (m_repr != TaggedRepresentation::LargeValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if ((info & kNum64Mask) != kNum64UnsignedTag)
        return false;
    auto *ptr = (tu_uint64 *) m_bytes.data();
    u64 = *ptr;
    boost::endian::big_to_native_inplace(u64);
    u64 >>= 4;
    return true;
}

bool
lyric_runtime::TaggedValue::getC32(char32_t &c32)
{
    if (m_repr != TaggedRepresentation::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortC21Tag)
        return false;
    auto *ptr = (tu_uint32 *) &m_bytes[4];
    c32 = *ptr;
    boost::endian::big_to_native_inplace(c32);
    c32 >>= 8;
    return true;
}

bool
lyric_runtime::TaggedValue::getF32(float &f32)
{
    if (m_repr != TaggedRepresentation::LargeValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kNum32FloatTag)
        return false;
    auto *ptr = (float *) &m_bytes[3];
    f32 = *ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getF64(double &f64)
{
    if (m_repr == TaggedRepresentation::SmallValue) [[unlikely]] {
        switch (get_enum_byte(m_bytes)) {
            case kF64ZeroEnum:
                f64 = 0.0;
                return true;
            case kF64NZeroEnum:
                f64 = -0.0;
                return true;
            default:
                return false;
        }
    }

    if (m_repr != TaggedRepresentation::LargeValue)
        return false;

    tu_uint8 tag = get_info_tag(m_bytes);
    switch (tag) {
        case kDoubleWordSelf1Tag:
        case kDoubleWordSelf2Tag: {
            tu_uint64 large;
            memcpy(&large, m_bytes.data(), 8);
            boost::endian::big_to_native_inplace(large);
            large = std::rotl(large, 60);
            auto *ptr = (double *) &large;
            f64 = *ptr;
            return true;
        }
        default:
            return false;
    }
}

void *
lyric_runtime::TaggedValue::getPointer(tu_uint8 pointertag)
{
    TU_ASSERT ((pointertag & ~kPointerMask) == 0);
    if (m_repr != TaggedRepresentation::Pointer)
        return nullptr;
    tu_uint8 tag = get_info_tag(m_bytes);
    if (tag != kDoubleWordPointerTag)
        return nullptr;
    if ((m_bytes[0] & kPointerMask) != pointertag)
        return nullptr;
    std::uintptr_t pointer;
    memcpy(&pointer, m_bytes.data(), 8);
    auto *ptr = (tu_uint8 *) &pointer;
    ptr[0] &= ~kPointerMask;
    ptr[7] &= ~kTagMask;
    boost::endian::big_to_native_inplace(pointer);
    return (void *) pointer;
}

bool
lyric_runtime::TaggedValue::getRef(AbstractRef *&ref)
{
    auto *ptr = getPointer(kPointerRefTag);
    if (ptr == nullptr)
        return false;
    ref = (AbstractRef *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getBytes(BytesRef *&bytes)
{
    auto *ptr = getPointer(kPointerBytesTag);
    if (ptr == nullptr)
        return false;
    bytes = (BytesRef *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getNamespace(NamespaceRef *&ns)
{
    auto *ptr = getPointer(kPointerNamespaceTag);
    if (ptr == nullptr)
        return false;
    ns = (NamespaceRef *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getProtocol(ProtocolRef *&protocol)
{
    auto *ptr = getPointer(kPointerProtocolTag);
    if (ptr == nullptr)
        return false;
    protocol = (ProtocolRef *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getRest(RestRef *&rest)
{
    auto *ptr = getPointer(kPointerRestTag);
    if (ptr == nullptr)
        return false;
    rest = (RestRef *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getStatus(StatusRef *&status)
{
    auto *ptr = getPointer(kPointerStatusTag);
    if (ptr == nullptr)
        return false;
    status = (StatusRef *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getString(StringRef *&string)
{
    auto *ptr = getPointer(kPointerStringTag);
    if (ptr == nullptr)
        return false;
    string = (StringRef *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getDescriptor(DescriptorEntry *&descriptor)
{
    auto *ptr = getPointer(kPointerDescTag);
    if (ptr == nullptr)
        return false;
    descriptor = (DescriptorEntry *) ptr;
    return true;
}

bool
lyric_runtime::TaggedValue::getType(TypeEntry *&type)
{
    auto *ptr = getPointer(kPointerTypeTag);
    if (ptr == nullptr)
        return false;
    type = (TypeEntry *) ptr;
    return true;
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromBool(bool b)
{
    std::array<tu_uint8,8> bytes;
    bytes[6] = b? kTrueEnum : kFalseEnum;
    bytes[7] = kShortEnumTag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromI8(tu_int8 i8)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_int8,1,boost::endian::order::native>(bytes.data() + 6, i8);
    bytes[7] = kShortI8Tag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromU8(tu_uint8 u8)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_uint8,1,boost::endian::order::native>(bytes.data() + 6, u8);
    bytes[7] = kShortU8Tag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromI16(tu_int16 i16)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_int16,2,boost::endian::order::native>(bytes.data() + 5, i16);
    bytes[7] = kShortI16Tag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromU16(tu_uint16 u16)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_uint16,2,boost::endian::order::native>(bytes.data() + 5, u16);
    bytes[7] = kShortU16Tag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromI32(tu_int32 i32)
{
    std::array<tu_uint8,8> bytes;
    if (i32 < -0x10000000 || i32 > 0x10000000) [[unlikely]] {
        boost::endian::endian_store<tu_int32,4,boost::endian::order::native>(bytes.data() + 3, i32);
        bytes[7] = kNum32SignedTag;
        return TaggedValue(TaggedRepresentation::LargeValue, bytes);
    }
    tu_uint32 small = std::abs(i32);
    small = boost::endian::native_to_big(small << 4);
    memcpy(bytes.data() + 4, &small, 4);
    bytes[7] |= kSingleWordI32Tag;
    if (i32 < 0) {
        bytes[7] |= kI32SignBit;
    }
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromU32(tu_uint32 u32)
{
    std::array<tu_uint8,8> bytes;
    if (u32 > (0xffffffff >> 3)) [[unlikely]] {
        boost::endian::endian_store<tu_uint32,4,boost::endian::order::native>(bytes.data() + 3, u32);
        bytes[7] = kNum32UnsignedTag;
        return TaggedValue(TaggedRepresentation::LargeValue, bytes);
    }
    tu_uint32 small = boost::endian::native_to_big(u32 << 3);
    memcpy(bytes.data() + 4, &small, 4);
    bytes[7] |= kSingleWordU32Tag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromI64(tu_int64 i64)
{
    if (i64 < -0x800000000000000 || i64 > 0x800000000000000)
        return {};
    std::array<tu_uint8,8> bytes;
    tu_uint64 large = std::abs(i64);
    large = boost::endian::native_to_big(large << 5);
    memcpy(bytes.data(), &large, 8);
    bytes[7] |= kNum64SignedTag;
    if (i64 < 0) {
        bytes[7] |= kI64SignBit;
    }
    return TaggedValue(TaggedRepresentation::LargeValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromU64(tu_uint64 u64)
{
    if (u64 > (0xffffffffffffffff >> 4))
        return {};
    std::array<tu_uint8,8> bytes;
    tu_uint64 large = boost::endian::native_to_big(u64 << 4);
    memcpy(bytes.data(), &large, 8);
    bytes[7] |= kNum64UnsignedTag;
    return TaggedValue(TaggedRepresentation::LargeValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromC32(char32_t c32)
{
    if (c32 > 0x10ffff)
        return {};
    std::array<tu_uint8,8> bytes;
    tu_uint32 small = boost::endian::native_to_big(c32 << 8);
    memcpy(bytes.data() + 4, &small, 4);
    bytes[7] = kShortC21Tag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromF32(float f32)
{
    std::array<tu_uint8,8> bytes;
    memcpy(bytes.data() + 3, &f32, 4);
    bytes[7] = kNum32FloatTag;
    return TaggedValue(TaggedRepresentation::LargeValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromF64(double f64)
{
    std::array<tu_uint8,8> bytes;
    if (f64 == 0.0) [[unlikely]] {
        bytes[6] = std::signbit(f64)? kF64NZeroEnum : kF64ZeroEnum;
        bytes[7] = kShortEnumTag;
        return TaggedValue(TaggedRepresentation::SmallValue, bytes);
    }

    tu_uint64 large;
    memcpy(&large, &f64, 8);
    large = std::rotr(large, 60);
    boost::endian::native_to_big_inplace(large);
    memcpy(bytes.data(), &large, 8);
    auto tag = bytes[7] & kTagMask;

    switch (tag) {
        case kDoubleWordSelf1Tag:
        case kDoubleWordSelf2Tag:
            return TaggedValue(TaggedRepresentation::LargeValue, bytes);
        default:
            return {};
    }
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromPointer(void *ptr, tu_uint8 pointertag)
{
    TU_ASSERT ((pointertag & ~kPointerMask) == 0);
    if (ptr == nullptr)
        return {};
    std::array<tu_uint8,8> bytes;
    auto pointer = (std::uintptr_t) ptr;
    boost::endian::native_to_big_inplace(pointer);
    memcpy(bytes.data(), &pointer, 8);
    if (bytes[0] & kPointerMask)
        return {};
    if (bytes[7] & kTagMask)
        return {};
    bytes[0] |= pointertag;
    bytes[7] |= kDoubleWordPointerTag;
    return TaggedValue(TaggedRepresentation::Pointer, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromRef(AbstractRef *ref)
{
    return fromPointer(ref, kPointerRefTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromBytes(BytesRef *bytes)
{
    return fromPointer(bytes, kPointerBytesTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromNamespace(NamespaceRef *ns)
{
    return fromPointer(ns, kPointerNamespaceTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromProtocol(ProtocolRef *protocol)
{
    return fromPointer(protocol, kPointerProtocolTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromRest(RestRef *rest)
{
    return fromPointer(rest, kPointerRestTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromStatus(StatusRef *status)
{
    return fromPointer(status, kPointerStatusTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromString(StringRef *str)
{
    return fromPointer(str, kPointerStringTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromDescriptor(DescriptorEntry *descriptor)
{
    return fromPointer(descriptor, kPointerDescTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::fromType(TypeEntry *type)
{
    return fromPointer(type, kPointerTypeTag);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::nil()
{
    std::array<tu_uint8,8> bytes;
    bytes[6] = kNilEnum;
    bytes[7] = kShortEnumTag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}

lyric_runtime::TaggedValue
lyric_runtime::TaggedValue::undef()
{
    std::array<tu_uint8,8> bytes;
    bytes[6] = kUndefEnum;
    bytes[7] = kShortEnumTag;
    return TaggedValue(TaggedRepresentation::SmallValue, bytes);
}