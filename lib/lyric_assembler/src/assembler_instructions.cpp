
#include <lyric_assembler/assembler_instructions.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/literal_cache.h>
#include <lyric_assembler/literal_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/synthetic_symbol.h>

lyric_assembler::NoOperandsInstruction::NoOperandsInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::NoOperandsInstruction::getType() const
{
    return InstructionType::NoOperands;
}

tempo_utils::Status
lyric_assembler::NoOperandsInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(m_opcode);
}

lyric_assembler::BoolImmediateInstruction::BoolImmediateInstruction(bool b)
    : m_b(b)
{
}

lyric_assembler::InstructionType
lyric_assembler::BoolImmediateInstruction::getType() const
{
    return InstructionType::BoolImmediate;
}

tempo_utils::Status
lyric_assembler::BoolImmediateInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadBool(m_b);
}

lyric_assembler::IntImmediateInstruction::IntImmediateInstruction(tu_int64 i64)
    : m_i64(i64)
{
}

lyric_assembler::InstructionType
lyric_assembler::IntImmediateInstruction::getType() const
{
    return InstructionType::IntImmediate;
}

tempo_utils::Status
lyric_assembler::IntImmediateInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadInt(m_i64);
}

lyric_assembler::FloatImmediateInstruction::FloatImmediateInstruction(double dbl)
    : m_dbl(dbl)
{
}

lyric_assembler::InstructionType
lyric_assembler::FloatImmediateInstruction::getType() const
{
    return InstructionType::FloatImmediate;
}

tempo_utils::Status
lyric_assembler::FloatImmediateInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadFloat(m_dbl);
}

lyric_assembler::CharImmediateInstruction::CharImmediateInstruction(UChar32 chr)
    : m_chr(chr)
{
}

lyric_assembler::InstructionType
lyric_assembler::CharImmediateInstruction::getType() const
{
    return InstructionType::CharImmediate;
}

tempo_utils::Status
lyric_assembler::CharImmediateInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadChar(m_chr);
}

lyric_assembler::LoadLiteralInstruction::LoadLiteralInstruction(
    lyric_object::Opcode opcode,
    LiteralAddress address)
    : m_opcode(opcode),
      m_address(address)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::LoadLiteralInstruction::getType() const
{
    return InstructionType::LoadLiteral;
}

tempo_utils::Status
lyric_assembler::LoadLiteralInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_STRING:
        case lyric_object::Opcode::OP_URL: {
            TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(m_opcode));
            return bytecodeBuilder.writeU32(m_address.addr);
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

lyric_assembler::LoadDataInstruction::LoadDataInstruction(AbstractSymbol *symbol)
    : m_symbol(symbol)
{
    TU_ASSERT (m_symbol != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::LoadDataInstruction::getType() const
{
    return InstructionType::LoadData;
}

tempo_utils::Status
lyric_assembler::LoadDataInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    m_symbol->touch();

    tu_uint32 address;
    tu_uint8 flags;

    switch (m_symbol->getSymbolType()) {
        case SymbolType::ARGUMENT:
            address = cast_symbol_to_argument(m_symbol)->getOffset().getOffset();
            flags = lyric_object::LOAD_ARGUMENT;
            break;
        case SymbolType::LOCAL:
            address = cast_symbol_to_local(m_symbol)->getOffset().getOffset();
            flags = lyric_object::LOAD_LOCAL;
            break;
        case SymbolType::LEXICAL:
            address = cast_symbol_to_lexical(m_symbol)->getOffset().getOffset();
            flags = lyric_object::LOAD_LEXICAL;
            break;
        case SymbolType::FIELD:
            address = cast_symbol_to_field(m_symbol)->getAddress().getAddress();
            flags = lyric_object::LOAD_FIELD;
            break;
        case SymbolType::STATIC:
            address = cast_symbol_to_static(m_symbol)->getAddress().getAddress();
            flags = lyric_object::LOAD_STATIC;
            break;
        case SymbolType::INSTANCE:
            address = cast_symbol_to_instance(m_symbol)->getAddress().getAddress();
            flags = lyric_object::LOAD_INSTANCE;
            break;
        case SymbolType::ENUM:
            address = cast_symbol_to_enum(m_symbol)->getAddress().getAddress();
            flags = lyric_object::LOAD_ENUM;
            break;
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kInvalidBinding,
                "cannot load data from {}", m_symbol->getSymbolUrl().toString());
    }

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_LOAD));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8(flags));
    return bytecodeBuilder.writeU32(address);
}

