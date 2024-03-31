
#include <lyric_assembler/code_builder.h>
#include <tempo_utils/big_endian.h>
#include <tempo_utils/ieee754.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::CodeBuilder::CodeBuilder(const std::vector<tu_uint8> &bytecode)
    : m_code(bytecode.cbegin(), bytecode.cend())
{
}

lyric_assembler::CodeBuilder::CodeBuilder(const CodeBuilder &other)
{
    m_code = other.m_code;
}

std::vector<uint8_t>
lyric_assembler::CodeBuilder::getBytecode() const
{
    return m_code;
}

int
lyric_assembler::CodeBuilder::currentOffset() const
{
    return m_code.size();
}

std::vector<uint8_t>::const_iterator
lyric_assembler::CodeBuilder::bytecodeBegin() const
{
    return m_code.cbegin();
}

std::vector<uint8_t>::const_iterator
lyric_assembler::CodeBuilder::bytecodeEnd() const
{
    return m_code.cend();
}

uint32_t
lyric_assembler::CodeBuilder::bytecodeSize() const
{
    return static_cast<uint32_t>(m_code.size());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::writeOpcode(lyric_object::Opcode op)
{
    if (lyric_object::Opcode::OP_UNKNOWN < op && op < lyric_object::Opcode::LAST_)
        return writeU8(static_cast<uint8_t>(op));
    throw tempo_utils::StatusException(
        AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "unknown opcode {}", static_cast<tu_uint8>(op)));
}

tempo_utils::Status
lyric_assembler::CodeBuilder::writeU8(uint8_t u8)
{
    m_code.push_back(u8);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::writeU16(uint16_t u16)
{
    auto offset = m_code.size();
    m_code.resize(offset + 2);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_u16(u16, ptr);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::writeU32(uint32_t u32)
{
    auto offset = m_code.size();
    m_code.resize(offset + 4);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_u32(u32, ptr);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::writeU64(uint64_t u64)
{
    auto offset = m_code.size();
    m_code.resize(offset + 8);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_u64(u64, ptr);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadNil()
{
    return writeOpcode(lyric_object::Opcode::OP_NIL);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadBool(bool b)
{
    return b ? writeOpcode(lyric_object::Opcode::OP_TRUE) : writeOpcode(lyric_object::Opcode::OP_FALSE);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadInt(int64_t i64)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_I64);
    if (!status.isOk())
        return status;
    auto offset = m_code.size();
    m_code.resize(offset + 8);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_i64(i64, ptr);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadFloat(double dbl)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_DBL);
    if (!status.isOk())
        return status;
    auto offset = m_code.size();
    m_code.resize(offset + 8);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_dbl(dbl, ptr);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadChar(UChar32 chr)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_CHR);
    if (!status.isOk())
        return status;
    auto offset = m_code.size();
    m_code.resize(offset + 4);
    auto *ptr = m_code.data() + offset;
    tempo_utils::write_i32(chr, ptr);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadLiteral(const LiteralAddress &literalAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LITERAL);
    if (!status.isOk())
        return status;
    return writeU32(literalAddress.addr);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadArgument(const ArgumentOffset &argumentOffset)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::LOAD_ARGUMENT); // load flags
    if (!status.isOk())
        return status;
    return writeU32(argumentOffset.getOffset());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadLocal(const LocalOffset &localOffset)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::LOAD_LOCAL); // load flags
    if (!status.isOk())
        return status;
    return writeU32(localOffset.getOffset());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadLexical(const LexicalOffset &lexicalOffset)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::LOAD_LEXICAL); // load flags
    if (!status.isOk())
        return status;
    return writeU32(lexicalOffset.getOffset());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadField(const FieldAddress &fieldAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::LOAD_FIELD); // load flags
    if (!status.isOk())
        return status;
    return writeU32(fieldAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadStatic(const StaticAddress &staticAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::LOAD_STATIC); // load flags
    if (!status.isOk())
        return status;
    return writeU32(staticAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadEnum(const EnumAddress &enumAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::LOAD_ENUM); // load flags
    if (!status.isOk())
        return status;
    return writeU32(enumAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadInstance(const InstanceAddress &instanceAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_LOAD);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::LOAD_INSTANCE); // load flags
    if (!status.isOk())
        return status;
    return writeU32(instanceAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadSynthetic(lyric_assembler::SyntheticType synthetic)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_SYNTHETIC);
    if (!status.isOk())
        return status;
    switch (synthetic) {
        case SyntheticType::THIS:
            return writeU8(lyric_object::SYNTHETIC_THIS);
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid synthetic type");
    }
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadClass(const ClassAddress &classAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Class));
    if (!status.isOk())
        return status;
    return writeU32(classAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadStruct(const StructAddress &structAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Struct));
    if (!status.isOk())
        return status;
    return writeU32(structAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadConcept(const ConceptAddress &conceptAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Concept));
    if (!status.isOk())
        return status;
    return writeU32(conceptAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadCall(const CallAddress &callAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Call));
    if (!status.isOk())
        return status;
    return writeU32(callAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadExistential(const ExistentialAddress &existentialAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Existential));
    if (!status.isOk())
        return status;
    return writeU32(existentialAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::loadType(const TypeAddress &typeAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Type));
    if (!status.isOk())
        return status;
    return writeU32(typeAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::storeArgument(const ArgumentOffset &argumentOffset)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::STORE_ARGUMENT); // store flags
    if (!status.isOk())
        return status;
    return writeU32(argumentOffset.getOffset());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::storeLocal(const LocalOffset &localOffset)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::STORE_LOCAL); // store flags
    if (!status.isOk())
        return status;
    return writeU32(localOffset.getOffset());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::storeLexical(const LexicalOffset &lexicalOffset)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::STORE_LEXICAL); // store flags
    if (!status.isOk())
        return status;
    return writeU32(lexicalOffset.getOffset());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::storeField(const FieldAddress &fieldAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::STORE_FIELD); // store flags
    if (!status.isOk())
        return status;
    return writeU32(fieldAddress.getAddress());
}

