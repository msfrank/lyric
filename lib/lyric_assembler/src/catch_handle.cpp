
#include <lyric_assembler/catch_handle.h>

lyric_assembler::CatchHandle::CatchHandle(const JumpLabel &startInclusive, ObjectState *state)
    : m_startInclusive(startInclusive),
      m_state(state)
{
    TU_ASSERT (m_startInclusive.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::JumpLabel
lyric_assembler::CatchHandle::getStartInclusive() const
{
    return m_startInclusive;
}

lyric_assembler::JumpLabel
lyric_assembler::CatchHandle::getEndExclusive() const
{
    return m_endExclusive;
}

lyric_common::TypeDef
lyric_assembler::CatchHandle::getExceptionType() const
{
    return m_exceptionType;
}

tempo_utils::Status
lyric_assembler::CatchHandle::setExceptionType(const lyric_common::TypeDef &exceptionType)
{
    if (!exceptionType.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid catch exception type");
    m_exceptionType = exceptionType;
    return {};
}

lyric_assembler::JumpTarget
lyric_assembler::CatchHandle::getResumeTarget() const
{
    return m_resumeTarget;
}

tempo_utils::Status
lyric_assembler::CatchHandle::setResumeTarget(const JumpTarget &resumeTarget)
{
    if (!resumeTarget.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid catch resume target");
    m_resumeTarget = resumeTarget;
    return {};
}

lyric_assembler::ObjectState *
lyric_assembler::CatchHandle::objectState() const
{
    return m_state;
}

tempo_utils::Status
lyric_assembler::CatchHandle::finalizeCatch(const JumpLabel &endExclusive)
{
    if (m_endExclusive.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "check handle is already finalized");
    m_endExclusive = endExclusive;
    TU_ASSERT (m_endExclusive.isValid());
    return {};

}