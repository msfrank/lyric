
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
#include <tempo_utils/unicode.h>

lyric_assembler::InstructionType
lyric_assembler::NoopInstruction::getType() const
{
    return InstructionType::Noop;
}

tempo_utils::Status
lyric_assembler::NoopInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::NoopInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_NOOP);
}

std::string
lyric_assembler::NoopInstruction::toString() const
{
    return "No Operation";
}

lyric_assembler::InstructionType
lyric_assembler::NilImmediateInstruction::getType() const
{
    return InstructionType::NilImmediate;
}

tempo_utils::Status
lyric_assembler::NilImmediateInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::NilImmediateInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_NIL);
}

std::string
lyric_assembler::NilImmediateInstruction::toString() const
{
    return "Nil Immediate";
}

lyric_assembler::InstructionType
lyric_assembler::UndefImmediateInstruction::getType() const
{
    return InstructionType::UndefImmediate;
}

tempo_utils::Status
lyric_assembler::UndefImmediateInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::UndefImmediateInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_UNDEF);
}

std::string
lyric_assembler::UndefImmediateInstruction::toString() const
{
    return "Undef Immediate";
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
lyric_assembler::BoolImmediateInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::BoolImmediateInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadBool(m_b);
}

std::string
lyric_assembler::BoolImmediateInstruction::toString() const
{
    return absl::StrCat("Immediate Bool: ", m_b? "true" : "false");
}

bool
lyric_assembler::BoolImmediateInstruction::boolValue() const
{
    return m_b;
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
lyric_assembler::IntImmediateInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::IntImmediateInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadInt(m_i64);
}

std::string
lyric_assembler::IntImmediateInstruction::toString() const
{
    return absl::StrCat("Immediate Int: ", m_i64);
}

tu_int64
lyric_assembler::IntImmediateInstruction::intValue() const
{
    return m_i64;
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
lyric_assembler::FloatImmediateInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::FloatImmediateInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadFloat(m_dbl);
}

std::string
lyric_assembler::FloatImmediateInstruction::toString() const
{
    return absl::StrCat("Immediate Float: ", m_dbl);
}

double
lyric_assembler::FloatImmediateInstruction::floatValue() const
{
    return m_dbl;
}

lyric_assembler::CharImmediateInstruction::CharImmediateInstruction(char32_t chr)
    : m_chr(chr)
{
}

lyric_assembler::InstructionType
lyric_assembler::CharImmediateInstruction::getType() const
{
    return InstructionType::CharImmediate;
}

tempo_utils::Status
lyric_assembler::CharImmediateInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::CharImmediateInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.loadChar(m_chr);
}

std::string
lyric_assembler::CharImmediateInstruction::toString() const
{
    return absl::StrCat("Immediate Char: ", tempo_utils::convert_to_utf8(m_chr));
}

char32_t
lyric_assembler::CharImmediateInstruction::charValue() const
{
    return m_chr;
}

lyric_assembler::BoolOperationInstruction::BoolOperationInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::BoolOperationInstruction::getType() const
{
    return InstructionType::BoolOperation;
}

