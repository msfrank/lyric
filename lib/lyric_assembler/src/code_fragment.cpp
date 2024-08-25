
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
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/synthetic_symbol.h>

lyric_assembler::CodeFragment::CodeFragment(ProcHandle *procHandle)
    : m_procHandle(procHandle),
      m_nextTargetId(0)
{
    TU_ASSERT (m_procHandle != nullptr);
}

tempo_utils::Status
lyric_assembler::CodeFragment::noOperation()
{
    m_instructions.emplace_back(std::make_shared<NoOperandsInstruction>(lyric_object::Opcode::OP_NOOP));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateNil()
{
    m_instructions.emplace_back(std::make_shared<NoOperandsInstruction>(lyric_object::Opcode::OP_NIL));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateUndef()
{
    m_instructions.emplace_back(std::make_shared<NoOperandsInstruction>(lyric_object::Opcode::OP_UNDEF));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateBool(bool b)
{
    m_instructions.emplace_back(std::make_shared<BoolImmediateInstruction>(b));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateInt(int64_t i64)
{
    m_instructions.emplace_back(std::make_shared<IntImmediateInstruction>(i64));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateFloat(double dbl)
{
    m_instructions.emplace_back(std::make_shared<FloatImmediateInstruction>(dbl));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::immediateChar(UChar32 chr)
{
    m_instructions.emplace_back(std::make_shared<CharImmediateInstruction>(chr));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadString(const std::string &str)
{
    auto *block = m_procHandle->procBlock();
    auto *state = block->blockState();
    auto *literalCache = state->literalCache();
    LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(str));
    m_instructions.emplace_back(std::make_shared<LoadLiteralInstruction>(lyric_object::Opcode::OP_STRING, address));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadUrl(const tempo_utils::Url &url)
{
    auto *block = m_procHandle->procBlock();
    auto *state = block->blockState();
    auto *literalCache = state->literalCache();
    LiteralAddress address;
    TU_ASSIGN_OR_RETURN (address, literalCache->makeLiteralUtf8(url.toString()));
    m_instructions.emplace_back(std::make_shared<LoadLiteralInstruction>(lyric_object::Opcode::OP_URL, address));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadRef(const DataReference &ref)
{
    auto *block = m_procHandle->procBlock();
    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));
    switch (ref.referenceType) {
        case ReferenceType::Descriptor:
            m_instructions.emplace_back(std::make_shared<LoadDescriptorInstruction>(symbol));
            return {};
        case ReferenceType::Value:
        case ReferenceType::Variable:
            m_instructions.emplace_back(std::make_shared<LoadDataInstruction>(symbol));
            return {};
        default:
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "invalid data reference");
    }
}

tempo_utils::Status
lyric_assembler::CodeFragment::loadType(const lyric_common::TypeDef &loadType)
{
    auto *block = m_procHandle->procBlock();
    auto *state = block->blockState();
    auto *typeCache = state->typeCache();

    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->getOrMakeType(loadType));

    m_instructions.emplace_back(std::make_shared<LoadTypeInstruction>(typeHandle));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::storeRef(const DataReference &ref)
{
    auto *block = m_procHandle->procBlock();
    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(ref.symbolUrl));
    switch (ref.referenceType) {
        case ReferenceType::Variable:
            m_instructions.emplace_back(std::make_shared<StoreDataInstruction>(symbol));
            return {};
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
}

tempo_utils::Status
lyric_assembler::CodeFragment::popValue()
{
    m_instructions.emplace_back(std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_POP, 0));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::dupValue()
{
    m_instructions.emplace_back(std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_DUP, 0));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::pickValue(tu_uint16 pickOffset)
{
    m_instructions.emplace_back(
        std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_PICK, pickOffset));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::dropValue(tu_uint16 dropOffset)
{
    m_instructions.emplace_back(
        std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_DROP, dropOffset));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::rpickValue(tu_uint16 pickOffset)
{
    m_instructions.emplace_back(
        std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_RPICK, pickOffset));
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::rdropValue(tu_uint16 dropOffset)
{
    m_instructions.emplace_back(
        std::make_shared<StackModificationInstruction>(lyric_object::Opcode::OP_RDROP, dropOffset));
    return {};
}

tempo_utils::Result<tu_uint32>
lyric_assembler::CodeFragment::makeJump(lyric_object::Opcode opcode)
{
    if (m_nextTargetId == std::numeric_limits<tu_uint32>::max())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "too many jump targets");

    auto targetId = m_nextTargetId++;
    m_jumpLabels[targetId] = {};

    auto jump = std::make_shared<JumpInstruction>(opcode, targetId);
    m_instructions.push_back(jump);
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

    if (m_labelTargets.contains(labelName))
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "assembly label {} already exists", labelName);
    m_labelTargets[labelName] = {};
    auto label = std::make_shared<LabelInstruction>(labelName);
    m_instructions.push_back(label);
    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::patchTarget(tu_uint32 targetId, std::string_view labelName)
{
    TU_ASSERT (!labelName.empty());

    auto labelTargetEntry = m_labelTargets.find(labelName);
    if (labelTargetEntry == m_labelTargets.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing assembly label {}", labelName);
    auto &labelTargetSet = labelTargetEntry->second;

    auto jumpLabelEntry = m_jumpLabels.find(targetId);
    if (jumpLabelEntry == m_jumpLabels.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing jump target");
    auto &jumpLabel = jumpLabelEntry->second;

    labelTargetSet.targets.insert(targetId);
    jumpLabel.name = labelName;

    return {};
}

tempo_utils::Status
lyric_assembler::CodeFragment::write(lyric_object::BytecodeBuilder &bytecodeBuilder) const
{
    absl::flat_hash_map<std::string,tu_uint16> labelOffsets;
    absl::flat_hash_map<tu_uint32,tu_uint16> patchOffsets;

    for (auto &instruction : m_instructions) {

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
    }

    for (const auto &entry : patchOffsets) {
        auto targetId = entry.first;
        auto patchOffset = entry.second;
        const auto &jumpLabel = m_jumpLabels.at(targetId);
        auto labelOffset = labelOffsets.at(jumpLabel.name);
        TU_RETURN_IF_NOT_OK (bytecodeBuilder.patch(patchOffset, labelOffset));
    }

    return {};
}