
#include <absl/strings/str_split.h>

#include <lyric_common/assembly_location.h>
#include <tempo_utils/log_stream.h>

lyric_common::AssemblyLocation::AssemblyLocation()
{
}

lyric_common::AssemblyLocation::AssemblyLocation(const tempo_utils::Url &location)
    : m_location(tempo_utils::make_prehashed<tempo_utils::Url>(location))
{
    TU_ASSERT (m_location->isValid());
}

lyric_common::AssemblyLocation::AssemblyLocation(const std::string &path)
    : AssemblyLocation(tempo_utils::Url::fromRelative(path))
{
}

lyric_common::AssemblyLocation::AssemblyLocation(
    const std::string &origin,
    const std::string &path)
    : AssemblyLocation(tempo_utils::Url::fromOrigin(origin, path))
{
}

lyric_common::AssemblyLocation::AssemblyLocation(const AssemblyLocation &other)
    : m_location(other.m_location)
{
}

lyric_common::AssemblyLocation::AssemblyLocation(AssemblyLocation &&other) noexcept
{
    m_location = std::move(other.m_location);
}

lyric_common::AssemblyLocation&
lyric_common::AssemblyLocation::operator=(const AssemblyLocation &other)
{
    m_location = other.m_location;
    return *this;
}

lyric_common::AssemblyLocation&
lyric_common::AssemblyLocation::operator=(AssemblyLocation &&other) noexcept
{
    if (this != &other) {
        m_location = std::move(other.m_location);
    }
    return *this;
}

bool
lyric_common::AssemblyLocation::isValid() const
{
    return m_location->isValid();
}

bool
lyric_common::AssemblyLocation::hasScheme() const
{
    return m_location->hasScheme();
}

bool
lyric_common::AssemblyLocation::hasOrigin() const
{
    return m_location->toOrigin().isValid();
}

bool
lyric_common::AssemblyLocation::hasAuthority() const
{
    return m_location->toAuthority().isValid();
}

bool
lyric_common::AssemblyLocation::hasPathParts() const
{
    return !m_location->toPath().isEmpty();
}

std::string
lyric_common::AssemblyLocation::getScheme() const
{
    return m_location->getScheme();
}

tempo_utils::UrlOrigin
lyric_common::AssemblyLocation::getOrigin() const
{
    return m_location->toOrigin();
}

tempo_utils::UrlAuthority
lyric_common::AssemblyLocation::getAuthority() const
{
    return m_location->toAuthority();
}

tempo_utils::UrlPath
lyric_common::AssemblyLocation::getPath() const
{
    return m_location->toPath();
}

std::string
lyric_common::AssemblyLocation::getAssemblyName() const
{
    return m_location->toPath().getLast().getPart();
}

std::string
lyric_common::AssemblyLocation::toString() const
{
    if (!isValid())
        return std::string();
    return m_location->toString();
}

tempo_utils::Url
lyric_common::AssemblyLocation::toUrl() const
{
    return *m_location;
}

bool
lyric_common::AssemblyLocation::operator==(const AssemblyLocation &other) const
{
    return m_location == other.m_location;
}

bool
lyric_common::AssemblyLocation::operator!=(const AssemblyLocation &other) const
{
    return !(*this == other);
}

lyric_common::AssemblyLocation
lyric_common::AssemblyLocation::fromString(const std::string &string)
{
    auto url = tempo_utils::Url::fromString(string);
    return fromUrl(url);
}

lyric_common::AssemblyLocation
lyric_common::AssemblyLocation::fromUrl(const tempo_utils::Url &url)
{
    return AssemblyLocation(url);
}

tempo_utils::LogMessage&&
lyric_common::operator<<(tempo_utils::LogMessage &&message, const lyric_common::AssemblyLocation &location)
{
    std::forward<tempo_utils::LogMessage>(message)
        << "AssemblyLocation("
        << location.toString()
        << ")";
    return std::move(message);
}