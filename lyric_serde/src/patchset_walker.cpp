
#include <lyric_serde/internal/patchset_reader.h>
#include <lyric_serde/patchset_walker.h>

lyric_serde::PatchsetWalker::PatchsetWalker()
{
}

lyric_serde::PatchsetWalker::PatchsetWalker(std::shared_ptr<const internal::PatchsetReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::PatchsetWalker::PatchsetWalker(const PatchsetWalker &other)
    : m_reader(other.m_reader)
{
}

bool
lyric_serde::PatchsetWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

lyric_serde::NamespaceWalker
lyric_serde::PatchsetWalker::getNamespace(tu_uint32 index) const
{
    if (!isValid())
        return {};
    if (m_reader->numNamespaces() <= index)
        return {};
    return NamespaceWalker(m_reader, index);
}

int
lyric_serde::PatchsetWalker::numNamespaces() const
{
    if (!isValid())
        return {};
    return m_reader->numNamespaces();
}

lyric_serde::ChangeWalker
lyric_serde::PatchsetWalker::getChange(tu_uint32 index) const
{
    if (!isValid())
        return {};
    if (m_reader->numChanges() <= index)
        return {};
    return ChangeWalker(m_reader, index);
}

int
lyric_serde::PatchsetWalker::numChanges() const
{
    if (!isValid())
        return {};
    return m_reader->numChanges();
}

lyric_serde::ValueWalker
lyric_serde::PatchsetWalker::getValue(tu_uint32 index) const
{
    if (!isValid())
        return {};
    if (m_reader->numValues() <= index)
        return {};
    return ValueWalker(m_reader, index);
}

int
lyric_serde::PatchsetWalker::numValues() const
{
    if (!isValid())
        return {};
    return m_reader->numValues();
}
