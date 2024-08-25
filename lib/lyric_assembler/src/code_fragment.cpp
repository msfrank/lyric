
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
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/proc_builder.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/synthetic_symbol.h>

lyric_assembler::CodeFragment::CodeFragment(ProcBuilder *procBuilder)
    : m_procBuilder(procBuilder)
{
    TU_ASSERT (m_procBuilder != nullptr);
}

std::unique_ptr<lyric_assembler::CodeFragment>
lyric_assembler::CodeFragment::makeFragment()
{
    return std::unique_ptr<CodeFragment>(new CodeFragment(m_procBuilder));
}

tempo_utils::Status
lyric_assembler::CodeFragment::appendFragment(std::unique_ptr<CodeFragment> &&fragment)
{
    Statement statement;
    statement.type = StatementType::Fragment;
    statement.fragment = std::move(fragment);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::noOperation()
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<NoOperandsInstruction>(lyric_object::Opcode::OP_NOOP);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateNil()
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<NoOperandsInstruction>(lyric_object::Opcode::OP_NIL);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateUndef()
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<NoOperandsInstruction>(lyric_object::Opcode::OP_UNDEF);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateBool(bool b)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<BoolImmediateInstruction>(b);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateInt(int64_t i64)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<IntImmediateInstruction>(i64);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateFloat(double dbl)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<FloatImmediateInstruction>(dbl);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateChar(UChar32 chr)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<CharImmediateInstruction>(chr);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadString(const std::string &str)
{
    auto *state = m_procBuilder->objectState();
    auto *literalCache = state->literalCache();
    LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(str));

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<LoadLiteralInstruction>(lyric_object::Opcode::OP_STRING, address);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadUrl(const tempo_utils::Url &url)
{
    auto *state = m_procBuilder->objectState();
    auto *literalCache = state->literalCache();
    LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(url.toString()));

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<LoadLiteralInstruction>(lyric_object::Opcode::OP_URL, address);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadRef(const DataReference &ref)
{
    auto *state = m_procBuilder->objectState();
    auto *symbolCache = state->symbolCache();

    Statement statement;
    statement.type = StatementType::Instruction;

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));
    switch (ref.referenceType) {
        case ReferenceType::Descriptor:
            statement.instruction = std::make_shared<LoadDescriptorInstruction>(symbol);
            break;
        case ReferenceType::Value:
        case ReferenceType::Variable:
            statement.instruction = std::make_shared<LoadDataInstruction>(symbol);
            break;
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid data reference");
    }

    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadType(const lyric_common::TypeDef &loadType)
{
    auto *state = m_procBuilder->objectState();
    auto *typeCache = state->typeCache();

    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(loadType));

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<LoadTypeInstruction>(typeHandle);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::storeRef(const DataReference &ref)
{
    auto *state = m_procBuilder->objectState();
    auto *symbolCache = state->symbolCache();

    Statement statement;
    statement.type = StatementType::Instruction;

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));
    switch (ref.referenceType) {
        case ReferenceType::Variable:
            statement.instruction = std::make_shared<StoreDataInstruction>(symbol);
            break;
        case ReferenceType::Descriptor:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kInvalidBinding, "cannot store to descriptor {}", ref.symbolUrl.toString());
        case ReferenceType::Value:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kInvalidBinding, "invalid data to value {}", ref.symbolUrl.toString());
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid data reference");
    }

    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::popValue()
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_POP, 0);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::dupValue()
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_DUP, 0);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::pickValue(tu_uint16 pickOffset)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_PICK, pickOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::dropValue(tu_uint16 dropOffset)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_DROP, dropOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::rpickValue(tu_uint16 pickOffset)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_RPICK, pickOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::rdropValue(tu_uint16 dropOffset)
{
    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_RDROP, dropOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::makeJump(lyric_object::Opcode opcode)
{
    tu_uint32 targetId;
    TU_ASSIGN_OR_RETURN (targetId, m_procBuilder->makeJump());

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<JumpInstruction>(opcode, targetId);
    m_statements.push_back(std::move(statement));
    return targetId;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::unconditionalJump()
{
    return makeJump(lyric_object::Opcode::OP_JUMP);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfNil()
{
    return makeJump(lyric_object::Opcode::OP_IF_NIL);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfNotNil()
{
    return makeJump(lyric_object::Opcode::OP_IF_NOTNIL);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfTrue()
{
    return makeJump(lyric_object::Opcode::OP_IF_TRUE);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfFalse()
{
    return makeJump(lyric_object::Opcode::OP_IF_FALSE);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfZero()
{
    return makeJump(lyric_object::Opcode::OP_IF_ZERO);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfNotZero()
{
    return makeJump(lyric_object::Opcode::OP_IF_NOTZERO);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfLessThan()
{
    return makeJump(lyric_object::Opcode::OP_IF_LT);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfLessOrEqual()
{
    return makeJump(lyric_object::Opcode::OP_IF_LE);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfGreaterThan()
{
    return makeJump(lyric_object::Opcode::OP_IF_GT);
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::jumpIfGreaterOrEqual()
{
    return makeJump(lyric_object::Opcode::OP_IF_GE);
}

tempo_utils::Status
lyric_assembler::CodeFragment::appendLabel(std::string_view labelName)
{
    TU_ASSERT (!labelName.empty());
    TU_RETURN_IF_NOT_OK (m_procBuilder->appendLabel(labelName));

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<LabelInstruction>(labelName);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::patchTarget(tu_uint32 targetId, std::string_view labelName)
{
    TU_ASSERT (!labelName.empty());
    return m_procBuilder->patchTarget(targetId, labelName);
}

tempo_utils::Status
lyric_assembler::CodeFragment::callStatic(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags)
{
    TU_ASSERT (callSymbol != nullptr);
    if (callSymbol->isBound() || callSymbol->isCtor())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for static call");

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<CallInstruction>(
        lyric_object::Opcode::OP_CALL_STATIC, callSymbol, placement, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::callVirtual(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags)
{
    TU_ASSERT (callSymbol != nullptr);
    if (!callSymbol->isBound() || callSymbol->isCtor())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for virtual call");

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<CallInstruction>(
        lyric_object::Opcode::OP_CALL_VIRTUAL, callSymbol, placement, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::callConcept(ActionSymbol *actionSymbol, tu_uint16 placement, tu_uint8 flags)
{
    TU_ASSERT (actionSymbol != nullptr);

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<CallInstruction>(
        lyric_object::Opcode::OP_CALL_CONCEPT, actionSymbol, placement, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::callExistential(
    ExistentialSymbol *existentialSymbol,
    tu_uint16 placement,
    tu_uint8 flags)
{
    TU_ASSERT (existentialSymbol != nullptr);

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<CallInstruction>(
        lyric_object::Opcode::OP_CALL_EXISTENTIAL, existentialSymbol, placement, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::callInline(CallSymbol *callSymbol)
{
    TU_ASSERT (callSymbol != nullptr);
    if (!callSymbol->isInline())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for inline call");

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<InlineInstruction>(callSymbol);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::constructNew(AbstractSymbol *newSymbol, tu_uint16 placement, tu_uint8 flags)
{
    TU_ASSERT (newSymbol != nullptr);

    Statement statement;
    statement.type = StatementType::Instruction;

    switch (newSymbol->getSymbolType()) {
        case SymbolType::CLASS:
        case SymbolType::ENUM:
        case SymbolType::INSTANCE:
        case SymbolType::STRUCT: {
            statement.instruction = std::make_shared<NewInstruction>(newSymbol, placement, flags);
            break;
        }
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid new symbol");
    }

    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::trap(tu_uint32 trapNumber, tu_uint8 flags)
{

    Statement statement;
    statement.type = StatementType::Instruction;
    statement.instruction = std::make_shared<TrapInstruction>(trapNumber, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::build(
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    absl::flat_hash_map<std::string,tu_uint16> &labelOffsets,
    absl::flat_hash_map<tu_uint32,tu_uint16> &patchOffsets) const
{
    for (auto &statement : m_statements) {

        switch (statement.type) {
            case StatementType::Fragment: {
                TU_RETURN_IF_NOT_OK (statement.fragment->build(bytecodeBuilder, labelOffsets, patchOffsets));
                break;
            }

            case StatementType::Instruction: {
                auto instruction = statement.instruction;

                std::string labelName;
                tu_uint16 labelOffset;
                tu_uint32 targetId;
                tu_uint16 patchOffset;
                TU_RETURN_IF_NOT_OK (instruction->apply(bytecodeBuilder, labelName, labelOffset, targetId, patchOffset));

                auto instructionType = instruction->getType();
                switch (instructionType) {
                    case InstructionType::Label: {
                        TU_ASSERT (!labelName.empty());
                        labelOffsets[labelName] = labelOffset;
                        break;
                    }
                    case InstructionType::Jump: {
                        patchOffsets[targetId] = patchOffset;
                        break;
                    }
                    default:
                        break;
                }
                break;
            }

            default:
                break;
        }
    }

    return {};
}