tempo_utils::Status
lyric_assembler::BoolOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::BoolOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_BOOL_CMP:
            return bytecodeBuilder.writeOpcode(m_opcode);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::BoolOperationInstruction::toString() const
{
    return absl::StrCat("Bool Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode));
}

lyric_object::Opcode
lyric_assembler::BoolOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::IntOperationInstruction::IntOperationInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::IntOperationInstruction::getType() const
{
    return InstructionType::IntOperation;
}

tempo_utils::Status
lyric_assembler::IntOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::IntOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_I64_ADD:
        case lyric_object::Opcode::OP_I64_SUB:
        case lyric_object::Opcode::OP_I64_MUL:
        case lyric_object::Opcode::OP_I64_DIV:
        case lyric_object::Opcode::OP_I64_NEG:
        case lyric_object::Opcode::OP_I64_CMP:
            return bytecodeBuilder.writeOpcode(m_opcode);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::IntOperationInstruction::toString() const
{
    return absl::StrCat("Int Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode));
}

lyric_object::Opcode
lyric_assembler::IntOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::FloatOperationInstruction::FloatOperationInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::FloatOperationInstruction::getType() const
{
    return InstructionType::FloatOperation;
}

tempo_utils::Status
lyric_assembler::FloatOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::FloatOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_DBL_ADD:
        case lyric_object::Opcode::OP_DBL_SUB:
        case lyric_object::Opcode::OP_DBL_MUL:
        case lyric_object::Opcode::OP_DBL_DIV:
        case lyric_object::Opcode::OP_DBL_NEG:
        case lyric_object::Opcode::OP_DBL_CMP:
            return bytecodeBuilder.writeOpcode(m_opcode);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::FloatOperationInstruction::toString() const
{
    return absl::StrCat("Float Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode));
}

lyric_object::Opcode
lyric_assembler::FloatOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::CharOperationInstruction::CharOperationInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::CharOperationInstruction::getType() const
{
    return InstructionType::CharOperation;
}

tempo_utils::Status
lyric_assembler::CharOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::CharOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_CHR_CMP:
            return bytecodeBuilder.writeOpcode(m_opcode);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::CharOperationInstruction::toString() const
{
    return absl::StrCat("Char Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode));
}

lyric_object::Opcode
lyric_assembler::CharOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::LogicalOperationInstruction::LogicalOperationInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::LogicalOperationInstruction::getType() const
{
    return InstructionType::LogicalOperation;
}

tempo_utils::Status
lyric_assembler::LogicalOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::LogicalOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_LOGICAL_AND:
        case lyric_object::Opcode::OP_LOGICAL_OR:
        case lyric_object::Opcode::OP_LOGICAL_NOT:
            return bytecodeBuilder.writeOpcode(m_opcode);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::LogicalOperationInstruction::toString() const
{
    return absl::StrCat("Logical Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode));
}

lyric_object::Opcode
lyric_assembler::LogicalOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::BitwiseOperationInstruction::BitwiseOperationInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::BitwiseOperationInstruction::getType() const
{
    return InstructionType::BitwiseOperation;
}

tempo_utils::Status
lyric_assembler::BitwiseOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::BitwiseOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_BITWISE_AND:
        case lyric_object::Opcode::OP_BITWISE_OR:
        case lyric_object::Opcode::OP_BITWISE_XOR:
        case lyric_object::Opcode::OP_BITWISE_LEFT_SHIFT:
        case lyric_object::Opcode::OP_BITWISE_RIGHT_SHIFT:
            return bytecodeBuilder.writeOpcode(m_opcode);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::BitwiseOperationInstruction::toString() const
{
    return absl::StrCat("Logical Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode));
}

lyric_object::Opcode
lyric_assembler::BitwiseOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::TypeOperationInstruction::TypeOperationInstruction(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::TypeOperationInstruction::getType() const
{
    return InstructionType::TypeOperation;
}

tempo_utils::Status
lyric_assembler::TypeOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::TypeOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_TYPE_CMP:
        case lyric_object::Opcode::OP_TYPE_OF:
            return bytecodeBuilder.writeOpcode(m_opcode);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::TypeOperationInstruction::toString() const
{
    return absl::StrCat("Type Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode));
}

lyric_object::Opcode
lyric_assembler::TypeOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::StackOperationInstruction::StackOperationInstruction(
    lyric_object::Opcode opcode,
    tu_uint16 offset)
    : m_opcode(opcode),
      m_offset(offset)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::StackOperationInstruction::getType() const
{
    return InstructionType::StackOperation;
}

tempo_utils::Status
lyric_assembler::StackOperationInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::StackOperationInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_POP:
        case lyric_object::Opcode::OP_DUP:
            return bytecodeBuilder.writeOpcode(m_opcode);
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

std::string
lyric_assembler::StackOperationInstruction::toString() const
{
    return absl::StrCat("Stack Operation: opcode=",
        lyric_object::opcode_to_name(m_opcode),
        " offset=", m_offset);
}

lyric_object::Opcode
lyric_assembler::StackOperationInstruction::getOpcode() const
{
    return m_opcode;
}

lyric_assembler::LoadLiteralInstruction::LoadLiteralInstruction(
    lyric_object::Opcode opcode,
    LiteralHandle *literalHandle)
    : m_opcode(opcode),
      m_literal(literalHandle)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
    TU_ASSERT (m_literal != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::LoadLiteralInstruction::getType() const
{
    return InstructionType::LoadLiteral;
}

tempo_utils::Status
lyric_assembler::LoadLiteralInstruction::touch(ObjectWriter &writer) const
{
    return writer.touchLiteral(m_literal);
}

tempo_utils::Status
lyric_assembler::LoadLiteralInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    switch (m_opcode) {
        case lyric_object::Opcode::OP_LITERAL:
        case lyric_object::Opcode::OP_STRING:
        case lyric_object::Opcode::OP_URL: {
            tu_uint32 address;
            TU_ASSIGN_OR_RETURN (address, writer.getLiteralAddress(m_literal));
            TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(m_opcode));
            return bytecodeBuilder.writeU32(address);
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::LoadLiteralInstruction::toString() const
{
    return absl::StrCat("Load Literal: opcode=", lyric_object::opcode_to_name(m_opcode));
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
lyric_assembler::LoadDataInstruction::touch(ObjectWriter &writer) const
{
    switch (m_symbol->getSymbolType()) {
        case SymbolType::ENUM:
            return writer.touchEnum(cast_symbol_to_enum(m_symbol));
        case SymbolType::FIELD:
            return writer.touchField(cast_symbol_to_field(m_symbol));
        case SymbolType::INSTANCE:
            return writer.touchInstance(cast_symbol_to_instance(m_symbol));
        case SymbolType::STATIC:
            return writer.touchStatic(cast_symbol_to_static(m_symbol));
        default:
            return {};
    }
}

tempo_utils::Status
lyric_assembler::LoadDataInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
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
        case SymbolType::ENUM:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Enum));
            flags = lyric_object::LOAD_ENUM;
            break;
        case SymbolType::FIELD:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Field));
            flags = lyric_object::LOAD_FIELD;
            break;
        case SymbolType::INSTANCE:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Instance));
            flags = lyric_object::LOAD_INSTANCE;
            break;
        case SymbolType::STATIC:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Static));
            flags = lyric_object::LOAD_STATIC;
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

std::string
lyric_assembler::LoadDataInstruction::toString() const
{
    return absl::StrCat("Load Data: symbol=", m_symbol->getSymbolUrl().toString());
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
lyric_assembler::LoadDescriptorInstruction::touch(ObjectWriter &writer) const
{
    switch (m_symbol->getSymbolType()) {
        case SymbolType::ACTION:
            return writer.touchAction(cast_symbol_to_action(m_symbol));
        case SymbolType::CALL:
            return writer.touchCall(cast_symbol_to_call(m_symbol));
        case SymbolType::CLASS:
            return writer.touchClass(cast_symbol_to_class(m_symbol));
        case SymbolType::CONCEPT:
            return writer.touchConcept(cast_symbol_to_concept(m_symbol));
        case SymbolType::ENUM:
            return writer.touchEnum(cast_symbol_to_enum(m_symbol));
        case SymbolType::EXISTENTIAL:
            return writer.touchExistential(cast_symbol_to_existential(m_symbol));
        case SymbolType::FIELD:
            return writer.touchField(cast_symbol_to_field(m_symbol));
        case SymbolType::INSTANCE:
            return writer.touchInstance(cast_symbol_to_instance(m_symbol));
        case SymbolType::NAMESPACE:
            return writer.touchNamespace(cast_symbol_to_namespace(m_symbol));
        case SymbolType::STRUCT:
            return writer.touchStruct(cast_symbol_to_struct(m_symbol));
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant,
                "cannot touch descriptor for {}", m_symbol->getSymbolUrl().toString());
    }
}

tempo_utils::Status
lyric_assembler::LoadDescriptorInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    tu_uint32 address;
    lyric_object::LinkageSection section;

    switch (m_symbol->getSymbolType()) {
        case SymbolType::ACTION:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Action));
            section = lyric_object::LinkageSection::Action;
            break;
        case SymbolType::CALL:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Call));
            section = lyric_object::LinkageSection::Call;
            break;
        case SymbolType::CLASS:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Class));
            section = lyric_object::LinkageSection::Class;
            break;
        case SymbolType::CONCEPT:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Concept));
            section = lyric_object::LinkageSection::Concept;
            break;
        case SymbolType::ENUM:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Enum));
            section = lyric_object::LinkageSection::Enum;
            break;
        case SymbolType::EXISTENTIAL:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Existential));
            section = lyric_object::LinkageSection::Existential;
            break;
        case SymbolType::FIELD:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Field));
            section = lyric_object::LinkageSection::Field;
            break;
        case SymbolType::INSTANCE:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Instance));
            section = lyric_object::LinkageSection::Instance;
            break;
        case SymbolType::NAMESPACE:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Namespace));
            section = lyric_object::LinkageSection::Namespace;
            break;
        case SymbolType::STRUCT:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Struct));
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

