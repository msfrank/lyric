
#include <boost/endian.hpp>

#include <lyric_runtime/tagged_value.h>

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

constexpr tu_uint8 kSingleWordShortTag      = 0x00;
constexpr tu_uint8 kSingleWordI32Tag        = 0x01;
constexpr tu_uint8 kSingleWordU32Tag        = 0x02;

constexpr tu_uint8 kSingleWordI32SignBit    = 0x08;

constexpr tu_uint8 kDoubleWordSelf1Tag      = 0x03;
constexpr tu_uint8 kDoubleWordSelf2Tag      = 0x04;
constexpr tu_uint8 kDoubleWordNum32Tag      = 0x05;
constexpr tu_uint8 kDoubleWordNum64Tag      = 0x06;
//constexpr tu_uint8 kDoubleWordPointerTag    = 0x07;

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
            TU_UNREACHABLE();
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
    tu_uint8 tag = get_info_tag(m_bytes);
    if (tag != kShortI8Tag)
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
            if (info & kSingleWordI32SignBit) {
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
        bytes[7] |= kSingleWordI32SignBit;
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