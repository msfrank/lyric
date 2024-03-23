
#include <lyric_object/bytecode_builder.h>
#include <lyric_object/bytecode_iterator.h>
#include <lyric_object/generated/object.h>
#include <lyric_object/object_result.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/ieee754.h>
#include <tempo_utils/log_stream.h>

lyric_object::BytecodeBuilder::BytecodeBuilder(const std::vector<tu_uint8> &bytecode)
    : m_code(bytecode)
{
}

lyric_object::BytecodeBuilder::BytecodeBuilder(const BytecodeBuilder &other)
{
    m_code = other.m_code;
}

lyric_object::BytecodeBuilder::BytecodeBuilder(BytecodeBuilder &&other)
{
    m_code.swap(other.m_code);
}

lyric_object::BytecodeBuilder&
lyric_object::BytecodeBuilder::operator=(const BytecodeBuilder &other)
{
    m_code = other.m_code;
    return *this;
}

lyric_object::BytecodeBuilder&
lyric_object::BytecodeBuilder::operator=(BytecodeBuilder &&other) noexcept
{
    if (this != &other) {
        m_code.swap(other.m_code);
    }
    return *this;
}

std::vector<tu_uint8>
lyric_object::BytecodeBuilder::getBytecode() const
{
    return m_code;
}

int
lyric_object::BytecodeBuilder::currentOffset() const
{
    return m_code.size();
}

std::vector<tu_uint8>::const_iterator
lyric_object::BytecodeBuilder::bytecodeBegin() const
{
    return m_code.cbegin();
}

std::vector<tu_uint8>::const_iterator
lyric_object::BytecodeBuilder::bytecodeEnd() const
{
    return m_code.cend();
}

uint32_t
lyric_object::BytecodeBuilder::bytecodeSize() const
{
    return static_cast<uint32_t>(m_code.size());
}

tempo_utils::Status
lyric_object::BytecodeBuilder::writeOpcode(Opcode op)
{
    if (Opcode::OP_UNKNOWN < op && op < Opcode::LAST_)
        return writeU8(static_cast<tu_uint8>(op));
    return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
        "unknown opcode {}", static_cast<tu_uint8>(op));
}

