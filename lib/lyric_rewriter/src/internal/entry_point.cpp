
#include <lyric_rewriter/internal/entry_point.h>

lyric_rewriter::internal::EntryPoint::EntryPoint(MacroRegistry *registry, lyric_parser::ArchetypeState *state)
    : m_registry(registry),
      m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_rewriter::internal::EntryPoint::~EntryPoint()
{
}

lyric_rewriter::MacroRegistry *
lyric_rewriter::internal::EntryPoint::getRegistry() const
{
    return m_registry;
}

lyric_parser::ArchetypeState *
lyric_rewriter::internal::EntryPoint::getState() const
{
    return m_state;
}