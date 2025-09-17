#include <absl/strings/substitute.h>

#include <lyric_object/bytecode_iterator.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/ieee754.h>
#include <tempo_utils/log_stream.h>

lyric_object::OpInfo ops[] = {
    { lyric_object::Opcode::OP_UNKNOWN,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_NOOP,                lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_UNDEF,               lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_NIL,                 lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_TRUE,                lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_FALSE,               lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_I64,                 lyric_object::OpInfoType::IMMEDIATE_I64 },
    { lyric_object::Opcode::OP_DBL,                 lyric_object::OpInfoType::IMMEDIATE_DBL },
    { lyric_object::Opcode::OP_CHR,                 lyric_object::OpInfoType::IMMEDIATE_CHR },
    { lyric_object::Opcode::OP_LITERAL,             lyric_object::OpInfoType::ADDRESS_U32 },
    { lyric_object::Opcode::OP_STRING,              lyric_object::OpInfoType::ADDRESS_U32 },
    { lyric_object::Opcode::OP_URL,                 lyric_object::OpInfoType::ADDRESS_U32 },
    { lyric_object::Opcode::OP_STATIC,              lyric_object::OpInfoType::ADDRESS_U32 },
    { lyric_object::Opcode::OP_SYNTHETIC,           lyric_object::OpInfoType::TYPE_U8 },
    { lyric_object::Opcode::OP_DESCRIPTOR,          lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32 },
    { lyric_object::Opcode::OP_LOAD,                lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32 },
    { lyric_object::Opcode::OP_STORE,               lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32 },
    { lyric_object::Opcode::OP_VA_LOAD,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_VA_SIZE,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_POP,                 lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_DUP,                 lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_PICK,                lyric_object::OpInfoType::OFFSET_U16 },
    { lyric_object::Opcode::OP_DROP,                lyric_object::OpInfoType::OFFSET_U16 },
    { lyric_object::Opcode::OP_RPICK,               lyric_object::OpInfoType::OFFSET_U16 },
    { lyric_object::Opcode::OP_RDROP,               lyric_object::OpInfoType::OFFSET_U16 },
    { lyric_object::Opcode::OP_I64_ADD,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_I64_SUB,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_I64_MUL,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_I64_DIV,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_I64_NEG,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_DBL_ADD,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_DBL_SUB,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_DBL_MUL,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_DBL_DIV,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_DBL_NEG,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_BOOL_CMP,            lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_I64_CMP,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_DBL_CMP,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_CHR_CMP,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_TYPE_CMP,            lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_LOGICAL_AND,         lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_LOGICAL_OR,          lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_LOGICAL_NOT,         lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_BITWISE_AND,         lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_BITWISE_OR,          lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_BITWISE_XOR,         lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_BITWISE_RIGHT_SHIFT, lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_BITWISE_LEFT_SHIFT,  lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_IF_NIL,              lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_NOTNIL,           lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_TRUE,             lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_FALSE,            lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_ZERO,             lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_NOTZERO,          lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_GT,               lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_GE,               lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_LT,               lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IF_LE,               lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_JUMP,                lyric_object::OpInfoType::JUMP_I16 },
    { lyric_object::Opcode::OP_IMPORT,              lyric_object::OpInfoType::ADDRESS_U32 },
    { lyric_object::Opcode::OP_CALL_STATIC,         lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16 },
    { lyric_object::Opcode::OP_CALL_VIRTUAL,        lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16 },
    { lyric_object::Opcode::OP_CALL_CONCEPT,        lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16 },
    { lyric_object::Opcode::OP_CALL_EXISTENTIAL,    lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16 },
    { lyric_object::Opcode::OP_TRAP,                lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32 },
    { lyric_object::Opcode::OP_RETURN,              lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_NEW,                 lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16 },
    { lyric_object::Opcode::OP_TYPE_OF,             lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_INTERRUPT,           lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_HALT,                lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::OP_ABORT,               lyric_object::OpInfoType::NO_OPERANDS },
    { lyric_object::Opcode::LAST_,                  lyric_object::OpInfoType::NO_OPERANDS },
};

lyric_object::OpInfoType
lyric_object::opcode_to_opinfo_type(Opcode opcode)
{
    auto opinfo = ops[static_cast<tu_uint8>(opcode)];
    return opinfo.type;
}

lyric_object::BytecodeIterator::BytecodeIterator()
    : m_bytecode(nullptr), m_size(0), m_curr(0)
{
}

lyric_object::BytecodeIterator::BytecodeIterator(const tu_uint8 *bytecode, size_t size)
    : m_bytecode(bytecode), m_size(size), m_curr(0)
{
    TU_ASSERT (bytecode != nullptr);
}

bool
lyric_object::BytecodeIterator::isValid() const
{
    return m_bytecode != nullptr;
}

const tu_uint8 *
lyric_object::BytecodeIterator::getCurr() const
{
    return (const tu_uint8 *) m_bytecode + m_curr;
}

const tu_uint8 *
lyric_object::BytecodeIterator::getBase() const
{
    return m_bytecode;
}

const tu_uint8 *
lyric_object::BytecodeIterator::getCanary() const
{
    return (const tu_uint8 *) m_bytecode + m_size;
}

