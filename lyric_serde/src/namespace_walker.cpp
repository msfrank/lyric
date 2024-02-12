
#include <lyric_serde/internal/patchset_reader.h>
#include <lyric_serde/namespace_walker.h>

lyric_serde::NamespaceWalker::NamespaceWalker()
    : m_reader(),
      m_index(0)
{
}

lyric_serde::NamespaceWalker::NamespaceWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::NamespaceWalker::NamespaceWalker(const NamespaceWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::NamespaceWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numNamespaces();
}

tu_uint32
lyric_serde::NamespaceWalker::getIndex() const
{
    return m_index;
}

std::string_view
lyric_serde::NamespaceWalker::urlView() const
{
    if (!isValid())
        return {};
    auto *ns = m_reader->getNamespace(m_index);
    if (ns == nullptr || ns->ns_url() == nullptr)
        return {};
    return ns->ns_url()->string_view();
}

tempo_utils::Url
lyric_serde::NamespaceWalker::getUrl() const
{
    return tempo_utils::Url::fromString(urlView());
}