std::string
lyric_assembler::LoadDescriptorInstruction::toString() const
{
    return absl::StrCat("Load Descriptor: symbol=", m_symbol->getSymbolUrl().toString());
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
lyric_assembler::LoadSyntheticInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::LoadSyntheticInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_SYNTHETIC));
    switch (m_type) {
        case SyntheticType::This:
            return bytecodeBuilder.writeU8(lyric_object::SYNTHETIC_THIS);
        case SyntheticType::Rest:
            return bytecodeBuilder.writeU8(lyric_object::SYNTHETIC_REST);
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid synthetic type");
   }
}

std::string
lyric_assembler::LoadSyntheticInstruction::toString() const
{
    switch (m_type) {
        case SyntheticType::This:
            return "Load 'this' synthetic reference";
        case SyntheticType::Rest:
            return "Load 'rest' synthetic reference";
        default:
            return absl::StrCat("Load invalid synthentic reference of type ", (int) m_type);
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
lyric_assembler::LoadTypeInstruction::touch(ObjectWriter &writer) const
{
    return writer.touchType(m_typeHandle);
}

tempo_utils::Status
lyric_assembler::LoadTypeInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    tu_uint32 offset;
    TU_ASSIGN_OR_RETURN (offset, writer.getTypeOffset(m_typeHandle->getTypeDef()));

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_DESCRIPTOR));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8(
        lyric_object::linkage_to_descriptor_section(lyric_object::LinkageSection::Type)));
    return bytecodeBuilder.writeU32(offset);
}

