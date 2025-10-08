
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

lyric_assembler::JumpLabel
lyric_assembler::CheckHandle::getStartInclusive() const
{
    return m_startInclusive;
}

lyric_assembler::JumpLabel
lyric_assembler::CheckHandle::getEndexclusive() const
{
    return m_endExclusive;
}

tempo_utils::Result<lyric_assembler::CatchHandle *>
lyric_assembler::CheckHandle::declareException(const lyric_common::TypeDef &exceptionType)
{
    if (m_exceptions.contains(exceptionType))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "exception handler for {} is already declared", exceptionType.toString());
    CatchHandle *catchHandle;
    TU_ASSIGN_OR_RETURN (catchHandle, m_procHandle->declareCatch(exceptionType));
    m_exceptions[exceptionType] = catchHandle;
    return catchHandle;
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::CatchHandle *>::const_iterator
lyric_assembler::CheckHandle::exceptionsBegin() const
{
    return m_exceptions.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::CatchHandle *>::const_iterator
lyric_assembler::CheckHandle::exceptionsEnd() const
{
    return m_exceptions.cend();
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
