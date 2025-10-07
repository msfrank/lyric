
#include <lyric_assembler/cleanup_handle.h>

lyric_assembler::CleanupHandle::CleanupHandle(const JumpLabel &startInclusive, ObjectState *state)
    : m_startInclusive(startInclusive),
      m_state(state)
{
    TU_ASSERT (m_startInclusive.isValid());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_assembler::CleanupHandle::finalizeCleanup(const JumpLabel &endExclusive)
{
    if (m_endExclusive.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "cleanup handle is already finalized");
    m_endExclusive = endExclusive;
    return {};
}