lyric_assembler::AbstractSymbol *
lyric_assembler::LoadDataInstruction::getSymbol() const
{
    return m_symbol;
}

lyric_assembler::LoadDescriptorInstruction::LoadDescriptorInstruction(AbstractSymbol *symbol)
    : m_symbol(symbol)
{
    TU_ASSERT (m_symbol != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::LoadDescriptorInstruction::getType() const
{
    return InstructionType::LoadDescriptor;
}

tempo_utils::Status
lyric_assembler::LoadDescriptorInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    m_symbol->touch();

    tu_uint32 address;
    lyric_object::LinkageSection section;

    switch (m_symbol->getSymbolType()) {
        case SymbolType::ACTION:
            address = cast_symbol_to_action(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Action;
            break;
        case SymbolType::CALL:
            address = cast_symbol_to_call(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Call;
            break;
        case SymbolType::CLASS:
            address = cast_symbol_to_class(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Class;
            break;
        case SymbolType::CONCEPT:
            address = cast_symbol_to_concept(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Concept;
            break;
        case SymbolType::ENUM:
            address = cast_symbol_to_enum(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Enum;
            break;
        case SymbolType::EXISTENTIAL:
            address = cast_symbol_to_existential(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Existential;
            break;
        case SymbolType::FIELD:
            address = cast_symbol_to_field(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Field;
            break;
        case SymbolType::INSTANCE:
            address = cast_symbol_to_instance(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Instance;
            break;
        case SymbolType::STRUCT:
            address = cast_symbol_to_struct(m_symbol)->getAddress().getAddress();
            section = lyric_object::LinkageSection::Struct;
            break;
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kInvalidBinding,
                "cannot load descriptor for {}", m_symbol->getSymbolUrl().toString());
    }

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8(lyric_object::linkage_to_descriptor_section(section)));
    return bytecodeBuilder.writeU32(address);
}

lyric_assembler::AbstractSymbol *
lyric_assembler::LoadDescriptorInstruction::getSymbol() const
{
    return m_symbol;
}

lyric_assembler::LoadSyntheticInstruction::LoadSyntheticInstruction(SyntheticType type)
    : m_type(type)
{
}

lyric_assembler::InstructionType
lyric_assembler::LoadSyntheticInstruction::getType() const
{
    return InstructionType::LoadSynthetic;
}

tempo_utils::Status
lyric_assembler::LoadSyntheticInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_SYNTHETIC));
    switch (m_type) {
        case SyntheticType::THIS:
            return bytecodeBuilder.writeU8(lyric_object::SYNTHETIC_THIS);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid synthetic type");
   }
}

lyric_assembler::LoadTypeInstruction::LoadTypeInstruction(TypeHandle *typeHandle)
    : m_typeHandle(typeHandle)
{
    TU_ASSERT (m_typeHandle != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::LoadTypeInstruction::getType() const
{
    return InstructionType::LoadType;
}

tempo_utils::Status
lyric_assembler::LoadTypeInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    m_typeHandle->touch();

    auto address = m_typeHandle->getAddress().getAddress();

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8(
        lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Type)));
    return bytecodeBuilder.writeU32(address);
}

lyric_assembler::StoreDataInstruction::StoreDataInstruction(AbstractSymbol *symbol)
    : m_symbol(symbol)
{
    TU_ASSERT (m_symbol != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::StoreDataInstruction::getType() const
{
    return InstructionType::StoreData;
}

tempo_utils::Status
lyric_assembler::StoreDataInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const {
    m_symbol->touch();

    tu_uint32 address;
    tu_uint8 flags;

    switch (m_symbol->getSymbolType()) {

        case SymbolType::ARGUMENT:
            address = cast_symbol_to_argument(m_symbol)->getOffset().getOffset();
            flags = lyric_object::STORE_ARGUMENT;
            break;
        case SymbolType::LOCAL:
            address = cast_symbol_to_local(m_symbol)->getOffset().getOffset();
            flags = lyric_object::STORE_LOCAL;
            break;
        case SymbolType::LEXICAL:
            address = cast_symbol_to_lexical(m_symbol)->getOffset().getOffset();
            flags = lyric_object::STORE_LEXICAL;
            break;
        case SymbolType::FIELD:
            address = cast_symbol_to_field(m_symbol)->getAddress().getAddress();
            flags = lyric_object::STORE_FIELD;
            break;
        case SymbolType::STATIC:
            address = cast_symbol_to_static(m_symbol)->getAddress().getAddress();
            flags = lyric_object::STORE_STATIC;
            break;
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kInvalidBinding,
                "cannot store data to {}", m_symbol->getSymbolUrl().toString());
    }

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_STORE));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8(flags));
    return bytecodeBuilder.writeU32(address);
}

lyric_assembler::AbstractSymbol *
lyric_assembler::StoreDataInstruction::getSymbol() const
{
    return m_symbol;
}

lyric_assembler::StackModificationInstruction::StackModificationInstruction(
    lyric_object::Opcode opcode,
    tu_uint16 offset)
    : m_opcode(opcode),
      m_offset(offset)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::StackModificationInstruction::getType() const
{
    return InstructionType::StackModification;
}

tempo_utils::Status
lyric_assembler::StackModificationInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_POP:
            return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_POP);
        case lyric_object::Opcode::OP_DUP:
            return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_DUP);
        case lyric_object::Opcode::OP_PICK:
        case lyric_object::Opcode::OP_DROP:
        case lyric_object::Opcode::OP_RPICK:
        case lyric_object::Opcode::OP_RDROP:
            TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(m_opcode));
            return bytecodeBuilder.writeU16(m_offset);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

