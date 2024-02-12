
#include <lyric_parser/archetype_namespace.h>

lyric_parser::ArchetypeNamespace::ArchetypeNamespace(
    const tempo_utils::Url &nsUrl,
    NamespaceAddress address,
    ArchetypeState *state)
    : m_nsUrl(nsUrl),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_nsUrl.isValid());
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Url
lyric_parser::ArchetypeNamespace::getNsUrl() const
{
    return m_nsUrl;
}

lyric_parser::NamespaceAddress
lyric_parser::ArchetypeNamespace::getAddress() const
{
    return m_address;
}