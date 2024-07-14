
#include <lyric_parser/archetype_namespace.h>

lyric_parser::ArchetypeNamespace::ArchetypeNamespace(
    const tempo_utils::Url &nsUrl,
    ArchetypeId *archetypeId,
    ArchetypeState *state)
    : m_nsUrl(nsUrl),
      m_archetypeId(archetypeId),
      m_state(state)
{
    TU_ASSERT (m_nsUrl.isValid());
    TU_ASSERT (m_archetypeId != nullptr);
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Url
lyric_parser::ArchetypeNamespace::getNsUrl() const
{
    return m_nsUrl;
}

lyric_parser::ArchetypeId *
lyric_parser::ArchetypeNamespace::getArchetypeId() const
{
    return m_archetypeId;
}