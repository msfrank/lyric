
#include <lyric_assembler/assembler_instructions.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/proc_builder.h>
#include <lyric_assembler/proc_handle.h>
#include <tempo_utils/log_stream.h>

lyric_assembler::ProcBuilder::ProcBuilder(ProcHandle *procHandle, ObjectState *state)
    : m_procHandle(procHandle),
      m_state(state)
{
    TU_ASSERT (m_procHandle != nullptr);
    TU_ASSERT (m_state != nullptr);
    m_rootFragment = std::unique_ptr<CodeFragment>(new CodeFragment(this));
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

absl::flat_hash_set<tu_uint32>
lyric_assembler::ProcBuilder::getTargetsForLabel(std::string_view labelName) const
{
    auto entry = m_labelTargets.find(labelName);
    if (entry != m_labelTargets.cend())
        return entry->second.targets;
    return {};
}

std::string
lyric_assembler::ProcBuilder::getLabelForTarget(tu_uint32 targetId) const
{
    auto entry = m_jumpLabels.find(targetId);
    if (entry != m_jumpLabels.cend())
        return entry->second.name;
    return {};
}

tempo_utils::Result<std::string>
lyric_assembler::ProcBuilder::makeLabel(std::string_view userLabel)
{
    std::string labelName(userLabel);
    if (labelName.empty()) {
        if (m_nextId == std::numeric_limits<tu_uint32>::max())
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "too many ids");
        labelName = absl::StrCat("label", m_nextId);
        if (m_labelTargets.contains(labelName))
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "assembly label {} already exists", labelName);
        m_nextId++;
    } else {
        if (m_labelTargets.contains(labelName))
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "assembly label {} already exists", labelName);
    }

    m_labelTargets[labelName] = {};
    TU_LOG_INFO << "proc " << m_procHandle << " makes label " << labelName;
    return labelName;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ProcBuilder::makeJump()
{
    if (m_nextId == std::numeric_limits<tu_uint32>::max())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "too many ids");

    auto targetId = m_nextId++;
    m_jumpLabels[targetId] = {};
    TU_LOG_INFO << "proc " << m_procHandle << " makes jump " << targetId;
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

    TU_LOG_INFO << "proc " << m_procHandle << " patches target " << targetId << " to label " << std::string(labelName);
    return {};
}

tempo_utils::Status
lyric_assembler::ProcBuilder::touch(ObjectWriter &writer) const
{
    return m_rootFragment->touch(writer);
}

tempo_utils::Status
lyric_assembler::ProcBuilder::build(
    const ObjectWriter &writer,
    lyric_object::BytecodeBuilder &bytecodeBuilder) const
{
    absl::flat_hash_map<std::string,tu_uint16> labelOffsets;
    absl::flat_hash_map<tu_uint32,tu_uint16> patchOffsets;

    TU_RETURN_IF_NOT_OK (m_rootFragment->build(writer, bytecodeBuilder, labelOffsets, patchOffsets));

    TU_LOG_INFO << "label targets:";
    for (const auto &entry : m_labelTargets) {
        TU_LOG_INFO << "  label " << entry.first;
        for (const auto &target : entry.second.targets) {
            TU_LOG_INFO << "    target " << target;
        }
    }
    TU_LOG_INFO << "jump labels:";
    for (const auto &entry : m_jumpLabels) {
        TU_LOG_INFO << "  target " << entry.first << " -> " << entry.second.name;
    }

    for (const auto &entry : patchOffsets) {
        auto targetId = entry.first;
        auto patchOffset = entry.second;
        const auto &jumpLabel = m_jumpLabels.at(targetId);
        auto &labelName = jumpLabel.name;
        auto labelOffset = labelOffsets.find(labelName);
        if (labelOffset == labelOffsets.cend())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "missing label '{}' for patch target {}", labelName, targetId);
        TU_RETURN_IF_NOT_OK (bytecodeBuilder.patch(patchOffset, labelOffset->second));
    }

    return {};
}
