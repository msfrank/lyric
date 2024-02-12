
#include <lyric_packaging/manifest_namespace.h>

lyric_packaging::ManifestNamespace::ManifestNamespace(
    const tempo_utils::Url &nsUrl,
    NamespaceAddress address,
    ManifestState *state)
    : m_nsUrl(nsUrl),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_nsUrl.isValid());
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Url
lyric_packaging::ManifestNamespace::getNsUrl() const
{
    return m_nsUrl;
}

lyric_packaging::NamespaceAddress
lyric_packaging::ManifestNamespace::getAddress() const
{
    return m_address;
}
