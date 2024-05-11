
#include <lyric_packaging/internal/manifest_reader.h>
#include <lyric_packaging/manifest_walker.h>

lyric_packaging::ManifestWalker::ManifestWalker()
{
}

lyric_packaging::ManifestWalker::ManifestWalker(std::shared_ptr<const internal::ManifestReader> reader)
    : m_reader(reader)
{
}

lyric_packaging::ManifestWalker::ManifestWalker(const ManifestWalker &other)
    : m_reader(other.m_reader)
{
}

bool
lyric_packaging::ManifestWalker::isValid() const
{
    return m_reader && m_reader->isValid();
}

lyric_packaging::EntryWalker
lyric_packaging::ManifestWalker::getRoot() const
{
    return getEntry(EntryPath::fromString("/"));
}

bool
lyric_packaging::ManifestWalker::hasEntry(const EntryPath &entryPath) const
{
    auto *pathDescriptor = m_reader->findPath(entryPath.pathView());
    if (pathDescriptor == nullptr)
        return false;
    auto *entryDescriptor = m_reader->getEntry(pathDescriptor->entry());
    return entryDescriptor != nullptr;
}

lyric_packaging::EntryWalker
lyric_packaging::ManifestWalker::getEntry(tu_uint32 offset) const
{
    return EntryWalker(m_reader, offset);
}

lyric_packaging::EntryWalker
lyric_packaging::ManifestWalker::getEntry(const EntryPath &entryPath) const
{
    auto *pathDescriptor = m_reader->findPath(entryPath.pathView());
    if (pathDescriptor == nullptr)
        return {};
    return EntryWalker(m_reader, pathDescriptor->entry());
}

tu_uint32
lyric_packaging::ManifestWalker::numEntries() const
{
    return m_reader->numEntries();
}
