
#include <lyric_serde/patchset_namespace.h>

lyric_serde::PatchsetNamespace::PatchsetNamespace(
    const tempo_utils::Url &nsUrl,
    NamespaceAddress address,
    PatchsetState *state)
    : m_nsUrl(nsUrl),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_nsUrl.isValid());
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Url
lyric_serde::PatchsetNamespace::getNsUrl() const
{
    return m_nsUrl;
}

lyric_serde::NamespaceAddress
lyric_serde::PatchsetNamespace::getAddress() const
{
    return m_address;
}
