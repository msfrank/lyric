
#include <lyric_assembler/check_handle.h>

#include "lyric_assembler/proc_handle.h"

lyric_assembler::CheckHandle::CheckHandle(
    const JumpLabel &startInclusive,
    const DataReference &caughtRef,
    ProcHandle *procHandle,
    ObjectState *state)
    : m_startInclusive(startInclusive),
      m_caughtRef(caughtRef),
      m_procHandle(procHandle),
      m_state(state)
{
    TU_ASSERT (m_startInclusive.isValid());
    TU_ASSERT (m_caughtRef.referenceType == ReferenceType::Value);
    TU_ASSERT (m_procHandle != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::JumpLabel
lyric_assembler::CheckHandle::getStartInclusive() const
{
    return m_startInclusive;
}

lyric_assembler::JumpLabel
lyric_assembler::CheckHandle::getEndExclusive() const
{
    return m_endExclusive;
}

lyric_assembler::DataReference
lyric_assembler::CheckHandle::getCaughtReference() const
{
    return m_caughtRef;
}

tempo_utils::Result<lyric_assembler::CatchHandle *>
lyric_assembler::CheckHandle::declareCatch(const JumpLabel &startInclusive)
{
    CatchHandle *catchPtr;
    TU_ASSIGN_OR_RETURN (catchPtr, m_procHandle->declareCatch(startInclusive));
    m_catches.push_back(catchPtr);
    return catchPtr;
}

std::vector<lyric_assembler::CatchHandle *>::const_iterator
lyric_assembler::CheckHandle::catchesBegin() const
{
    return m_catches.cbegin();
}

std::vector<lyric_assembler::CatchHandle *>::const_iterator
lyric_assembler::CheckHandle::catchesEnd() const
{
    return m_catches.cend();
}

int
lyric_assembler::CheckHandle::numCatches() const
{
    return m_catches.size();
}

void
lyric_assembler::CheckHandle::appendChild(CheckHandle *child)
{
    m_children.push_back(child);
}

std::vector<lyric_assembler::CheckHandle *>::const_iterator
lyric_assembler::CheckHandle::childrenBegin() const
{
    return m_children.cbegin();
}

std::vector<lyric_assembler::CheckHandle *>::const_iterator
lyric_assembler::CheckHandle::childrenEnd() const
{
    return m_children.cend();
}

int
lyric_assembler::CheckHandle::numChildren() const
{
    return m_children.size();
}

tempo_utils::Status
lyric_assembler::CheckHandle::finalizeCheck(const JumpLabel &endExclusive)
{
    if (m_endExclusive.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "check handle is already finalized");
    m_endExclusive = endExclusive;
    TU_ASSERT (m_endExclusive.isValid());

    TU_RETURN_IF_NOT_OK (m_procHandle->popCheck());

    return {};
}