std::string
lyric_assembler::LoadTypeInstruction::toString() const
{
    return absl::StrCat("Load Type: typeDef=", m_typeHandle->getTypeDef().toString());
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
lyric_assembler::StoreDataInstruction::touch(ObjectWriter &writer) const
{
    switch (m_symbol->getSymbolType()) {
        case SymbolType::FIELD:
            return writer.touchField(cast_symbol_to_field(m_symbol));
        case SymbolType::STATIC:
            return writer.touchStatic(cast_symbol_to_static(m_symbol));
        default:
            return {};
    }
}

tempo_utils::Status
lyric_assembler::StoreDataInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
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
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Field));
            flags = lyric_object::STORE_FIELD;
            break;
        case SymbolType::STATIC:
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Static));
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

std::string
lyric_assembler::StoreDataInstruction::toString() const
{
    return absl::StrCat("Store Data: symbol=", m_symbol->getSymbolUrl().toString());
}

lyric_assembler::AbstractSymbol *
lyric_assembler::StoreDataInstruction::getSymbol() const
{
    return m_symbol;
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
lyric_assembler::LabelInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::LabelInstruction::apply(
    const ObjectWriter &writer,
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
lyric_assembler::LabelInstruction::toString() const
{
    return absl::StrCat("Label: ", m_name);
}

std::string
lyric_assembler::LabelInstruction::getName() const
{
    return m_name;
}

lyric_assembler::JumpInstruction::JumpInstruction(tu_uint32 targetId)
    : m_targetId(targetId)
{
}

lyric_assembler::InstructionType
lyric_assembler::JumpInstruction::getType() const
{
    return InstructionType::Jump;
}

tempo_utils::Status
lyric_assembler::JumpInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::JumpInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    targetId = m_targetId;
    return bytecodeBuilder.jump(patchOffset);
}

std::string
lyric_assembler::JumpInstruction::toString() const
{
    return absl::StrCat("Jump: targetId=", m_targetId);
}

lyric_object::Opcode
lyric_assembler::JumpInstruction::getOpcode() const
{
    return lyric_object::Opcode::OP_JUMP;
}

tu_uint32
lyric_assembler::JumpInstruction::getTargetId() const
{
    return m_targetId;
}

lyric_assembler::BranchInstruction::BranchInstruction(lyric_object::Opcode opcode, tu_uint32 targetId)
    : m_opcode(opcode),
      m_targetId(targetId)
{
    TU_ASSERT (m_opcode != lyric_object::Opcode::OP_UNKNOWN);
}

lyric_assembler::InstructionType
lyric_assembler::BranchInstruction::getType() const
{
    return InstructionType::Branch;
}

tempo_utils::Status
lyric_assembler::BranchInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::BranchInstruction::apply(
    const ObjectWriter &writer,
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
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
}

std::string
lyric_assembler::BranchInstruction::toString() const
{
    return absl::StrCat("Branch: opcode=",
        lyric_object::opcode_to_name(m_opcode),
        " targetId=", m_targetId);
}

lyric_object::Opcode
lyric_assembler::BranchInstruction::getOpcode() const
{
    return m_opcode;
}

tu_uint32
lyric_assembler::BranchInstruction::getTargetId() const
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
lyric_assembler::CallInstruction::touch(ObjectWriter &writer) const
{
    switch (m_symbol->getSymbolType()) {
        case SymbolType::ACTION:
            return writer.touchAction(cast_symbol_to_action(m_symbol));
        case SymbolType::CALL:
            return writer.touchCall(cast_symbol_to_call(m_symbol));
        default:
            return {};
    }
}

tempo_utils::Status
lyric_assembler::CallInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    tu_uint32 address;

    switch (m_opcode) {
        case lyric_object::Opcode::OP_CALL_VIRTUAL: {
            auto *callSymbol = cast_symbol_to_call(m_symbol);
            auto *virtualCall = callSymbol->virtualCall();
            if (virtualCall != nullptr) {
                TU_ASSIGN_OR_RETURN (address,
                    writer.getSectionAddress(virtualCall->getSymbolUrl(), lyric_object::LinkageSection::Call));
            } else {
                TU_ASSIGN_OR_RETURN (address,
                    writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Call));
            }
            break;
        }
        case lyric_object::Opcode::OP_CALL_EXISTENTIAL:
        case lyric_object::Opcode::OP_CALL_STATIC: {
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Call));
            break;
        }
        case lyric_object::Opcode::OP_CALL_CONCEPT: {
            TU_ASSIGN_OR_RETURN (address,
                writer.getSectionAddress(m_symbol->getSymbolUrl(), lyric_object::LinkageSection::Action));
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

std::string
lyric_assembler::CallInstruction::toString() const
{
    return absl::StrCat("Call: opcode=", lyric_object::opcode_to_name(m_opcode),
        " symbol=", m_symbol->getSymbolUrl().toString(),
        " placement=", m_placement,
        " flags=", m_flags);
}

lyric_assembler::ReturnInstruction::ReturnInstruction()
{
}

lyric_assembler::InstructionType
lyric_assembler::ReturnInstruction::getType() const
{
    return InstructionType::Return;
}

tempo_utils::Status
lyric_assembler::ReturnInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::ReturnInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_RETURN);
}

