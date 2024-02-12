#ifndef LYRIC_OBJECT_BYTECODE_ITERATOR_H
#define LYRIC_OBJECT_BYTECODE_ITERATOR_H

#include <cstdint>

#include <unicode/umachine.h>

#include "object_types.h"

namespace lyric_object {

    enum class OpInfoType {
        NO_OPERANDS,
        ADDRESS_U32,
        FLAGS_U8_ADDRESS_U32,
        JUMP_I16,
        OFFSET_U16,
        FLAGS_U8_ADDRESS_U32_PLACEMENT_U16,
        FLAGS_U8_OFFSET_U16_PLACEMENT_U16,
        TYPE_U8,
        IMMEDIATE_I64,
        IMMEDIATE_DBL,
        IMMEDIATE_CHR,
    };

    struct OpInfo {
        Opcode opcode;
        OpInfoType type;
    };

    struct OpCell {
        Opcode opcode;
        tu_uint32 offset;
        OpInfoType type;
        union {
            struct {
                tu_uint32 address;
            } address_u32;
            struct {
                tu_uint8 flags;
                tu_uint32 address;
            } flags_u8_address_u32;
            struct {
                tu_int16 jump;
            } jump_i16;
            struct {
                tu_uint16 offset;
            } offset_u16;
            struct {
                tu_uint8 flags;
                tu_uint32 address;
                tu_uint16 placement;
            } flags_u8_address_u32_placement_u16;
            struct {
                tu_uint8 flags;
                tu_uint16 offset;
                tu_uint16 placement;
            } flags_u8_offset_u16_placement_u16;
            struct {
                tu_uint8 type;
            } type_u8;
            struct {
                int64_t i64;
            } immediate_i64;
            struct {
                double dbl;
            } immediate_dbl;
            struct {
                UChar32 chr;
            } immediate_chr;
        } operands;
    };

    class BytecodeIterator {

    public:
        BytecodeIterator();
        BytecodeIterator(const tu_uint8 *bytecode, size_t size);

        bool isValid() const;

        const tu_uint8 *getCurr() const;
        const tu_uint8 *getBase() const;
        const tu_uint8 *getCanary() const;
        tu_uint32 getSize() const;
        bool hasNext() const;

        bool getNext(OpCell &op);
        bool reset(tu_uint32 address);
        bool move(tu_int16 offset);

    private:
        const tu_uint8 *m_bytecode;
        tu_uint32 m_size;
        tu_uint32 m_curr;
    };

    constexpr tu_uint8 opinfo_type_to_size(OpInfoType type)
    {
        switch (type) {
            case OpInfoType::NO_OPERANDS:
                return 0;
            case OpInfoType::TYPE_U8:
                return 1;
            case OpInfoType::JUMP_I16:
            case OpInfoType::OFFSET_U16:
                return 2;
            case OpInfoType::ADDRESS_U32:
            case OpInfoType::IMMEDIATE_CHR:
                return 4;
            case OpInfoType::FLAGS_U8_OFFSET_U16_PLACEMENT_U16:
            case OpInfoType::FLAGS_U8_ADDRESS_U32:
                return 5;
            case OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16:
                return 7;
            case OpInfoType::IMMEDIATE_I64:
            case OpInfoType::IMMEDIATE_DBL:
                return 8;
        }
        TU_UNREACHABLE();
    }

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const BytecodeIterator &it);
}

#endif // LYRIC_OBJECT_BYTECODE_ITERATOR_H
