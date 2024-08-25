
#include <lyric_assembler/assembler_instructions.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/proc_builder.h>
#include <lyric_assembler/proc_handle.h>

lyric_assembler::ProcBuilder::ProcBuilder(ProcHandle *procHandle)
    : m_procHandle(procHandle)
{
    TU_ASSERT (m_procHandle != nullptr);
    m_rootFragment = std::unique_ptr<CodeFragment>(new CodeFragment(this));
    auto *block = procHandle->procBlock();
    m_state = block->blockState();
}

lyric_assembler::ProcHandle *
lyric_assembler::ProcBuilder::procHandle() const
{
    return m_procHandle;
}

lyric_assembler::CodeFragment *
lyric_assembler::ProcBuilder::rootFragment() const
{
    return m_rootFragment.get();
}

lyric_assembler::ObjectState *
lyric_assembler::ProcBuilder::objectState() const
{
    return m_state;
}

tempo_utils::Status
lyric_assembler::ProcBuilder::appendLabel(std::string_view labelName)
{
    TU_ASSERT (!labelName.empty());

    if (m_labelTargets.contains(labelName))
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "assembly label {} already exists", labelName);
    m_labelTargets[labelName] = {};
    auto label = std::make_shared<LabelInstruction>(labelName);
    return {};
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ProcBuilder::makeJump()
{
    if (m_nextTargetId == std::numeric_limits<tu_uint32>::max())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "too many jump targets");

    auto targetId = m_nextTargetId++;
    m_jumpLabels[targetId] = {};
    return targetId;
}

tempo_utils::Status
lyric_assembler::ProcBuilder::patchTarget(tu_uint32 targetId, std::string_view labelName)
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
lyric_assembler::ProcBuilder::build(lyric_object::BytecodeBuilder &bytecodeBuilder) const
{
    absl::flat_hash_map<std::string,tu_uint16> labelOffsets;
    absl::flat_hash_map<tu_uint32,tu_uint16> patchOffsets;

    TU_RETURN_IF_NOT_OK (m_rootFragment->build(bytecodeBuilder, labelOffsets, patchOffsets));

    for (const auto &entry : patchOffsets) {
        auto targetId = entry.first;
        auto patchOffset = entry.second;
        const auto &jumpLabel = m_jumpLabels.at(targetId);
        auto labelOffset = labelOffsets.at(jumpLabel.name);
        TU_RETURN_IF_NOT_OK (bytecodeBuilder.patch(patchOffset, labelOffset));
    }

    return {};
}