tempo_utils::Status
lyric_object::BytecodeBuilder::writeU8(tu_uint8 u8)
{
    m_code.push_back(u8);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::writeU16(tu_uint16 u16)
{
    auto offset = m_code.size();
    m_code.resize(offset + 2);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_u16(u16, ptr);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::writeU32(tu_uint32 u32)
{
    auto offset = m_code.size();
    m_code.resize(offset + 4);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_u32(u32, ptr);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::writeU64(tu_uint64 u64)
{
    auto offset = m_code.size();
    m_code.resize(offset + 8);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_u64(u64, ptr);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadNil()
{
    return writeOpcode(Opcode::OP_NIL);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadBool(bool b)
{
    return b ? writeOpcode(Opcode::OP_TRUE) : writeOpcode(Opcode::OP_FALSE);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadInt(tu_int64 i64)
{
    auto status = writeOpcode(Opcode::OP_I64);
    if (!status.isOk())
        return status;
    auto offset = m_code.size();
    m_code.resize(offset + 8);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_i64(i64, ptr);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadFloat(double dbl)
{
    auto status = writeOpcode(Opcode::OP_DBL);
    if (!status.isOk())
        return status;
    auto offset = m_code.size();
    m_code.resize(offset + 8);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_dbl(dbl, ptr);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadChar(UChar32 chr)
{
    auto status = writeOpcode(Opcode::OP_CHR);
    if (!status.isOk())
        return status;
    auto offset = m_code.size();
    m_code.resize(offset + 4);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_i32(chr, ptr);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadArgument(tu_uint16 index)
{
    auto status = writeOpcode(Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(LOAD_ARGUMENT); // load flags
    if (!status.isOk())
        return status;
    return writeU32(index);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadLocal(tu_uint16 index)
{
    auto status = writeOpcode(Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(LOAD_LOCAL); // load flags
    if (!status.isOk())
        return status;
    return writeU32(index);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadLexical(tu_uint16 index)
{
    auto status = writeOpcode(Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(LOAD_LEXICAL); // load flags
    if (!status.isOk())
        return status;
    return writeU32(index);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadReceiver()
{
    auto status = writeOpcode(Opcode::OP_SYNTHETIC);
    if (!status.isOk())
        return status;
    return writeU8(SYNTHETIC_THIS);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadLiteral(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_LITERAL);
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadField(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(LOAD_FIELD); // load flags
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadStatic(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(LOAD_STATIC); // load flags
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadEnum(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(LOAD_ENUM); // load flags
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadInstance(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(LOAD_INSTANCE); // load flags
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadClass(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(static_cast<tu_uint8>(lyo1::DescriptorSection::Class));
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadStruct(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(static_cast<tu_uint8>(lyo1::DescriptorSection::Struct));
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadConcept(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(static_cast<tu_uint8>(lyo1::DescriptorSection::Concept));
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadCall(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(static_cast<tu_uint8>(lyo1::DescriptorSection::Call));
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::loadType(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(static_cast<tu_uint8>(lyo1::DescriptorSection::Type));
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::storeArgument(tu_uint16 index)
{
    auto status = writeOpcode(Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(STORE_ARGUMENT); // store flags
    if (!status.isOk())
        return status;
    return writeU32(index);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::storeLocal(tu_uint16 index)
{
    auto status = writeOpcode(Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(STORE_LOCAL); // store flags
    if (!status.isOk())
        return status;
    return writeU32(index);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::storeLexical(tu_uint16 index)
{
    auto status = writeOpcode(Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(STORE_LEXICAL); // store flags
    if (!status.isOk())
        return status;
    return writeU32(index);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::storeField(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(STORE_FIELD); // store flags
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::storeStatic(tu_uint32 address)
{
    auto status = writeOpcode(Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(STORE_STATIC); // store flags
    if (!status.isOk())
        return status;
    return writeU32(address);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::makePatch(tu_uint16 &patchOffset)
{
    patchOffset = (static_cast<uint16_t>(m_code.size()));
    return writeU16(0);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfNil(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_NIL);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfNotNil(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_NOTNIL);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfTrue(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_TRUE);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfFalse(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_FALSE);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfZero(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_ZERO);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfNotZero(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_NOTZERO);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfLessThan(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_LT);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfLessOrEqual(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_LE);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfGreaterThan(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_GT);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpIfGreaterOrEqual(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_IF_GE);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jump(tu_uint16 &patchOffset)
{
    auto status = writeOpcode(Opcode::OP_JUMP);
    return makePatch(patchOffset);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::jumpTo(const tu_uint16 jumpLabel)
{
    auto status = writeOpcode(Opcode::OP_JUMP);
    if (!status.isOk())
        return status;
    tu_uint16 patchOffset;
    status = makePatch(patchOffset);
    if (!status.isOk())
        return status;
    return patch(patchOffset, jumpLabel);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::makeLabel(tu_uint16 &jumpLabel)
{
    jumpLabel = static_cast<uint16_t>(m_code.size());
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::patch(tu_uint16 patchOffset, tu_uint16 jumpLabel)
{
    if (std::cmp_greater_equal(patchOffset + 1, m_code.size()))
        return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
            "invalid patch offset {}", patchOffset);
    if (jumpLabel > m_code.size())
        return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
            "invalid jump label {}", jumpLabel);
    // dstoffset is the patch offset + 2 bytes for the address
    auto dstoffset = patchOffset + 2;
    // delta is the difference between the jump label and dstoffset
    int32_t delta = jumpLabel - dstoffset;
    if (delta < std::numeric_limits<int16_t>::min() || delta > std::numeric_limits<int16_t>::max())
        return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
            "jump delta {} is too large", delta);
    auto i16 = H_TO_BE16(static_cast<int16_t>(delta));
    auto *ptr = (const tu_uint8 *) &i16;
    m_code[patchOffset] = ptr[0];
    m_code[patchOffset + 1] = ptr[1];
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::popValue()
{
    return writeOpcode(Opcode::OP_POP);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::dupValue()
{
    return writeOpcode(Opcode::OP_DUP);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::pickValue(uint16_t pickOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_PICK);
    if (!status.isOk())
        return status;
    writeU16(pickOffset);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::dropValue(uint16_t dropOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_DROP);
    if (!status.isOk())
        return status;
    writeU16(dropOffset);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::rpickValue(uint16_t pickOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_RPICK);
    if (!status.isOk())
        return status;
    writeU16(pickOffset);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::rdropValue(uint16_t dropOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_RDROP);
    if (!status.isOk())
        return status;
    writeU16(dropOffset);
    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::callStatic(
    tu_uint32 address,
    uint16_t placementSize,
    [[maybe_unused]] tu_uint8 flags)
{
    if (address == INVALID_ADDRESS_U32)
        return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
            "invalid address for static call");

    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_CALL_STATIC);
    if (!status.isOk())
        return status;

    // write flags (currently no flags defined so we set this to zero)
    status = writeU8(0);
    if (!status.isOk())
        return status;

    // write call address
    status = writeU32(address);
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::callVirtual(
    tu_uint32 address,
    uint16_t placementSize,
    tu_uint8 flags)
{
    if (address == INVALID_ADDRESS_U32)
        return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
            "invalid address for virtual call");

    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_CALL_VIRTUAL);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write virtual offset
    status = writeU32(address);
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::callConcept(
    tu_uint32 address,
    uint16_t placementSize,
    tu_uint8 flags)
{
    if (address == INVALID_ADDRESS_U32)
        return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
            "invalid address for concept call");

    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_CALL_CONCEPT);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write action address
    status = writeU32(address);
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::callExistential(
    tu_uint32 address,
    uint16_t placementSize,
    tu_uint8 flags)
{
    if (address == INVALID_ADDRESS_U32)
        return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
            "invalid address for existential call");

    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_CALL_EXISTENTIAL);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write action address
    status = writeU32(address);
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::callInline(
    const BytecodeBuilder *inlineCode,
    [[maybe_unused]] tu_uint8 flags)
{
    TU_ASSERT (inlineCode != nullptr);

    const auto *code = inlineCode->m_code.data();
    auto size = inlineCode->m_code.size();

    // skip over the argument load instructions. arguments will already be present
    // on the stack when invoking an inlined call.
    lyric_object::BytecodeIterator it(code, size);
    for(;;) {
        lyric_object::OpCell op;
        if (!it.getNext(op))
            return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                "invalid inline code");
        if (op.opcode != Opcode::OP_LOAD || op.operands.flags_u8_address_u32.flags != LOAD_ARGUMENT)
            break;
        auto offset = lyric_object::opinfo_type_to_size(op.type) + 1;
        code += offset;
        size -= offset;
    }

    // append the remaining code
    for (tu_uint32 i = 0; i < size; i++) {
        m_code.push_back(code[i]);
    }

    return ObjectStatus::ok();
}

tempo_utils::Status
lyric_object::BytecodeBuilder::callNew(
    uint32_t address,
    uint16_t placementSize,
    tu_uint8 newType,
    tu_uint8 flags)
{
    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_NEW);
    if (!status.isOk())
        return status;

    // write flags and newtype
    status = writeU8((newType << 4u) | flags);
    if (!status.isOk())
        return status;

    // write call address
    status = writeU32(address);
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_object::BytecodeBuilder::trap(uint32_t address, tu_uint8 flags)
{
    tempo_utils::Status status;
    status = writeOpcode(Opcode::OP_TRAP);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write trap address
    return writeU32(address);
}
