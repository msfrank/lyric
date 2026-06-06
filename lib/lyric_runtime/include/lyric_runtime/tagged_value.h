#ifndef LYRIC_RUNTIME_TAGGED_VALUE_H
#define LYRIC_RUNTIME_TAGGED_VALUE_H

#include <tempo_utils/integer_types.h>

#include "abstract_ref.h"
#include "data_cell.h"

namespace lyric_runtime {

    constexpr tu_uint32 kInvalidSmallValue      = 0x00000008;
    constexpr tu_uint64 kInvalidLargeValue      = 0x0000000000000007;

    enum class TaggedRepresentation {
        Invalid,
        SmallValue,
        LargeValue,
        Pointer,
    };

    class TaggedValue final {
    public:
        TaggedValue();

        bool isValid() const;

        TaggedRepresentation getRepresentation() const;
        DataCellType getType() const;

        bool isUndef() const;
        bool isNil() const;

        bool getBool(bool &b);
        bool getI8(tu_int8 &i8);
        bool getI16(tu_int16 &i16);
        bool getI32(tu_int32 &i32);
        bool getU8(tu_uint8 &u8);
        bool getU16(tu_uint16 &u16);
        bool getU32(tu_uint32 &u32);
        bool getC32(char32_t &c32);

        static TaggedValue fromBool(bool b);
        static TaggedValue fromI8(tu_int8 i8);
        static TaggedValue fromI16(tu_int16 i16);
        static TaggedValue fromI32(tu_int32 i32);
        static TaggedValue fromU8(tu_uint8 u8);
        static TaggedValue fromU16(tu_uint16 u16);
        static TaggedValue fromU32(tu_uint32 u32);
        static TaggedValue fromC32(char32_t c32);

        static TaggedValue nil();
        static TaggedValue undef();

    private:
        TaggedRepresentation m_repr;
        std::array<tu_uint8,8> m_bytes;

        TaggedValue(TaggedRepresentation repr, std::array<tu_uint8,8> bytes);
    };
}

#endif // LYRIC_RUNTIME_TAGGED_VALUE_H
