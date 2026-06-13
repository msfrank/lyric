#ifndef LYRIC_OBJECT_BYTECODE_ITERATOR_H
#define LYRIC_OBJECT_BYTECODE_ITERATOR_H

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
        IMMEDIATE_I8,
        IMMEDIATE_I16,
        IMMEDIATE_I32,
        IMMEDIATE_I64,
        IMMEDIATE_U8,
        IMMEDIATE_U16,
        IMMEDIATE_U32,
        IMMEDIATE_U64,
        IMMEDIATE_F32,
        IMMEDIATE_F64,
        IMMEDIATE_C32,
    };

    struct OpInfo {
        Opcode opcode;              /**< The Opcode id. */
        OpInfoType type;            /**< The Opcode type. */
    };

    struct OpCell final {
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
            struct { tu_int8   i8; } immediate_i8;
            struct { tu_int16 i16; } immediate_i16;
            struct { tu_int32 i32; } immediate_i32;
            struct { tu_int64 i64; } immediate_i64;
            struct { tu_uint8   u8;} immediate_u8;
            struct { tu_uint16 u16;} immediate_u16;
            struct { tu_uint32 u32;} immediate_u32;
            struct { tu_uint64 u64;} immediate_u64;
            struct { float    f32; } immediate_f32;
            struct { double   f64; } immediate_f64;
            struct { char32_t c32; } immediate_c32;
        } operands;
    };

    class BytecodeIterator {

    public:
        BytecodeIterator();
        explicit BytecodeIterator(std::span<const tu_uint8> bytecode);
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
            case OpInfoType::IMMEDIATE_I8:
            case OpInfoType::IMMEDIATE_U8:
                return 1;
            case OpInfoType::JUMP_I16:
            case OpInfoType::OFFSET_U16:
            case OpInfoType::IMMEDIATE_I16:
            case OpInfoType::IMMEDIATE_U16:
                return 2;
            case OpInfoType::ADDRESS_U32:
            case OpInfoType::IMMEDIATE_I32:
            case OpInfoType::IMMEDIATE_U32:
            case OpInfoType::IMMEDIATE_F32:
            case OpInfoType::IMMEDIATE_C32:
                return 4;
            case OpInfoType::FLAGS_U8_OFFSET_U16_PLACEMENT_U16:
            case OpInfoType::FLAGS_U8_ADDRESS_U32:
                return 5;
            case OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16:
                return 7;
            case OpInfoType::IMMEDIATE_I64:
            case OpInfoType::IMMEDIATE_U64:
            case OpInfoType::IMMEDIATE_F64:
                return 8;
        }
        TU_UNREACHABLE();
    }

    OpInfoType opcode_to_opinfo_type(Opcode opcode);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const BytecodeIterator &it);
}

#endif // LYRIC_OBJECT_BYTECODE_ITERATOR_H