tu_uint32
lyric_object::BytecodeIterator::getSize() const
{
    return m_size;
}

bool
lyric_object::BytecodeIterator::hasNext() const
{
    return m_curr < m_size;
}

bool
lyric_object::BytecodeIterator::getNext(lyric_object::OpCell &op)
{
    // we reached the end of the bytecode
    if (m_size <= m_curr)
        return false;

    // record the offset of the next opcode
    op.offset = m_curr;

    // read the opcode and increment curr
    op.opcode = static_cast<lyric_object::Opcode>(m_bytecode[m_curr++]);
    if (lyric_object::Opcode::LAST_ <= op.opcode) {
        op.type = OpInfoType::NO_OPERANDS;
        return true;
    }

    // read the operands and increment curr
    const auto &opinfo = ops[static_cast<tu_uint8>(op.opcode)];
    auto size = lyric_object::opinfo_type_to_size(opinfo.type);
    if (m_size < m_curr + size)
        return false;
    op.type = opinfo.type;

    const tu_uint8 *ptr = &m_bytecode[m_curr];
    switch (opinfo.type) {
        case lyric_object::OpInfoType::NO_OPERANDS:
            break;
        case OpInfoType::ADDRESS_U32:
            op.operands.address_u32.address = tempo_utils::read_u32_and_advance(ptr);
            break;
        case OpInfoType::FLAGS_U8_ADDRESS_U32:
            op.operands.flags_u8_address_u32.flags = tempo_utils::read_u8_and_advance(ptr);
            op.operands.flags_u8_address_u32.address = tempo_utils::read_u32_and_advance(ptr);
            break;
        case lyric_object::OpInfoType::JUMP_I16:
            op.operands.jump_i16.jump = tempo_utils::read_i16_and_advance(ptr);
            break;
        case lyric_object::OpInfoType::OFFSET_U16:
            op.operands.offset_u16.offset = tempo_utils::read_u16_and_advance(ptr);
            break;
        case lyric_object::OpInfoType::FLAGS_U8_ADDRESS_U32_PLACEMENT_U16:
            op.operands.flags_u8_address_u32_placement_u16.flags = tempo_utils::read_u8_and_advance(ptr);
            op.operands.flags_u8_address_u32_placement_u16.address = tempo_utils::read_u32_and_advance(ptr);
            op.operands.flags_u8_address_u32_placement_u16.placement = tempo_utils::read_u16_and_advance(ptr);
            break;
        case OpInfoType::FLAGS_U8_OFFSET_U16_PLACEMENT_U16:
            op.operands.flags_u8_offset_u16_placement_u16.flags = tempo_utils::read_u8_and_advance(ptr);
            op.operands.flags_u8_offset_u16_placement_u16.offset = tempo_utils::read_u16_and_advance(ptr);
            op.operands.flags_u8_offset_u16_placement_u16.placement = tempo_utils::read_u16_and_advance(ptr);
            break;
        case lyric_object::OpInfoType::TYPE_U8:
            op.operands.type_u8.type = tempo_utils::read_u8_and_advance(ptr);
            break;
        case OpInfoType::IMMEDIATE_I64:
            op.operands.immediate_i64.i64 = tempo_utils::read_i64_and_advance(ptr);
            break;
        case lyric_object::OpInfoType::IMMEDIATE_DBL:
            op.operands.immediate_dbl.dbl = tempo_utils::read_dbl_and_advance(ptr);
            break;
        case lyric_object::OpInfoType::IMMEDIATE_CHR:
            op.operands.immediate_chr.chr = tempo_utils::read_i32_and_advance(ptr);
            break;
    }
    m_curr += size;

    return true;
}

bool
lyric_object::BytecodeIterator::reset(tu_uint32 address)
{
    // if address points to 1) an addressable byte in the bytecode, or 2) the byte after the end
    // of the bytecode, then reset curr and return true
    if (address <= m_size) {
        m_curr = address;
        return true;
    }
    // otherwise if address is invalid then do not modify curr and return false
    return false;
}

bool
lyric_object::BytecodeIterator::move(int16_t offset)
{
    if (offset < 0) {
        // if offset is negative and the magnitude is larger than curr (which would place curr before
        // the start of the bytecode) then do not modify curr and return false
        if (std::abs(offset) > m_curr)
            return false;
        // otherwise offset points to an addressable byte in the bytecode so reset curr and return true
        m_curr += offset;
        return true;
    } else {
        // if offset is positive and the magnitude is larger than the delta between size and curr
        // (which would place curr past the byte after the end of the bytecode) then do not modify
        // curr and return false
        if (std::cmp_greater(offset, m_size - m_curr))
            return false;
        // otherwise offset points to an addressable byte in the bytecode so reset curr and return true
        m_curr += offset;
        return true;
    }
}

tempo_utils::LogMessage&&
lyric_object::operator<<(tempo_utils::LogMessage &&message, const lyric_object::BytecodeIterator &it)
{
    std::forward<tempo_utils::LogMessage>(message)
        << absl::Substitute("BytecodeIterator($0+$1)",
                            (void*) it.getCurr(),
                            static_cast<int>(it.getCurr() - it.getBase()));
    return std::move(message);
}
