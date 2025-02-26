
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

tempo_utils::Status
lyric_assembler::CodeFragment::insertInstruction(int index, std::shared_ptr<AbstractInstruction> instruction)
{
    if (index >= m_statements.size())
        return appendInstruction(instruction);

    auto it = m_statements.begin();
    std::advance(it, index);

    Statement statement;
    statement.instruction = instruction;
    m_statements.insert(it, std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::appendInstruction(std::shared_ptr<AbstractInstruction> instruction)
{
    Statement statement;
    statement.instruction = instruction;
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Result<lyric_assembler::JumpLabel>
lyric_assembler::CodeFragment::insertLabel(int index, std::string_view userLabel)
{
    if (index >= m_statements.size())
        return appendLabel(userLabel);

    std::string labelName;
    TU_ASSIGN_OR_RETURN (labelName, m_procBuilder->makeLabel(userLabel));

    auto it = m_statements.begin();
    std::advance(it, index);

    Statement statement;
    statement.instruction = std::make_shared<LabelInstruction>(labelName);
    m_statements.insert(it, std::move(statement));
    return JumpLabel(labelName);
}

tempo_utils::Result<lyric_assembler::JumpLabel>
lyric_assembler::CodeFragment::appendLabel(std::string_view userLabel)
{
    std::string labelName;
    TU_ASSIGN_OR_RETURN (labelName, m_procBuilder->makeLabel(userLabel));

    Statement statement;
    statement.instruction = std::make_shared<LabelInstruction>(labelName);
    m_statements.push_back(std::move(statement));
    return JumpLabel(labelName);
}

std::unique_ptr<lyric_assembler::CodeFragment>
lyric_assembler::CodeFragment::makeFragment()
{
    return std::unique_ptr<CodeFragment>(new CodeFragment(m_procBuilder));
}

tempo_utils::Status
lyric_assembler::CodeFragment::insertFragment(int index, std::unique_ptr<CodeFragment> &&fragment)
{
    TU_ASSERT (fragment != nullptr);

    if (index >= m_statements.size())
        return appendFragment(std::move(fragment));

    auto it = m_statements.begin();
    std::advance(it, index);

    auto &statements = fragment->m_statements;
    m_statements.insert(it, statements.begin(), statements.end());

    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::appendFragment(std::unique_ptr<CodeFragment> &&fragment)
{
    TU_ASSERT (fragment != nullptr);

    auto &statements = fragment->m_statements;
    m_statements.insert(m_statements.end(), statements.begin(), statements.end());

    return {};
}

const lyric_assembler::Statement&
lyric_assembler::CodeFragment::getStatement(int index) const
{
    return m_statements.at(index);
}

void
lyric_assembler::CodeFragment::removeStatement(int index)
{
    auto it = m_statements.begin();
    std::advance(it, index);
    m_statements.erase(it);
}

std::vector<lyric_assembler::Statement>::const_iterator
lyric_assembler::CodeFragment::statementsBegin() const
{
    return m_statements.cbegin();
}

std::vector<lyric_assembler::Statement>::const_iterator
lyric_assembler::CodeFragment::statementsEnd() const
{
    return m_statements.cend();
}

int
lyric_assembler::CodeFragment::numStatements() const
{
    return m_statements.size();
}

tempo_utils::Status
lyric_assembler::CodeFragment::noOperation()
{
    Statement statement;
    statement.instruction = std::make_shared<NoopInstruction>();
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateNil()
{
    Statement statement;
    statement.instruction = std::make_shared<NilImmediateInstruction>();
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateUndef()
{
    Statement statement;
    statement.instruction = std::make_shared<UndefImmediateInstruction>();
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateBool(bool b)
{
    Statement statement;
    statement.instruction = std::make_shared<BoolImmediateInstruction>(b);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateInt(int64_t i64)
{
    Statement statement;
    statement.instruction = std::make_shared<IntImmediateInstruction>(i64);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateFloat(double dbl)
{
    Statement statement;
    statement.instruction = std::make_shared<FloatImmediateInstruction>(dbl);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateChar(UChar32 chr)
{
    Statement statement;
    statement.instruction = std::make_shared<CharImmediateInstruction>(chr);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadString(const std::string &str)
{
    auto *state = m_procBuilder->objectState();
    auto *literalCache = state->literalCache();
    LiteralHandle *literal;
    TU_ASSIGN_OR_RETURN (literal, literalCache->makeUtf8(str));

    Statement statement;
    statement.instruction = std::make_shared<LoadLiteralInstruction>(lyric_object::Opcode::OP_STRING, literal);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadUrl(const tempo_utils::Url &url)
{
    auto *state = m_procBuilder->objectState();
    auto *literalCache = state->literalCache();
    LiteralHandle *literal;
    TU_ASSIGN_OR_RETURN (literal, literalCache->makeUtf8(url.toString()));

    Statement statement;
    statement.instruction = std::make_shared<LoadLiteralInstruction>(lyric_object::Opcode::OP_URL, literal);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadData(AbstractSymbol *symbol)
{
    Statement statement;
    statement.instruction = std::make_shared<LoadDataInstruction>(symbol);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadDescriptor(AbstractSymbol *symbol)
{
    Statement statement;
    statement.instruction = std::make_shared<LoadDescriptorInstruction>(symbol);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadRef(const DataReference &ref)
{
    auto *state = m_procBuilder->objectState();
    auto *symbolCache = state->symbolCache();

    Statement statement;

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));

    if (symbol->getSymbolType() == SymbolType::SYNTHETIC) {
        auto *syntheticSymbol = cast_symbol_to_synthetic(symbol);
        statement.instruction = std::make_shared<LoadSyntheticInstruction>(syntheticSymbol->getSyntheticType());
    } else {
        switch (ref.referenceType) {
            case ReferenceType::Namespace:
                statement.instruction = std::make_shared<LoadDescriptorInstruction>(symbol);
                break;
            case ReferenceType::Value:
            case ReferenceType::Variable:
                statement.instruction = std::make_shared<LoadDataInstruction>(symbol);
                break;
            case ReferenceType::Descriptor:
                if (symbol->getSymbolType() == SymbolType::STATIC) {
                    statement.instruction = std::make_shared<LoadDataInstruction>(symbol);
                } else {
                    statement.instruction = std::make_shared<LoadDescriptorInstruction>(symbol);
                }
                break;
            default:
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid data reference");
        }
    }

    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadRef(const ImplReference &ref)
{
    auto *state = m_procBuilder->objectState();
    auto *symbolCache = state->symbolCache();

    Statement statement;

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.usingRef.symbolUrl));

    if (symbol->getSymbolType() == SymbolType::SYNTHETIC) {
        auto *syntheticSymbol = cast_symbol_to_synthetic(symbol);
        statement.instruction = std::make_shared<LoadSyntheticInstruction>(syntheticSymbol->getSyntheticType());
    } else {
        statement.instruction = std::make_shared<LoadDataInstruction>(symbol);
    }

    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadThis()
{
    Statement statement;
    statement.instruction = std::make_shared<LoadSyntheticInstruction>(SyntheticType::This);
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
    statement.instruction = std::make_shared<LoadTypeInstruction>(typeHandle);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::storeData(AbstractSymbol *symbol)
{
    Statement statement;
    statement.instruction = std::make_shared<StoreDataInstruction>(symbol);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::storeRef(const DataReference &ref, bool initialStore)
{
    auto *state = m_procBuilder->objectState();
    auto *symbolCache = state->symbolCache();

    Statement statement;

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));
    switch (ref.referenceType) {
        case ReferenceType::Value:
            if (!initialStore)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kInvalidBinding, "cannot store to value {}", ref.symbolUrl.toString());
            statement.instruction = std::make_shared<StoreDataInstruction>(symbol);
            break;
        case ReferenceType::Variable:
            statement.instruction = std::make_shared<StoreDataInstruction>(symbol);
            break;
        case ReferenceType::Descriptor:
            if (symbol->getSymbolType() != SymbolType::STATIC)
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kInvalidBinding, "cannot store to descriptor {}", ref.symbolUrl.toString());
            statement.instruction = std::make_shared<StoreDataInstruction>(symbol);
            break;
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
    statement.instruction = std::make_shared<StackOperationInstruction>(lyric_object::Opcode::OP_POP, 0);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::dupValue()
{
    Statement statement;
    statement.instruction = std::make_shared<StackOperationInstruction>(lyric_object::Opcode::OP_DUP, 0);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::pickValue(tu_uint16 pickOffset)
{
    Statement statement;
    statement.instruction = std::make_shared<StackOperationInstruction>(lyric_object::Opcode::OP_PICK, pickOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::dropValue(tu_uint16 dropOffset)
{
    Statement statement;
    statement.instruction = std::make_shared<StackOperationInstruction>(lyric_object::Opcode::OP_DROP, dropOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::rpickValue(tu_uint16 pickOffset)
{
    Statement statement;
    statement.instruction = std::make_shared<StackOperationInstruction>(lyric_object::Opcode::OP_RPICK, pickOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::rdropValue(tu_uint16 dropOffset)
{
    Statement statement;
    statement.instruction = std::make_shared<StackOperationInstruction>(lyric_object::Opcode::OP_RDROP, dropOffset);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::intAdd()
{
    Statement statement;
    statement.instruction = std::make_shared<IntOperationInstruction>(lyric_object::Opcode::OP_I64_ADD);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::intSubtract()
{
    Statement statement;
    statement.instruction = std::make_shared<IntOperationInstruction>(lyric_object::Opcode::OP_I64_SUB);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::intMultiply()
{
    Statement statement;
    statement.instruction = std::make_shared<IntOperationInstruction>(lyric_object::Opcode::OP_I64_MUL);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::intDivide()
{
    Statement statement;
    statement.instruction = std::make_shared<IntOperationInstruction>(lyric_object::Opcode::OP_I64_DIV);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::intNegate()
{
    Statement statement;
    statement.instruction = std::make_shared<IntOperationInstruction>(lyric_object::Opcode::OP_I64_NEG);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::floatAdd()
{
    Statement statement;
    statement.instruction = std::make_shared<FloatOperationInstruction>(lyric_object::Opcode::OP_DBL_ADD);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::floatSubtract()
{
    Statement statement;
    statement.instruction = std::make_shared<FloatOperationInstruction>(lyric_object::Opcode::OP_DBL_SUB);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::floatMultiply()
{
    Statement statement;
    statement.instruction = std::make_shared<FloatOperationInstruction>(lyric_object::Opcode::OP_DBL_MUL);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::floatDivide()
{
    Statement statement;
    statement.instruction = std::make_shared<FloatOperationInstruction>(lyric_object::Opcode::OP_DBL_DIV);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::floatNegate()
{
    Statement statement;
    statement.instruction = std::make_shared<FloatOperationInstruction>(lyric_object::Opcode::OP_DBL_NEG);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::boolCompare()
{
    Statement statement;
    statement.instruction = std::make_shared<BoolOperationInstruction>(lyric_object::Opcode::OP_BOOL_CMP);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::intCompare()
{
    Statement statement;
    statement.instruction = std::make_shared<IntOperationInstruction>(lyric_object::Opcode::OP_I64_CMP);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::floatCompare()
{
    Statement statement;
    statement.instruction = std::make_shared<FloatOperationInstruction>(lyric_object::Opcode::OP_DBL_CMP);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::charCompare()
{
    Statement statement;
    statement.instruction = std::make_shared<CharOperationInstruction>(lyric_object::Opcode::OP_CHR_CMP);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::typeCompare()
{
    Statement statement;
    statement.instruction = std::make_shared<TypeOperationInstruction>(lyric_object::Opcode::OP_TYPE_CMP);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::logicalAnd()
{
    Statement statement;
    statement.instruction = std::make_shared<LogicalOperationInstruction>(lyric_object::Opcode::OP_LOGICAL_AND);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::logicalOr()
{
    Statement statement;
    statement.instruction = std::make_shared<LogicalOperationInstruction>(lyric_object::Opcode::OP_LOGICAL_OR);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::logicalNot()
{
    Statement statement;
    statement.instruction = std::make_shared<LogicalOperationInstruction>(lyric_object::Opcode::OP_LOGICAL_NOT);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::makeJump(lyric_object::Opcode opcode)
{
    tu_uint32 targetId;
    TU_ASSIGN_OR_RETURN (targetId, m_procBuilder->makeJump());

    Statement statement;
    switch (opcode) {
        case lyric_object::Opcode::OP_JUMP:
            statement.instruction = std::make_shared<JumpInstruction>(targetId);
            break;
        case lyric_object::Opcode::OP_IF_GE:
        case lyric_object::Opcode::OP_IF_GT:
        case lyric_object::Opcode::OP_IF_LE:
        case lyric_object::Opcode::OP_IF_LT:
        case lyric_object::Opcode::OP_IF_TRUE:
        case lyric_object::Opcode::OP_IF_FALSE:
        case lyric_object::Opcode::OP_IF_NIL:
        case lyric_object::Opcode::OP_IF_NOTNIL:
        case lyric_object::Opcode::OP_IF_ZERO:
        case lyric_object::Opcode::OP_IF_NOTZERO:
            statement.instruction = std::make_shared<BranchInstruction>(opcode, targetId);
            break;
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid opcode");
    }
    m_statements.push_back(std::move(statement));
    return JumpTarget(targetId);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::unconditionalJump()
{
    return makeJump(lyric_object::Opcode::OP_JUMP);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfNil()
{
    return makeJump(lyric_object::Opcode::OP_IF_NIL);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfNotNil()
{
    return makeJump(lyric_object::Opcode::OP_IF_NOTNIL);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfTrue()
{
    return makeJump(lyric_object::Opcode::OP_IF_TRUE);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfFalse()
{
    return makeJump(lyric_object::Opcode::OP_IF_FALSE);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfZero()
{
    return makeJump(lyric_object::Opcode::OP_IF_ZERO);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfNotZero()
{
    return makeJump(lyric_object::Opcode::OP_IF_NOTZERO);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfLessThan()
{
    return makeJump(lyric_object::Opcode::OP_IF_LT);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfLessOrEqual()
{
    return makeJump(lyric_object::Opcode::OP_IF_LE);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfGreaterThan()
{
    return makeJump(lyric_object::Opcode::OP_IF_GT);
}

tempo_utils::Result<lyric_assembler::JumpTarget>
lyric_assembler::CodeFragment::jumpIfGreaterOrEqual()
{
    return makeJump(lyric_object::Opcode::OP_IF_GE);
}

tempo_utils::Status
lyric_assembler::CodeFragment::patchTarget(JumpTarget jumpTarget, JumpLabel jumpLabel)
{
    TU_ASSERT (jumpTarget.isValid());
    TU_ASSERT (jumpLabel.isValid());
    return m_procBuilder->patchTarget(jumpTarget.getId(), jumpLabel.labelView());
}

tempo_utils::Status
lyric_assembler::CodeFragment::callStatic(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags)
{
    TU_ASSERT (callSymbol != nullptr);
    if (callSymbol->isBound() || callSymbol->isCtor())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for static call");

    Statement statement;
    statement.instruction = std::make_shared<CallInstruction>(
        lyric_object::Opcode::OP_CALL_STATIC, callSymbol, placement, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::callVirtual(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags)
{
    TU_ASSERT (callSymbol != nullptr);
    if (!callSymbol->isBound())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for virtual call");

    Statement statement;
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
    statement.instruction = std::make_shared<CallInstruction>(
        lyric_object::Opcode::OP_CALL_CONCEPT, actionSymbol, placement, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::callExistential(
    CallSymbol *callSymbol,
    tu_uint16 placement,
    tu_uint8 flags)
{
    TU_ASSERT (callSymbol != nullptr);

    Statement statement;
    statement.instruction = std::make_shared<CallInstruction>(
        lyric_object::Opcode::OP_CALL_EXISTENTIAL, callSymbol, placement, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::constructNew(AbstractSymbol *newSymbol, tu_uint16 placement, tu_uint8 flags)
{
    TU_ASSERT (newSymbol != nullptr);

    Statement statement;

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
    statement.instruction = std::make_shared<TrapInstruction>(trapNumber, flags);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::returnToCaller()
{
    Statement statement;
    statement.instruction = std::make_shared<ReturnInstruction>();
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::invokeTypeOf()
{
    Statement statement;
    statement.instruction = std::make_shared<TypeOperationInstruction>(lyric_object::Opcode::OP_TYPE_OF);
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::invokeInterrupt()
{
    Statement statement;
    statement.instruction = std::make_shared<InterruptInstruction>();
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::invokeHalt()
{
    Statement statement;
    statement.instruction = std::make_shared<HaltInstruction>();
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::invokeAbort()
{
    Statement statement;
    statement.instruction = std::make_shared<AbortInstruction>();
    m_statements.push_back(std::move(statement));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::touch(ObjectWriter &writer) const
{
    for (auto &statement : m_statements) {
        TU_RETURN_IF_NOT_OK (statement.instruction->touch(writer));
    }
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::build(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder,
    absl::flat_hash_map<std::string,tu_uint16> &labelOffsets,
    absl::flat_hash_map<tu_uint32,tu_uint16> &patchOffsets) const
{
    for (auto &statement : m_statements) {
        auto instruction = statement.instruction;

        std::string labelName;
        tu_uint16 labelOffset;
        tu_uint32 targetId;
        tu_uint16 patchOffset;
        TU_RETURN_IF_NOT_OK (
            instruction->apply(writer, bytecodeBuilder, labelName, labelOffset, targetId, patchOffset));

        auto instructionType = instruction->getType();
        switch (instructionType) {
            case InstructionType::Label: {
                TU_ASSERT (!labelName.empty());
                labelOffsets[labelName] = labelOffset;
                break;
            }
            case InstructionType::Branch:
            case InstructionType::Jump: {
                patchOffsets[targetId] = patchOffset;
                break;
            }
            default:
                break;
        }
    }

    return {};
}