tempo_utils::Status
lyric_assembler::CodeBuilder::storeStatic(const StaticAddress &staticAddress)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_STORE);
    if (!status.isOk())
        return status;
    status = writeU8(lyric_object::STORE_STATIC); // store flags
    if (!status.isOk())
        return status;
    return writeU32(staticAddress.getAddress());
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::makePatch()
{
    PatchOffset offset(static_cast<uint16_t>(m_code.size()));
    auto status = writeU16(0);
    if (!status.isOk())
        return status;
    return offset;
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfNil()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_NIL);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfNotNil()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_NOTNIL);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfTrue()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_TRUE);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfFalse()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_FALSE);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfZero()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_ZERO);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfNotZero()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_NOTZERO);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfLessThan()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_LT);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfLessOrEqual()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_LE);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfGreaterThan()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_GT);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jumpIfGreaterOrEqual()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_IF_GE);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Result<lyric_assembler::PatchOffset>
lyric_assembler::CodeBuilder::jump()
{
    auto status = writeOpcode(lyric_object::Opcode::OP_JUMP);
    if (!status.isOk())
        return status;
    return makePatch();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::jump(const JumpLabel &label)
{
    auto status = writeOpcode(lyric_object::Opcode::OP_JUMP);
    if (!status.isOk())
        return status;
    auto makePatchResult = makePatch();
    if (makePatchResult.isStatus())
        return makePatchResult.getStatus();
    return patch(makePatchResult.getResult(), label);
}

tempo_utils::Result<lyric_assembler::JumpLabel>
lyric_assembler::CodeBuilder::makeLabel()
{
    JumpLabel jump(static_cast<uint16_t>(m_code.size()));
    return jump;
}

tempo_utils::Status
lyric_assembler::CodeBuilder::patch(const PatchOffset &dst, const JumpLabel &src)
{
    // dstoffset is the patch offset + 2 bytes for the address
    auto dstoffset = dst.getOffset() + 2;
    // delta is the difference between the jump label and dstoffset
    int32_t delta = src.getOffset() - dstoffset;
    TU_ASSERT (std::numeric_limits<int16_t>::min() <= delta && delta <= std::numeric_limits<int16_t>::max());
    auto idx = dst.getOffset();
    TU_ASSERT(idx < m_code.size());
    auto i16 = H_TO_BE16(static_cast<int16_t>(delta));
    auto *ptr = (const tu_uint8 *) &i16;
    m_code[idx] = ptr[0];
    m_code[idx + 1] = ptr[1];
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::popValue()
{
    return writeOpcode(lyric_object::Opcode::OP_POP);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::dupValue()
{
    return writeOpcode(lyric_object::Opcode::OP_DUP);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::pickValue(uint16_t pickOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_PICK);
    if (!status.isOk())
        return status;
    writeU16(pickOffset);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::dropValue(uint16_t dropOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_DROP);
    if (!status.isOk())
        return status;
    writeU16(dropOffset);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::rpickValue(uint16_t pickOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_RPICK);
    if (!status.isOk())
        return status;
    writeU16(pickOffset);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::rdropValue(uint16_t dropOffset)
{
    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_RDROP);
    if (!status.isOk())
        return status;
    writeU16(dropOffset);
    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::callStatic(
    const CallAddress &address,
    uint16_t placementSize,
    [[maybe_unused]] uint8_t flags)
{
    TU_ASSERT (address.isValid());

    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_CALL_STATIC);
    if (!status.isOk())
        return status;

    // write flags (currently no flags defined so we set this to zero)
    status = writeU8(0);
    if (!status.isOk())
        return status;

    // write call address
    status = writeU32(address.getAddress());
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::callVirtual(const CallAddress &address, uint16_t placementSize, uint8_t flags)
{
    TU_ASSERT (address.isValid());

    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_CALL_VIRTUAL);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write virtual offset
    status = writeU32(address.getAddress());
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

//tempo_utils::Status
//lyric_assembler::CodeBuilder::callExtension(const ActionAddress &address, uint16_t placementSize, uint8_t flags)
//{
//    TU_ASSERT (address.isValid());
//
//    tempo_utils::Status status;
//    status = writeOpcode(lyric_object::Opcode::OP_CALL_EXTENSION);
//    if (!status.isOk())
//        return status;
//
//    // write flags
//    status = writeU8(flags);
//    if (!status.isOk())
//        return status;
//
//    // write action address
//    status = writeU32(address.getAddress());
//    if (!status.isOk())
//        return status;
//
//    // write placement size
//    return writeU16(placementSize);
//}

tempo_utils::Status
lyric_assembler::CodeBuilder::callConcept(const ActionAddress &address, uint16_t placementSize, uint8_t flags)
{
    TU_ASSERT (address.isValid());

    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_CALL_CONCEPT);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write action address
    status = writeU32(address.getAddress());
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::callExistential(const CallAddress &address, tu_uint16 placementSize, tu_uint8 flags)
{
    TU_ASSERT (address.isValid());

    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_CALL_EXISTENTIAL);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write action address
    status = writeU32(address.getAddress());
    if (!status.isOk())
        return status;

    // write placement size
    return writeU16(placementSize);
}

tempo_utils::Status
lyric_assembler::CodeBuilder::callInline(const CodeBuilder *inlineCode, [[maybe_unused]] uint8_t flags)
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
            throw tempo_utils::StatusException(
                AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant, "invalid inline code"));
        if (op.opcode != lyric_object::Opcode::OP_LOAD || op.operands.flags_u8_address_u32.flags != lyric_object::LOAD_ARGUMENT)
            break;
        auto offset = lyric_object::opinfo_type_to_size(op.type) + 1;
        code += offset;
        size -= offset;
    }

    // append the remaining code
    for (tu_uint32 i = 0; i < size; i++) {
        m_code.push_back(code[i]);
    }

    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::CodeBuilder::callNew(
    uint32_t address,
    uint16_t placementSize,
    uint8_t newType,
    uint8_t flags)
{
    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_NEW);
    if (!status.isOk())
        return status;

    // write flags (currently no flags defined so we set this to zero)
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
lyric_assembler::CodeBuilder::trap(uint32_t address, uint8_t flags)
{
    tempo_utils::Status status;
    status = writeOpcode(lyric_object::Opcode::OP_TRAP);
    if (!status.isOk())
        return status;

    // write flags
    status = writeU8(flags);
    if (!status.isOk())
        return status;

    // write trap address
    return writeU32(address);
}