lyric_assembler::LabelInstruction::LabelInstruction(std::string_view name)
    : m_name(name)
{
    TU_ASSERT (!m_name.empty());
}

lyric_assembler::InstructionType
lyric_assembler::LabelInstruction::getType() const
{
    return InstructionType::Label;
}

tempo_utils::Status
lyric_assembler::LabelInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    labelName = getName();
    return bytecodeBuilder.makeLabel(labelOffset);
}

std::string
lyric_assembler::LabelInstruction::getName() const
{
    return m_name;
}

lyric_assembler::JumpInstruction::JumpInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::JumpInstruction::JumpInstruction(lyric_object::Opcode opcode, tu_uint32 targetId)
    : m_opcode(opcode),
      m_targetId(targetId)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::JumpInstruction::getType() const
{
    return InstructionType::Jump;
}

tempo_utils::Status
lyric_assembler::JumpInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    targetId = m_targetId;
    switch (m_opcode) {
        case lyric_object::Opcode::OP_IF_NIL:
            return bytecodeBuilder.jumpIfNil(patchOffset);
        case lyric_object::Opcode::OP_IF_NOTNIL:
            return bytecodeBuilder.jumpIfNotNil(patchOffset);
        case lyric_object::Opcode::OP_IF_TRUE:
            return bytecodeBuilder.jumpIfTrue(patchOffset);
        case lyric_object::Opcode::OP_IF_FALSE:
            return bytecodeBuilder.jumpIfFalse(patchOffset);
        case lyric_object::Opcode::OP_IF_ZERO:
            return bytecodeBuilder.jumpIfZero(patchOffset);
        case lyric_object::Opcode::OP_IF_NOTZERO:
            return bytecodeBuilder.jumpIfNotZero(patchOffset);
        case lyric_object::Opcode::OP_IF_GT:
            return bytecodeBuilder.jumpIfGreaterThan(patchOffset);
        case lyric_object::Opcode::OP_IF_GE:
            return bytecodeBuilder.jumpIfGreaterOrEqual(patchOffset);
        case lyric_object::Opcode::OP_IF_LT:
            return bytecodeBuilder.jumpIfLessThan(patchOffset);
        case lyric_object::Opcode::OP_IF_LE:
            return bytecodeBuilder.jumpIfLessOrEqual(patchOffset);
        case lyric_object::Opcode::OP_JUMP:
            return bytecodeBuilder.jump(patchOffset);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

lyric_object::Opcode
lyric_assembler::JumpInstruction::getOpcode() const
{
    return m_opcode;
}

tu_uint32
lyric_assembler::JumpInstruction::getTargetId() const
{
    return m_targetId;
}

lyric_assembler::CallInstruction::CallInstruction(
    lyric_object::Opcode opcode,
    AbstractSymbol *symbol,
    tu_uint16 placement,
    tu_uint8 flags)
    : m_opcode(opcode),
      m_symbol(symbol),
      m_placement(placement),
      m_flags(flags)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
    TU_ASSERT (m_symbol != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::CallInstruction::getType() const
{
    return InstructionType::Call;
}

tempo_utils::Status
lyric_assembler::CallInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    tu_uint32 address;

    switch (m_opcode) {
        case lyric_object::Opcode::OP_CALL_STATIC:
        case lyric_object::Opcode::OP_CALL_VIRTUAL: {
            if (m_symbol->getSymbolType() != SymbolType::CALL)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "expected Call symbol for opcode");
            auto *callSymbol = cast_symbol_to_call(m_symbol);
            address = callSymbol->getAddress().getAddress();
            break;
        }
        case lyric_object::Opcode::OP_CALL_CONCEPT: {
            if (m_symbol->getSymbolType() != SymbolType::ACTION)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "expected Action symbol for opcode");
            auto *actionSymbol = cast_symbol_to_action(m_symbol);
            address = actionSymbol->getAddress().getAddress();
            break;
        }
        case lyric_object::Opcode::OP_CALL_EXISTENTIAL: {
            if (m_symbol->getSymbolType() != SymbolType::CALL)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "expected Call symbol for opcode");
            auto *callSymbol = cast_symbol_to_call(m_symbol);
            address = callSymbol->getAddress().getAddress();
            break;
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(m_opcode));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8(m_flags));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU32(address));
    return bytecodeBuilder.writeU16(m_placement);
}