std::string
lyric_assembler::ReturnInstruction::toString() const
{
    return "Return";
}

lyric_assembler::NewInstruction::NewInstruction(
    CallSymbol *ctorSymbol,
    tu_uint16 placement,
    tu_uint8 flags)
    : m_ctorSymbol(ctorSymbol),
      m_placement(placement),
      m_flags(flags)
{
    TU_ASSERT (m_ctorSymbol != nullptr);
}

lyric_assembler::InstructionType
lyric_assembler::NewInstruction::getType() const
{
    return InstructionType::New;
}

tempo_utils::Status
lyric_assembler::NewInstruction::touch(ObjectWriter &writer) const
{
    return writer.touchConstructor(m_ctorSymbol->getSymbolUrl());
}

tempo_utils::Status
lyric_assembler::NewInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    tu_uint8 newType = 0;

    tu_uint32 address;
    TU_ASSIGN_OR_RETURN (address, writer.getSectionAddress(
        m_ctorSymbol->getSymbolUrl(), lyric_object::LinkageSection::Call));

    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_NEW));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU8((newType << 4u) | m_flags));
    TU_RETURN_IF_NOT_OK (bytecodeBuilder.writeU32(address));
    return bytecodeBuilder.writeU16(m_placement);
}

std::string
lyric_assembler::NewInstruction::toString() const
{
    return absl::StrCat("New: ctor=", m_ctorSymbol->getSymbolUrl().toString(),
        " placement=", m_placement,
        " flags=", m_flags);
}

