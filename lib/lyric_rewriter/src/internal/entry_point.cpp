
#include <lyric_rewriter/internal/entry_point.h>

lyric_rewriter::internal::EntryPoint::EntryPoint(lyric_parser::ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_rewriter::internal::EntryPoint::~EntryPoint()
{
}

lyric_parser::ArchetypeState *
lyric_rewriter::internal::EntryPoint::getState() const
{
    return m_state;
}