lyric_assembler::InlineInstruction::InlineInstruction(CallSymbol *callSymbol)
    : m_symbol(callSymbol)
{
    TU_ASSERT (m_symbol != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::InlineInstruction::getType() const
{
    return InstructionType::Inline;
}

tempo_utils::Status
lyric_assembler::InlineInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
//    auto *procHandle = m_symbol->callProc();
//
//    // store arguments in temporaries
//
//    // rewrite argument loads as temporary loads
//    auto *srcCode = procHandle->procCode();
//    auto *dstCode = procHandle->procCode();
//    return procCode->build(bytecodeBuilder);
    TU_UNREACHABLE();
}

lyric_assembler::NewInstruction::NewInstruction(
    AbstractSymbol *symbol,
    tu_uint16 placement,
    tu_uint8 flags)
    : m_symbol(symbol),
      m_placement(placement),
      m_flags(flags)
{
    TU_ASSERT (m_symbol != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::NewInstruction::getType() const
{
    return InstructionType::New;
}

tempo_utils::Status
lyric_assembler::NewInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    tu_uint8 newType;
    tu_uint32 address;

    switch (m_symbol->getSymbolType()) {
        case SymbolType::CLASS:
            newType = lyric_object::NEW_CLASS;
            address = cast_symbol_to_class(m_symbol)->getAddress().getAddress();
            break;
        case SymbolType::ENUM:
            newType = lyric_object::NEW_ENUM;
            address = cast_symbol_to_enum(m_symbol)->getAddress().getAddress();
            break;
        case SymbolType::INSTANCE:
            newType = lyric_object::NEW_INSTANCE;
            address = cast_symbol_to_instance(m_symbol)->getAddress().getAddress();
            break;
        case SymbolType::STRUCT:
            newType = lyric_object::NEW_STRUCT;
            address = cast_symbol_to_struct(m_symbol)->getAddress().getAddress();
            break;
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid new symbol");
    }

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_NEW));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8((newType << 4u) | m_flags));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU32(address));
    return bytecodeBuilder.writeU16(m_placement);
}

lyric_assembler::TrapInstruction::TrapInstruction(tu_uint32 trapNumber, tu_uint8 flags)
    : m_trapNumber(trapNumber),
      m_flags(flags)
{
}

lyric_assembler::InstructionType
lyric_assembler::TrapInstruction::getType() const
{
    return InstructionType::Trap;
}

tempo_utils::Status
lyric_assembler::TrapInstruction::apply(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.trap(m_trapNumber, m_flags);
}
