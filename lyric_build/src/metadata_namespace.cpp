
#include <lyric_build/metadata_namespace.h>

lyric_build::MetadataNamespace::MetadataNamespace(
    const tempo_utils::Url &nsUrl,
    NamespaceAddress address,
    MetadataState *state)
    : m_nsUrl(nsUrl),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_nsUrl.isValid());
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Url
lyric_build::MetadataNamespace::getNsUrl() const
{
    return m_nsUrl;
}

lyric_build::NamespaceAddress
lyric_build::MetadataNamespace::getAddress() const
{
    return m_address;
}
