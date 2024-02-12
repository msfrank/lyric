
#include <lyric_packaging/entry_path.h>
#include <tempo_utils/log_stream.h>

lyric_packaging::EntryPath::EntryPath()
{
}

lyric_packaging::EntryPath::EntryPath(const tempo_utils::UrlPath &path)
    : m_path(path)
{
}

lyric_packaging::EntryPath::EntryPath(const lyric_packaging::EntryPath &other)
    : m_path(other.m_path)
{
}

bool
lyric_packaging::EntryPath::isValid() const
{
    return m_path.isValid();
}

bool
lyric_packaging::EntryPath::isEmpty() const
{
    return m_path.isEmpty();
}

int
lyric_packaging::EntryPath::numParts() const
{
    return m_path.numParts();
}

std::string
lyric_packaging::EntryPath::getPart(int index) const
{
    return m_path.getPart(index).getPart();
}

std::string_view
lyric_packaging::EntryPath::partView(int index) const
{
    return m_path.partView(index);
}

lyric_packaging::EntryPath
lyric_packaging::EntryPath::getInit() const
{
    return EntryPath(m_path.getInit());
}

lyric_packaging::EntryPath
lyric_packaging::EntryPath::getTail() const
{
    return EntryPath(m_path.getTail());
}

std::string
lyric_packaging::EntryPath::getFilename() const
{
    return m_path.getLast().getPart();
}

std::string_view
lyric_packaging::EntryPath::filenameView() const
{
    return m_path.lastView();
}

std::string_view
lyric_packaging::EntryPath::pathView() const
{
    return m_path.pathView();
}

lyric_packaging::EntryPath
lyric_packaging::EntryPath::traverse(std::string_view part)
{
    auto path = m_path.traverse(tempo_utils::UrlPathPart(part));
    return EntryPath(path);
}

std::string
lyric_packaging::EntryPath::toString() const
{
    return m_path.toString();
}

tempo_utils::Url
lyric_packaging::EntryPath::toUrl() const
{
    return tempo_utils::Url::fromRelative(m_path.pathView());
}

bool
lyric_packaging::EntryPath::operator==(const lyric_packaging::EntryPath &other) const
{
    return m_path == other.m_path;
}

bool
lyric_packaging::EntryPath::operator!=(const lyric_packaging::EntryPath &other) const
{
    return !(*this == other);
}

lyric_packaging::EntryPath
lyric_packaging::EntryPath::fromString(std::string_view s)
{
    return EntryPath(tempo_utils::UrlPath::fromString(s));
}

tempo_utils::LogMessage&&
lyric_packaging::operator<<(tempo_utils::LogMessage &&message, const lyric_packaging::EntryPath &entryPath)
{
    std::forward<tempo_utils::LogMessage>(message) << entryPath.toString();
    return std::move(message);
}