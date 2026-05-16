
#include <lyric_assembler/loop_handle.h>
#include <lyric_assembler/proc_handle.h>

lyric_assembler::LoopHandle::LoopHandle(
    const JumpLabel &topOfLoop,
    ProcHandle *procHandle,
    ObjectState *state)
    : m_topOfLoop(topOfLoop),
      m_procHandle(procHandle),
      m_state(state)
{
    TU_ASSERT (m_topOfLoop.isValid());
    TU_NOTNULL (m_procHandle);
    TU_NOTNULL (m_state);
}

lyric_assembler::JumpLabel
lyric_assembler::LoopHandle::getTopOfLoop() const
{
    return m_topOfLoop;
}

tempo_utils::Status
lyric_assembler::LoopHandle::continueLoop(const JumpTarget &continueTarget)
{
    if (m_loopExit.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "loop handle is already finalized");
    m_continueTargets.push_back(continueTarget);
    return {};
}

std::vector<lyric_assembler::JumpTarget>::const_iterator
lyric_assembler::LoopHandle::continueTargetsBegin() const
{
    return m_continueTargets.cbegin();
}

std::vector<lyric_assembler::JumpTarget>::const_iterator
lyric_assembler::LoopHandle::continueTargetsEnd() const
{
    return m_continueTargets.cend();
}

tempo_utils::Status
lyric_assembler::LoopHandle::breakLoop(const JumpTarget &breakTarget)
{
    if (m_loopExit.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "loop handle is already finalized");
    m_breakTargets.push_back(breakTarget);
    return {};
}

std::vector<lyric_assembler::JumpTarget>::const_iterator
lyric_assembler::LoopHandle::breakTargetsBegin() const
{
    return m_breakTargets.cbegin();
}

std::vector<lyric_assembler::JumpTarget>::const_iterator
lyric_assembler::LoopHandle::breakTargetsEnd() const
{
    return m_breakTargets.cend();
}

tempo_utils::Status
lyric_assembler::LoopHandle::finalizeLoop(const JumpLabel &loopExit)
{
    if (m_loopExit.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "loop handle is already finalized");
    m_loopExit = loopExit;
    TU_ASSERT (m_loopExit.isValid());

    auto *fragment = m_procHandle->procFragment();

    // patch jumps
    for (const auto &jumpTarget : m_continueTargets) {
        TU_RETURN_IF_NOT_OK (fragment->patchTarget(jumpTarget, m_topOfLoop));
    }
    for (const auto &jumpTarget : m_breakTargets) {
        TU_RETURN_IF_NOT_OK (fragment->patchTarget(jumpTarget, m_loopExit));
    }

    // pop the loop handle off the stack
    TU_RETURN_IF_NOT_OK (m_procHandle->popLoop());

    return {};
}
