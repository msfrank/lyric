
#include <bit>

#include <absl/strings/substitute.h>
#include <boost/endian.hpp>

#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/f64_ref.h>
#include <lyric_runtime/i64_ref.h>
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/namespace_ref.h>
#include <lyric_runtime/operand.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/rest_ref.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/u64_ref.h>
#include <tempo_utils/unicode.h>

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
 *   I64:       00010100 (20)
 *   U64:       00010110 (22)
 *   F64:       00011000 (24)
 *   Num:       00011010 (26)
 */

/*
 * Short byte tags:
 *   Invalid:   00000000 (0)
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
 *   Invalid:   00 00 00 00
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
constexpr tu_uint8 kPointerI64Tag           = 0x14;
constexpr tu_uint8 kPointerU64Tag           = 0x16;
constexpr tu_uint8 kPointerF64Tag           = 0x18;
constexpr tu_uint8 kPointerNumTag           = 0x1A;

constexpr tu_uint32 kUndefEnum              = 0x01;
constexpr tu_uint32 kNilEnum                = 0x02;
constexpr tu_uint32 kTrueEnum               = 0x03;
constexpr tu_uint32 kFalseEnum              = 0x04;
constexpr tu_uint32 kF64ZeroEnum            = 0x05;
constexpr tu_uint32 kF64NZeroEnum           = 0x06;

constexpr tu_int32 kMinI32StackValue        = -0x10000000;
constexpr tu_int32 kMaxI32StackValue        = 0x10000000;
constexpr tu_uint32 kMaxU32StackValue       = 0x1fffffff;
constexpr tu_int64 kMinI64StackValue        = -0x800000000000000;
constexpr tu_int64 kMaxI64StackValue        = 0x800000000000000;
constexpr tu_uint64 kMaxU64StackValue       = 0x0fffffffffffffff;
constexpr char32_t kMaxC32Value             = 0x10ffff;

inline tu_uint8 get_info_byte(const std::array<tu_uint8,8> &bytes) { return bytes[7]; }

inline tu_uint8 get_info_tag(const std::array<tu_uint8,8> &bytes) { return bytes[7] & kTagMask; }

lyric_runtime::Operand::Operand()
    : m_bytes{0,0,0,0,0,0,0,0}
{
}

lyric_runtime::Operand::Operand(std::array<tu_uint8,8> bytes)
    : m_bytes(bytes)
{
}

lyric_runtime::Operand::Operand(const Operand &other)
    : m_bytes(other.m_bytes)
{
}

lyric_runtime::Operand::Operand(Operand &&other) noexcept
    : m_bytes{std::exchange(other.m_bytes, {})}
{
}

lyric_runtime::Operand&
lyric_runtime::Operand::operator=(const Operand &other)
{
    m_bytes = other.m_bytes;
    return *this;
}

lyric_runtime::Operand&
lyric_runtime::Operand::operator=(Operand &&other) noexcept
{
    if (this != &other) {
        m_bytes.swap(other.m_bytes);
    }
    return *this;
}

bool
lyric_runtime::Operand::isValid() const
{
    return get_info_byte(m_bytes) != 0;
}

template<class ValueType>
bool values_equal(const lyric_runtime::Operand &lhs, const lyric_runtime::Operand &rhs)
{
    ValueType l, r;
    if (!operand_to_value(lhs, l))
        return false;
    if (!operand_to_value(rhs, r))
        return false;
    return l == r;
}

template<class RefType>
bool refs_equal(const lyric_runtime::Operand &lhs, const lyric_runtime::Operand &rhs)
{
    RefType *l, *r;
    if (!operand_to_pointer(lhs, l))
        return false;
    if (!operand_to_pointer(rhs, r))
        return false;
    return l->equals(r);
}

template<class PointerType>
bool pointers_same(const lyric_runtime::Operand &lhs, const lyric_runtime::Operand &rhs)
{
    PointerType *l, *r;
    if (!operand_to_pointer(lhs, l))
        return false;
    if (!operand_to_pointer(rhs, r))
        return false;
    return l == r;
}

bool
lyric_runtime::Operand::isEqualTo(const Operand &other) const
{
    if (getType() != other.getType())
        return false;

    switch (getType()) {
        case OperandType::Nil:
        case OperandType::Undef:
                return true;

        case OperandType::Bool:        return values_equal<bool>(*this, other);
        case OperandType::Int8:        return values_equal<tu_int8>(*this, other);
        case OperandType::Int16:       return values_equal<tu_int16>(*this, other);
        case OperandType::Int32:       return values_equal<tu_int32>(*this, other);
        case OperandType::Int64:       return values_equal<tu_int64>(*this, other);
        case OperandType::UInt8:       return values_equal<tu_uint8>(*this, other);
        case OperandType::UInt16:      return values_equal<tu_uint16>(*this, other);
        case OperandType::UInt32:      return values_equal<tu_uint32>(*this, other);
        case OperandType::UInt64:      return values_equal<tu_uint64>(*this, other);
        case OperandType::Float32:     return values_equal<float>(*this, other);
        case OperandType::Float64:     return values_equal<double>(*this, other);
        case OperandType::Char32:      return values_equal<char32_t>(*this, other);

        case OperandType::Bytes:       return refs_equal<BytesRef>(*this, other);
        case OperandType::String:      return refs_equal<StringRef>(*this, other);
        case OperandType::Status:      return refs_equal<StatusRef>(*this, other);
        case OperandType::Namespace:   return refs_equal<NamespaceRef>(*this, other);
        case OperandType::Protocol:    return refs_equal<ProtocolRef>(*this, other);
        case OperandType::Rest:        return refs_equal<RestRef>(*this, other);
        case OperandType::Ref:         return refs_equal<BaseRef>(*this, other);

        case OperandType::Descriptor: {
            DescriptorEntry *l, *r;
            if (!getDescriptor(l))
                return false;
            if (!other.getDescriptor(r))
                return false;
            return l->getSegmentIndex() == r->getSegmentIndex()
                && l->getDescriptorIndex() == r->getDescriptorIndex()
                && l->getLinkageSection() == r->getLinkageSection();
        }

        case OperandType::Type: {
            TypeEntry *l, *r;
            if (!getType(l))
                return false;
            if (!other.getType(r))
                return false;
            return l->getSegmentIndex() == r->getSegmentIndex()
                && l->getDescriptorIndex() == r->getDescriptorIndex();
        }

        default:
            return false;
    }
}

bool
lyric_runtime::Operand::isSameAs(const Operand &other) const
{
    if (getType() != other.getType())
        return false;

    switch (getType()) {
        case OperandType::Nil:
        case OperandType::Undef:
                return true;

        case OperandType::Bool:        return values_equal<bool>(*this, other);
        case OperandType::Int8:        return values_equal<tu_int8>(*this, other);
        case OperandType::Int16:       return values_equal<tu_int16>(*this, other);
        case OperandType::Int32:       return values_equal<tu_int32>(*this, other);
        case OperandType::Int64:       return values_equal<tu_int64>(*this, other);
        case OperandType::UInt8:       return values_equal<tu_uint8>(*this, other);
        case OperandType::UInt16:      return values_equal<tu_uint16>(*this, other);
        case OperandType::UInt32:      return values_equal<tu_uint32>(*this, other);
        case OperandType::UInt64:      return values_equal<tu_uint64>(*this, other);
        case OperandType::Float32:     return values_equal<float>(*this, other);
        case OperandType::Float64:     return values_equal<double>(*this, other);
        case OperandType::Char32:      return values_equal<char32_t>(*this, other);

        case OperandType::Bytes:       return pointers_same<BytesRef>(*this, other);
        case OperandType::String:      return pointers_same<StringRef>(*this, other);
        case OperandType::Status:      return pointers_same<StatusRef>(*this, other);
        case OperandType::Namespace:   return pointers_same<NamespaceRef>(*this, other);
        case OperandType::Protocol:    return pointers_same<ProtocolRef>(*this, other);
        case OperandType::Rest:        return pointers_same<RestRef>(*this, other);
        case OperandType::Ref:         return pointers_same<BaseRef>(*this, other);
        case OperandType::Descriptor:  return pointers_same<DescriptorEntry>(*this, other);
        case OperandType::Type:        return pointers_same<TypeEntry>(*this, other);

        default:
            return false;
    }
}

lyric_runtime::OverlayType
lyric_runtime::Operand::getOverlay() const
{
    tu_uint8 info = get_info_byte(m_bytes);
    if (info == 0)
        return OverlayType::Invalid;

    tu_uint8 tag = info & kTagMask;
    switch (tag) {
        case kSingleWordShortTag:
        case kSingleWordI32Tag:
        case kSingleWordU32Tag:
            return OverlayType::SmallValue;
        case kDoubleWordSelf1Tag:
        case kDoubleWordSelf2Tag:
        case kDoubleWordNum32Tag:
        case kDoubleWordNum64Tag:
            return OverlayType::LargeValue;
        case kDoubleWordPointerTag:
            return OverlayType::Pointer;
        default:
            return OverlayType::Invalid;
    }
}

size_t
lyric_runtime::Operand::getSize() const
{
    switch (getOverlay()) {
        case OverlayType::SmallValue:
            return 4;
        case OverlayType::LargeValue:
        case OverlayType::Pointer:
            return 8;
        default:
            return 0;
    }
}

std::span<const tu_uint8>
lyric_runtime::Operand::getBytes() const
{
    switch (getOverlay()) {
        case OverlayType::SmallValue:
            return std::span(m_bytes.data() + 4, 4);
        case OverlayType::LargeValue:
        case OverlayType::Pointer:
            return std::span(m_bytes.data(), m_bytes.size());
        default:
            return {};
    }
}

lyric_runtime::OperandType
lyric_runtime::Operand::getType() const
{
    tu_uint8 info = get_info_byte(m_bytes);
    switch (getOverlay()) {
        case OverlayType::SmallValue: {
            tu_uint8 tag = info & kTagMask;
            switch (tag) {
                case kSingleWordI32Tag:
                    return OperandType::Int32;
                case kSingleWordU32Tag:
                    return OperandType::UInt32;
                case kSingleWordShortTag: {
                    const tu_uint8 shorttag = info;
                    switch (shorttag) {
                        case kShortI8Tag:
                            return OperandType::Int8;
                        case kShortU8Tag:
                            return OperandType::UInt8;
                        case kShortI16Tag:
                            return OperandType::Int16;
                        case kShortU16Tag:
                            return OperandType::UInt16;
                        case kShortC21Tag:
                            return OperandType::Char32;
                        case kShortEnumTag: {
                            tu_uint8 enumtag = m_bytes[6];
                            switch (enumtag) {
                                case kUndefEnum:
                                    return OperandType::Undef;
                                case kNilEnum:
                                    return OperandType::Nil;
                                case kTrueEnum:
                                case kFalseEnum:
                                    return OperandType::Bool;
                                case kF64ZeroEnum:
                                case kF64NZeroEnum:
                                    return OperandType::Float64;
                                default:
                                    return OperandType::Invalid;
                            }
                        }
                        default:
                            return OperandType::Invalid;
                    }
                }
                default:
                    return OperandType::Invalid;
            }
        }

        case OverlayType::LargeValue: {
            tu_uint8 tag = info & kTagMask;
            switch (tag) {
                case kDoubleWordSelf1Tag:
                case kDoubleWordSelf2Tag:
                    return OperandType::Float64;
                case kDoubleWordNum32Tag: {
                    tu_uint8 num32tag = info;
                    switch (num32tag) {
                        case kNum32SignedTag:
                            return OperandType::Int32;
                        case kNum32UnsignedTag:
                            return OperandType::UInt32;
                        case kNum32FloatTag:
                            return OperandType::Float32;
                        default:
                            return OperandType::Invalid;
                    }

                }
                case kDoubleWordNum64Tag: {
                    tu_uint8 num64tag = info & 0x0f;
                    switch (num64tag) {
                        case kNum64SignedTag:
                            return OperandType::Int64;
                        case kNum64UnsignedTag:
                            return OperandType::UInt64;
                        default:
                            return OperandType::Invalid;
                    }
                }
                default:
                    return OperandType::Invalid;
            }
        }

        case OverlayType::Pointer: {
            tu_uint8 tag = info & kTagMask;
            if (tag != kDoubleWordPointerTag)
                return OperandType::Invalid;
            tu_uint8 pointertag = m_bytes[0] & kPointerMask;
            switch (pointertag) {
                case kPointerRefTag:
                    return OperandType::Ref;
                case kPointerBytesTag:
                    return OperandType::Bytes;
                case kPointerNamespaceTag:
                    return OperandType::Namespace;
                case kPointerProtocolTag:
                    return OperandType::Protocol;
                case kPointerRestTag:
                    return OperandType::Rest;
                case kPointerStatusTag:
                    return OperandType::Status;
                case kPointerStringTag:
                    return OperandType::String;
                case kPointerDescTag:
                    return OperandType::Descriptor;
                case kPointerTypeTag:
                    return OperandType::Type;
                case kPointerI64Tag:
                    return OperandType::Int64;
                case kPointerU64Tag:
                    return OperandType::UInt64;
                case kPointerF64Tag:
                    return OperandType::Float64;
                case kPointerNumTag:
                    return OperandType::Invalid;
                default:
                    return OperandType::Invalid;
            }
        }

        default:
            return OperandType::Invalid;
    }
}

inline tu_uint8 get_enum_byte(const std::array<tu_uint8,8> &bytes)
{
    if (get_info_byte(bytes) != kShortEnumTag)
        return 0;
    return bytes[6];
}

bool
lyric_runtime::Operand::isUndef() const
{
    if (getOverlay() != OverlayType::SmallValue)
        return false;
    return get_enum_byte(m_bytes) == kUndefEnum;
}

bool
lyric_runtime::Operand::isNil() const
{
    if (getOverlay() != OverlayType::SmallValue)
        return false;
    return get_enum_byte(m_bytes) == kNilEnum;
}

bool
lyric_runtime::Operand::isIntegral() const
{
    switch (getType()) {
        case OperandType::UInt8:
        case OperandType::UInt16:
        case OperandType::UInt32:
        case OperandType::UInt64:
        case OperandType::Int8:
        case OperandType::Int16:
        case OperandType::Int32:
        case OperandType::Int64:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::Operand::isRational() const
{
    switch (getType()) {
        case OperandType::Float32:
        case OperandType::Float64:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::Operand::isSigned() const
{
    switch (getType()) {
        case OperandType::Int8:
        case OperandType::Int16:
        case OperandType::Int32:
        case OperandType::Int64:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::Operand::isUnsigned() const
{
    switch (getType()) {
        case OperandType::UInt8:
        case OperandType::UInt16:
        case OperandType::UInt32:
        case OperandType::UInt64:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::Operand::isReference() const
{
    switch (getType()) {
        case OperandType::Bytes:
        case OperandType::Namespace:
        case OperandType::Protocol:
        case OperandType::Ref:
        case OperandType::Rest:
        case OperandType::Status:
        case OperandType::String:
            return true;
        default:
            return false;
    }
}

bool
lyric_runtime::Operand::getBool(bool &b) const
{
    if (getOverlay() != OverlayType::SmallValue)
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
lyric_runtime::Operand::getI8(tu_int8 &i8) const
{
    if (getOverlay() != OverlayType::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortI8Tag)
        return false;
    i8 = static_cast<tu_int8>(m_bytes[6]);
    return true;
}

bool
lyric_runtime::Operand::getU8(tu_uint8 &u8) const
{
    if (getOverlay() != OverlayType::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortU8Tag)
        return false;
    u8 = m_bytes[6];
    return true;
}

bool
lyric_runtime::Operand::getI16(tu_int16 &i16) const
{
    if (getOverlay() != OverlayType::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortI16Tag)
        return false;
    i16 = boost::endian::endian_load<tu_int16,2,boost::endian::order::native>(m_bytes.data() + 5);
    return true;
}

bool
lyric_runtime::Operand::getU16(tu_uint16 &u16) const
{
    if (getOverlay() != OverlayType::SmallValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kShortU16Tag)
        return false;
    u16 = boost::endian::endian_load<tu_uint16,2,boost::endian::order::native>(m_bytes.data() + 5);
    return true;
}

bool
lyric_runtime::Operand::getI32(tu_int32 &i32) const
{
    switch (getOverlay()) {
        case OverlayType::SmallValue: {
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
        case OverlayType::LargeValue: {
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
lyric_runtime::Operand::getU32(tu_uint32 &u32) const
{
    switch (getOverlay()) {
        case OverlayType::SmallValue: {
            tu_uint8 tag = get_info_tag(m_bytes);
            if (tag != kSingleWordU32Tag)
                return false;
            auto *ptr = (tu_uint32 *) &m_bytes[4];
            u32 = *ptr;
            boost::endian::big_to_native_inplace(u32);
            u32 >>= 3;
            return true;
        }
        case OverlayType::LargeValue: {
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
lyric_runtime::Operand::getI64(tu_int64 &i64) const
{
    tu_uint8 info = get_info_byte(m_bytes);
    if ((info & kTagMask) == kDoubleWordPointerTag) {
        auto *ptr = (I64Ref *) getPointer(kPointerI64Tag);
        if (ptr == nullptr)
            return false;
        i64 = ptr->getI64();
        return true;
    }
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
lyric_runtime::Operand::getU64(tu_uint64 &u64) const
{
    tu_uint8 info = get_info_byte(m_bytes);
    if ((info & kTagMask) == kDoubleWordPointerTag) {
        auto *ptr = (U64Ref *) getPointer(kPointerU64Tag);
        if (ptr == nullptr)
            return false;
        u64 = ptr->getU64();
        return true;
    }
    if ((info & kNum64Mask) != kNum64UnsignedTag)
        return false;
    auto *ptr = (tu_uint64 *) m_bytes.data();
    u64 = *ptr;
    boost::endian::big_to_native_inplace(u64);
    u64 >>= 4;
    return true;
}

bool
lyric_runtime::Operand::getC32(char32_t &c32) const
{
    if (getOverlay() != OverlayType::SmallValue)
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
lyric_runtime::Operand::getF32(float &f32) const
{
    if (getOverlay() != OverlayType::LargeValue)
        return false;
    tu_uint8 info = get_info_byte(m_bytes);
    if (info != kNum32FloatTag)
        return false;
    auto *ptr = (float *) &m_bytes[3];
    f32 = *ptr;
    return true;
}

bool
lyric_runtime::Operand::getF64(double &f64) const
{
    switch (getOverlay()) {
        case OverlayType::SmallValue: {
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

        case OverlayType::LargeValue: {
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

        case OverlayType::Pointer: {
            auto *ptr = (F64Ref *) getPointer(kPointerF64Tag);
            if (ptr == nullptr)
                return false;
            f64 = ptr->getF64();
            return true;
        }

        default:
            return false;
    }
}

void *
lyric_runtime::Operand::getPointer(tu_uint8 pointertag) const
{
    TU_ASSERT ((pointertag & ~kPointerMask) == 0);
    if (getOverlay() != OverlayType::Pointer)
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
lyric_runtime::Operand::getRef(BaseRef *&ref) const
{
    auto *ptr = getPointer(kPointerRefTag);
    if (ptr == nullptr)
        return false;
    ref = (BaseRef *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getBytes(BytesRef *&bytes) const
{
    auto *ptr = getPointer(kPointerBytesTag);
    if (ptr == nullptr)
        return false;
    bytes = (BytesRef *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getNamespace(NamespaceRef *&ns) const
{
    auto *ptr = getPointer(kPointerNamespaceTag);
    if (ptr == nullptr)
        return false;
    ns = (NamespaceRef *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getProtocol(ProtocolRef *&protocol) const
{
    auto *ptr = getPointer(kPointerProtocolTag);
    if (ptr == nullptr)
        return false;
    protocol = (ProtocolRef *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getRest(RestRef *&rest) const
{
    auto *ptr = getPointer(kPointerRestTag);
    if (ptr == nullptr)
        return false;
    rest = (RestRef *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getStatus(StatusRef *&status) const
{
    auto *ptr = getPointer(kPointerStatusTag);
    if (ptr == nullptr)
        return false;
    status = (StatusRef *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getString(StringRef *&string) const
{
    auto *ptr = getPointer(kPointerStringTag);
    if (ptr == nullptr)
        return false;
    string = (StringRef *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getDescriptor(DescriptorEntry *&descriptor) const
{
    auto *ptr = getPointer(kPointerDescTag);
    if (ptr == nullptr)
        return false;
    descriptor = (DescriptorEntry *) ptr;
    return true;
}

bool
lyric_runtime::Operand::getDescriptor(DescriptorEntry *&descriptor, lyric_object::LinkageSection section) const
{
    TU_ASSERT (section != lyric_object::LinkageSection::Invalid);
    auto *ptr = getPointer(kPointerDescTag);
    if (ptr == nullptr)
        return false;
    descriptor = (DescriptorEntry *) ptr;
    if (descriptor->getLinkageSection() != section)
        return false;
    return true;
}

bool
lyric_runtime::Operand::getType(TypeEntry *&type) const
{
    auto *ptr = getPointer(kPointerTypeTag);
    if (ptr == nullptr)
        return false;
    type = (TypeEntry *) ptr;
    return true;
}

lyric_runtime::Operand
lyric_runtime::Operand::fromBool(bool b)
{
    std::array<tu_uint8,8> bytes;
    bytes[6] = b? kTrueEnum : kFalseEnum;
    bytes[7] = kShortEnumTag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromI8(tu_int8 i8)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_int8,1,boost::endian::order::native>(bytes.data() + 6, i8);
    bytes[7] = kShortI8Tag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromU8(tu_uint8 u8)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_uint8,1,boost::endian::order::native>(bytes.data() + 6, u8);
    bytes[7] = kShortU8Tag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromI16(tu_int16 i16)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_int16,2,boost::endian::order::native>(bytes.data() + 5, i16);
    bytes[7] = kShortI16Tag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromU16(tu_uint16 u16)
{
    std::array<tu_uint8,8> bytes;
    boost::endian::endian_store<tu_uint16,2,boost::endian::order::native>(bytes.data() + 5, u16);
    bytes[7] = kShortU16Tag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromI32(tu_int32 i32)
{
    std::array<tu_uint8,8> bytes;
    if (i32 < kMinI32StackValue || i32 > kMaxI32StackValue) [[unlikely]] {
        boost::endian::endian_store<tu_int32,4,boost::endian::order::native>(bytes.data() + 3, i32);
        bytes[7] = kNum32SignedTag;
        return Operand(bytes);
    }
    tu_uint32 small = std::abs(i32);
    small = boost::endian::native_to_big(small << 4);
    memcpy(bytes.data() + 4, &small, 4);
    bytes[7] |= kSingleWordI32Tag;
    if (i32 < 0) {
        bytes[7] |= kI32SignBit;
    }
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromU32(tu_uint32 u32)
{
    std::array<tu_uint8,8> bytes;
    if (u32 > kMaxU32StackValue) [[unlikely]] {
        boost::endian::endian_store<tu_uint32,4,boost::endian::order::native>(bytes.data() + 3, u32);
        bytes[7] = kNum32UnsignedTag;
        return Operand(bytes);
    }
    tu_uint32 small = boost::endian::native_to_big(u32 << 3);
    memcpy(bytes.data() + 4, &small, 4);
    bytes[7] |= kSingleWordU32Tag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromI64(tu_int64 i64)
{
    if (i64 < kMinI64StackValue || i64 > kMaxI64StackValue)
        return {};
    std::array<tu_uint8,8> bytes;
    tu_uint64 large = std::abs(i64);
    large = boost::endian::native_to_big(large << 5);
    memcpy(bytes.data(), &large, 8);
    bytes[7] |= kNum64SignedTag;
    if (i64 < 0) {
        bytes[7] |= kI64SignBit;
    }
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromI64(I64Ref *i64)
{
    return fromPointer(i64, kPointerI64Tag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromU64(tu_uint64 u64)
{
    if (u64 > kMaxU64StackValue)
        return {};
    std::array<tu_uint8,8> bytes;
    tu_uint64 large = boost::endian::native_to_big(u64 << 4);
    memcpy(bytes.data(), &large, 8);
    bytes[7] |= kNum64UnsignedTag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromU64(U64Ref *u64)
{
    return fromPointer(u64, kPointerU64Tag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromC32(char32_t c32)
{
    if (c32 > kMaxC32Value)
        return {};
    std::array<tu_uint8,8> bytes;
    tu_uint32 small = boost::endian::native_to_big(c32 << 8);
    memcpy(bytes.data() + 4, &small, 4);
    bytes[7] = kShortC21Tag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromF32(float f32)
{
    std::array<tu_uint8,8> bytes;
    memcpy(bytes.data() + 3, &f32, 4);
    bytes[7] = kNum32FloatTag;
    return Operand(bytes);
}

inline bool encode_f64(double f64, std::array<tu_uint8,8> &bytes)
{
    tu_uint64 large;
    memcpy(&large, &f64, 8);
    large = std::rotr(large, 60);
    boost::endian::native_to_big_inplace(large);
    memcpy(bytes.data(), &large, 8);
    auto tag = bytes[7] & kTagMask;

    switch (tag) {
        case kDoubleWordSelf1Tag:
        case kDoubleWordSelf2Tag:
            return true;
        default:
            return false;
    }
}

lyric_runtime::Operand
lyric_runtime::Operand::fromF64(double f64)
{
    std::array<tu_uint8,8> bytes;
    if (f64 == 0.0) [[unlikely]] {
        bytes[6] = std::signbit(f64)? kF64NZeroEnum : kF64ZeroEnum;
        bytes[7] = kShortEnumTag;
        return Operand(bytes);
    }

    if (encode_f64(f64, bytes))
        return Operand(bytes);
    return {};
}

lyric_runtime::Operand
lyric_runtime::Operand::fromF64(F64Ref *f64)
{
    return fromPointer(f64, kPointerF64Tag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromPointer(void *ptr, tu_uint8 pointertag)
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
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromRef(BaseRef *ref)
{
    return fromPointer(ref, kPointerRefTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromBytes(BytesRef *bytes)
{
    return fromPointer(bytes, kPointerBytesTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromNamespace(NamespaceRef *ns)
{
    return fromPointer(ns, kPointerNamespaceTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromProtocol(ProtocolRef *protocol)
{
    return fromPointer(protocol, kPointerProtocolTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromRest(RestRef *rest)
{
    return fromPointer(rest, kPointerRestTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromStatus(StatusRef *status)
{
    return fromPointer(status, kPointerStatusTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromString(StringRef *str)
{
    return fromPointer(str, kPointerStringTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromDescriptor(DescriptorEntry *descriptor)
{
    return fromPointer(descriptor, kPointerDescTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::fromType(TypeEntry *type)
{
    return fromPointer(type, kPointerTypeTag);
}

lyric_runtime::Operand
lyric_runtime::Operand::nil()
{
    std::array<tu_uint8,8> bytes;
    bytes[6] = kNilEnum;
    bytes[7] = kShortEnumTag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::undef()
{
    std::array<tu_uint8,8> bytes;
    bytes[6] = kUndefEnum;
    bytes[7] = kShortEnumTag;
    return Operand(bytes);
}

lyric_runtime::Operand
lyric_runtime::Operand::parse(std::span<const tu_uint8> raw)
{
    std::array<tu_uint8,8> bytes;

    switch (raw.size()) {
        case 4: {
            tu_uint8 tag = raw[3] & kTagMask;
            switch (tag) {
                case kSingleWordShortTag:
                case kSingleWordI32Tag:
                case kSingleWordU32Tag:
                    memcpy(bytes.data() + 4, raw.data(), 4);
                    return Operand(bytes);
                default:
                    return {};
            }
        }
        case 8: {
            tu_uint8 tag = raw[7] & kTagMask;
            switch (tag) {
                case kSingleWordShortTag:
                case kSingleWordI32Tag:
                case kSingleWordU32Tag:
                    memcpy(bytes.data() + 4, raw.data() + 4, 4);
                    return Operand(bytes);
                case kDoubleWordSelf1Tag:
                case kDoubleWordSelf2Tag:
                case kDoubleWordNum32Tag:
                case kDoubleWordNum64Tag:
                    memcpy(bytes.data(), raw.data(), 8);
                    return Operand(bytes);
                case kDoubleWordPointerTag:
                    memcpy(bytes.data(), raw.data(), 8);
                    return Operand(bytes);
                default:
                    return {};
            }
        }
        default:
            return {};
    }
}

lyric_runtime::OverlayType
lyric_runtime::Operand::parseRepresentation(const tu_uint8 &infoByte)
{
    tu_uint8 tag = infoByte & kTagMask;
    switch (tag) {
        case kSingleWordShortTag:
        case kSingleWordI32Tag:
        case kSingleWordU32Tag:
            return OverlayType::SmallValue;
        case kDoubleWordSelf1Tag:
        case kDoubleWordSelf2Tag:
        case kDoubleWordNum32Tag:
        case kDoubleWordNum64Tag:
            return OverlayType::LargeValue;
        case kDoubleWordPointerTag:
            return OverlayType::Pointer;
        default:
            return OverlayType::Invalid;
    }
}

size_t
lyric_runtime::Operand::parseSize(const tu_uint8 &infoByte)
{
    tu_uint8 tag = infoByte & kTagMask;
    switch (tag) {
        case kSingleWordShortTag:
        case kSingleWordI32Tag:
        case kSingleWordU32Tag:
            return 4;
        case kDoubleWordSelf1Tag:
        case kDoubleWordSelf2Tag:
        case kDoubleWordNum32Tag:
        case kDoubleWordNum64Tag:
        case kDoubleWordPointerTag:
            return 8;
        default:
            return 0;
    }
}

inline lyric_runtime::AbstractRef *get_ref(const lyric_runtime::Operand &operand)
{
    switch (operand.getType()) {
        case lyric_runtime::OperandType::Bytes: {
            lyric_runtime::BytesRef *bytes;
            TU_ASSERT (operand.getBytes(bytes));
            return bytes;
        }
        case lyric_runtime::OperandType::String: {
            lyric_runtime::StringRef *str;
            TU_ASSERT (operand.getString(str));
            return str;
        }
        case lyric_runtime::OperandType::Status: {
            lyric_runtime::StatusRef *status;
            TU_ASSERT (operand.getStatus(status));
            return status;
        }
        case lyric_runtime::OperandType::Namespace: {
            lyric_runtime::NamespaceRef *ns;
            TU_ASSERT (operand.getNamespace(ns));
            return ns;
        }
        case lyric_runtime::OperandType::Protocol: {
            lyric_runtime::ProtocolRef *protocol;
            TU_ASSERT (operand.getProtocol(protocol));
            return protocol;
        }
        case lyric_runtime::OperandType::Rest: {
            lyric_runtime::RestRef *rest;
            TU_ASSERT (operand.getRest(rest));
            return rest;
        }
        case lyric_runtime::OperandType::Ref: {
            lyric_runtime::BaseRef *ref;
            TU_ASSERT (operand.getRef(ref));
            ref->setReachable();
            return ref;
        }
        default:
            return nullptr;
    }
}

void
lyric_runtime::Operand::hashEquality(absl::HashState state) const
{
    switch (getOverlay()) {
        case OverlayType::Pointer:
            break;
        case OverlayType::SmallValue:
            absl::HashState::combine_contiguous(std::move(state), m_bytes.data() + 4, 4);
            return;
        case OverlayType::LargeValue:
        case OverlayType::Invalid:
            absl::HashState::combine_contiguous(std::move(state), m_bytes.data(), 8);
            return;
    }

    switch (getType()) {
        case OperandType::Bytes:
        case OperandType::String:
        case OperandType::Status:
        case OperandType::Namespace:
        case OperandType::Protocol:
        case OperandType::Rest:
        case OperandType::Ref: {
            AbstractRef *ref = get_ref(*this);
            TU_NOTNULL (ref);
            ref->hashValue(std::move(state));
            return;
        }

        case OperandType::Descriptor: {
            DescriptorEntry *descriptor;
            TU_ASSERT (getDescriptor(descriptor));
            absl::HashState::combine(std::move(state),
                descriptor->getSegmentIndex(),
                descriptor->getLinkageSection(),
                descriptor->getDescriptorIndex());
            return;
        }
        case OperandType::Type: {
            TypeEntry *type;
            TU_ASSERT (getType(type));
            absl::HashState::combine(std::move(state),
                type->getSegmentIndex(),
                type->getDescriptorIndex());
            return;
        }

        default:
            break;
    }

    TU_UNREACHABLE();
}

void
lyric_runtime::Operand::hashIdentity(absl::HashState state) const
{
    switch (getOverlay()) {
        case OverlayType::SmallValue:
            absl::HashState::combine_contiguous(std::move(state), m_bytes.data() + 4, 4);
            break;
        case OverlayType::LargeValue:
        case OverlayType::Pointer:
            absl::HashState::combine_contiguous(std::move(state), m_bytes.data(), 8);
            break;
        case OverlayType::Invalid:
            break;
    }
}

std::string
lyric_runtime::Operand::toString() const
{
    switch (getType()) {
        case OperandType::Nil:
            return "nil";
        case OperandType::Undef:
            return "undef";
        case OperandType::Bool: {
            bool b;
            TU_ASSERT (getBool(b));
            return b ? "true" : "false";
        }
        case OperandType::Int8: {
            tu_int8 i8;
            TU_ASSERT (getI8(i8));
            return absl::StrCat(i8);
            break;
        }
        case OperandType::Int16: {
            tu_int16 i16;
            TU_ASSERT (getI16(i16));
            return absl::StrCat(i16);
        }
        case OperandType::Int32: {
            tu_int32 i32;
            TU_ASSERT (getI32(i32));
            return absl::StrCat(i32);
        }
        case OperandType::Int64: {
            tu_int64 i64;
            TU_ASSERT (getI64(i64));
            return absl::StrCat(i64);
        }
        case OperandType::UInt8: {
            tu_uint8 u8;
            TU_ASSERT (getU8(u8));
            return absl::StrCat(u8);
        }
        case OperandType::UInt16: {
            tu_uint16 u16;
            TU_ASSERT (getU16(u16));
            return absl::StrCat(u16);
        }
        case OperandType::UInt32: {
            tu_uint32 u32;
            TU_ASSERT (getU32(u32));
            return absl::StrCat(u32);
        }
        case OperandType::UInt64: {
            tu_uint64 u64;
            TU_ASSERT (getU64(u64));
            return absl::StrCat(u64);
        }
        case OperandType::Float32: {
            float f32;
            TU_ASSERT (getF32(f32));
            return absl::StrCat(f32);
        }
        case OperandType::Float64: {
            double f64;
            TU_ASSERT (getF64(f64));
            return absl::StrCat(f64);
        }
        case OperandType::Char32: {
            char32_t chr;
            TU_ASSERT (getC32(chr));
            return tempo_utils::convert_to_utf8(chr);
        }
        case OperandType::Bytes: {
            BytesRef *bytes;
            TU_ASSERT (getBytes(bytes));
            return bytes->toString();
        }
        case OperandType::String: {
            StringRef *str;
            TU_ASSERT (getString(str));
            return str->toString();
        }
        case OperandType::Status: {
            StatusRef *status;
            TU_ASSERT (getStatus(status));
            return status->toString();
        }
        case OperandType::Namespace: {
            NamespaceRef *ns;
            TU_ASSERT (getNamespace(ns));
            return ns->toString();
        }
        case OperandType::Protocol: {
            ProtocolRef *protocol;
            TU_ASSERT (getProtocol(protocol));
            return protocol->toString();
        }
        case OperandType::Rest: {
            RestRef *rest;
            TU_ASSERT (getRest(rest));
            return rest->toString();
        }
        case OperandType::Ref: {
            BaseRef *ref;
            TU_ASSERT (getRef(ref));
            return ref->toString();
        }
        case OperandType::Descriptor: {
            DescriptorEntry *descriptor;
            TU_ASSERT (getDescriptor(descriptor));
            return absl::Substitute("descriptor(object=$0, section=$1, offset=$2)",
                descriptor->getSegmentIndex(),
                lyric_object::linkage_section_to_name(descriptor->getLinkageSection()),
                descriptor->getDescriptorIndex());
        }
        case OperandType::Type: {
            TypeEntry *type;
            TU_ASSERT (getType(type));
            return absl::Substitute("type(object=$0, type=$1, offset=$2)",
                type->getSegmentIndex(),
                type->getTypeDef().toString(),
                type->getDescriptorIndex());
        }

        case OperandType::Invalid:
        default:
            return "???";
    }
}

void
lyric_runtime::Operand::setReachable() const
{
    auto *ref = get_ref(*this);
    if (ref != nullptr) {
        ref->setReachable();
    }
}

void
lyric_runtime::Operand::clearReachable() const
{
    auto *ref = get_ref(*this);
    if (ref != nullptr) {
        ref->clearReachable();
    }
}

bool
lyric_runtime::Operand::isReachable() const
{
    auto *ref = get_ref(*this);
    if (ref == nullptr)
        return false;
    return ref->isReachable();
}

tempo_utils::LogMessage&&
lyric_runtime::operator<<(tempo_utils::LogMessage &&message, const Operand &operand)
{
    auto s = operand.toString();

    switch (operand.getType()) {
        case OperandType::Nil:
            std::forward<tempo_utils::LogMessage>(message) << "Operand(nil)";
            break;
        case OperandType::Undef:
            std::forward<tempo_utils::LogMessage>(message) << "Operand(undef)";
            break;
        case OperandType::Bool:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(bool=", s, ")");
            break;
        case OperandType::Int8:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(i8=", s, ")");
            break;
        case OperandType::Int16:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(i16=", s, ")");
            break;
        case OperandType::Int32:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(i32=", s, ")");
            break;
        case OperandType::Int64:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(i64=", s, ")");
            break;
        case OperandType::UInt8:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(u8=", s, ")");
            break;
        case OperandType::UInt16:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(u16=", s, ")");
            break;
        case OperandType::UInt32:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(u32=", s, ")");
            break;
        case OperandType::UInt64:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(u64=", s, ")");
            break;
        case OperandType::Float32:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(f32=", s, ")");
            break;
        case OperandType::Float64:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(f64=", s, ")");
            break;
        case OperandType::Char32:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(c32=", s, ")");
            break;
        case OperandType::Bytes:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::String:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Status:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Namespace:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Protocol:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Rest:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Ref:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Descriptor:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Type:
            std::forward<tempo_utils::LogMessage>(message) << absl::StrCat("Operand(", s, ")");
            break;
        case OperandType::Invalid:
        default:
            std::forward<tempo_utils::LogMessage>(message) << "Operand(" << s << ")";
            break;

    }
    return std::move(message);
}

bool lyric_runtime::operand_to_value(const Operand &op, bool &v)
{
    return op.getBool(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_int8 &v)
{
    return op.getI8(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_int16 &v)
{
    return op.getI16(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_int32 &v)
{
    return op.getI32(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_int64 &v)
{
    return op.getI64(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_uint8 &v)
{
    return op.getU8(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_uint16 &v)
{
    return op.getU16(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_uint32 &v)
{
    return op.getU32(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, tu_uint64 &v)
{
    return op.getU64(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, float &v)
{
    return op.getF32(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, double &v)
{
    return op.getF64(v);
}

bool lyric_runtime::operand_to_value(const Operand &op, char32_t &v)
{
    return op.getC32(v);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, BaseRef *&r)
{
    return op.getRef(r);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, BytesRef *&r)
{
    return op.getBytes(r);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, NamespaceRef *&r)
{
    return op.getNamespace(r);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, ProtocolRef *&r)
{
    return op.getProtocol(r);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, RestRef *&r)
{
    return op.getRest(r);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, StatusRef *&r)
{
    return op.getStatus(r);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, StringRef *&r)
{
    return op.getString(r);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, DescriptorEntry *&e)
{
    return op.getDescriptor(e);
}

bool lyric_runtime::operand_to_pointer(const Operand &op, TypeEntry *&e)
{
    return op.getType(e);
}

void lyric_runtime::value_to_operand(bool v, Operand &op, HeapManager *)
{
    op = Operand::fromBool(v);
}

void lyric_runtime::value_to_operand(tu_int8 v, Operand &op, HeapManager *)
{
    op = Operand::fromI8(v);
}

void lyric_runtime::value_to_operand(tu_int16 v, Operand &op, HeapManager *)
{
    op = Operand::fromI16(v);
}

void lyric_runtime::value_to_operand(tu_int32 v, Operand &op, HeapManager *)
{
    op = Operand::fromI32(v);
}

void lyric_runtime::value_to_operand(tu_int64 v, Operand &op, HeapManager *heapManager)
{
    if (v < kMinI64StackValue || v > kMaxI64StackValue) [[unlikely]] {
        op = heapManager->allocateI64(v);
    } else {
        op = Operand::fromI64(v);
    }
}

void lyric_runtime::value_to_operand(tu_uint8 v, Operand &op, HeapManager *)
{
    op = Operand::fromU8(v);
}

void lyric_runtime::value_to_operand(tu_uint16 v, Operand &op, HeapManager *)
{
    op = Operand::fromU16(v);
}

void lyric_runtime::value_to_operand(tu_uint32 v, Operand &op, HeapManager *)
{
    op = Operand::fromU32(v);
}

void lyric_runtime::value_to_operand(tu_uint64 v, Operand &op, HeapManager *heapManager)
{
    if (v > kMaxU64StackValue) [[unlikely]] {
        op = heapManager->allocateU64(v);
    } else {
        op = Operand::fromU64(v);
    }
}

void lyric_runtime::value_to_operand(float v, Operand &op, HeapManager *)
{
    op = Operand::fromF32(v);
}

void lyric_runtime::value_to_operand(double v, Operand &op, HeapManager *heapManager)
{
    std::array<tu_uint8,8> bytes;
    if (!encode_f64(v, bytes)) {
        op = heapManager->allocateF64(v);
    } else {
        op = Operand(bytes);
    }
}

void lyric_runtime::value_to_operand(char32_t v, Operand &op, HeapManager *)
{
    op = Operand::fromC32(v);
}
