
#include <lyric_assembler/check_handle.h>

#include "lyric_assembler/proc_handle.h"

lyric_assembler::CheckHandle::CheckHandle(
    const JumpLabel &startInclusive,
    ProcHandle *procHandle,
    ObjectState *state)
    : m_startInclusive(startInclusive),
      m_procHandle(procHandle),
      m_state(state)
{
    TU_ASSERT (m_startInclusive.isValid());
    TU_ASSERT (m_procHandle != nullptr);
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Result<lyric_assembler::CodeFragment *>
lyric_assembler::CheckHandle::declareException(const lyric_common::TypeDef &exceptionType)
{
    if (m_exceptions.contains(exceptionType))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "exception handler for {} is already declared", exceptionType.toString());
    auto *procFragment = m_procHandle->procFragment();
    auto fragment = procFragment->makeFragment();
    auto *fragmentPtr = fragment.get();
    m_exceptions[exceptionType] = std::move(fragment);
    return fragmentPtr;
}

int
lyric_assembler::CheckHandle::numExceptions() const
{
    return m_exceptions.size();
}

tempo_utils::Status
lyric_assembler::CheckHandle::finalizeCheck(const JumpLabel &endExclusive)
{
    if (m_endExclusive.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "check handle is already finalized");
    m_endExclusive = endExclusive;
    return {};
}