lyric_assembler::TrapInstruction::TrapInstruction(
    const lyric_common::ModuleLocation &pluginLocation,
    std::string_view trapName,
    tu_uint8 flags)
    : m_pluginLocation(pluginLocation),
      m_trapName(trapName),
      m_flags(flags)
{
    TU_ASSERT (m_pluginLocation.isValid());
    TU_ASSERT (!m_trapName.empty());
}

lyric_assembler::InstructionType
lyric_assembler::TrapInstruction::getType() const
{
    return InstructionType::Trap;
}

tempo_utils::Status
lyric_assembler::TrapInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::TrapInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    tu_uint32 trapNumber;
    TU_ASSIGN_OR_RETURN (trapNumber, writer.getTrapNumber(m_pluginLocation, m_trapName));
    return bytecodeBuilder.trap(trapNumber, m_flags);
}

std::string
lyric_assembler::TrapInstruction::toString() const
{
    return absl::StrCat("Trap: trapName=", m_trapName, " flags=", m_flags);
}

lyric_assembler::InstructionType
lyric_assembler::VaLoadInstruction::getType() const
{
    return InstructionType::VaLoad;
}

tempo_utils::Status
lyric_assembler::VaLoadInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::VaLoadInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_VA_LOAD);
}

std::string
lyric_assembler::VaLoadInstruction::toString() const
{
    return "Opcode: OP_VA_LOAD";
}

lyric_assembler::InstructionType
lyric_assembler::VaSizeInstruction::getType() const
{
    return InstructionType::VaSize;
}

tempo_utils::Status
lyric_assembler::VaSizeInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::VaSizeInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_VA_SIZE);
}

std::string
lyric_assembler::VaSizeInstruction::toString() const
{
    return "Opcode: OP_VA_SIZE";
}

lyric_assembler::InstructionType
lyric_assembler::InterruptInstruction::getType() const
{
    return InstructionType::Interrupt;
}

tempo_utils::Status
lyric_assembler::InterruptInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::InterruptInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_INTERRUPT);
}

std::string
lyric_assembler::InterruptInstruction::toString() const
{
    return "Opcode: OP_INTERRUPT";
}

lyric_assembler::InstructionType
lyric_assembler::HaltInstruction::getType() const
{
    return InstructionType::Halt;
}

tempo_utils::Status
lyric_assembler::HaltInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::HaltInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_HALT);
}

std::string
lyric_assembler::HaltInstruction::toString() const
{
    return "Opcode: OP_HALT";
}

lyric_assembler::InstructionType
lyric_assembler::AbortInstruction::getType() const
{
    return InstructionType::Abort;
}

tempo_utils::Status
lyric_assembler::AbortInstruction::touch(ObjectWriter &writer) const
{
    return {};
}

tempo_utils::Status
lyric_assembler::AbortInstruction::apply(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    std::string &labelName,
    tu_uint16 &labelOffset,
    tu_uint32 &targetId,
    tu_uint16 &patchOffset) const
{
    return bytecodeBuilder.writeOpcode(lyric_object::Opcode::OP_ABORT);
}

std::string
lyric_assembler::AbortInstruction::toString() const
{
    return "Opcode: OP_ABORT";
}
