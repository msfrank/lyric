
#include <absl/strings/str_split.h>

#include <lyric_common/module_location.h>
#include <tempo_utils/log_stream.h>

lyric_common::ModuleLocation::ModuleLocation()
{
}

lyric_common::ModuleLocation::ModuleLocation(const tempo_utils::Url &location)
    : m_location(tempo_utils::make_prehashed<tempo_utils::Url>(location))
{
    TU_ASSERT (m_location->isValid());
}

lyric_common::ModuleLocation::ModuleLocation(std::string_view path)
    : ModuleLocation(tempo_utils::Url::fromRelative(path))
{
}

lyric_common::ModuleLocation::ModuleLocation(std::string_view origin, std::string_view path)
    : ModuleLocation(tempo_utils::Url::fromOrigin(origin, path))
{
}

lyric_common::ModuleLocation::ModuleLocation(const ModuleLocation &other)
    : m_location(other.m_location)
{
}

lyric_common::ModuleLocation::ModuleLocation(ModuleLocation &&other) noexcept
{
    m_location = std::move(other.m_location);
}

lyric_common::ModuleLocation&
lyric_common::ModuleLocation::operator=(const ModuleLocation &other)
{
    m_location = other.m_location;
    return *this;
}

lyric_common::ModuleLocation&
lyric_common::ModuleLocation::operator=(ModuleLocation &&other) noexcept
{
    if (this != &other) {
        m_location = std::move(other.m_location);
    }
    return *this;
}

bool
lyric_common::ModuleLocation::isValid() const
{
    return m_location->isValid();
}

bool
lyric_common::ModuleLocation::isAbsolute() const
{
    return m_location->isAbsolute();
}

bool
lyric_common::ModuleLocation::isRelative() const
{
    return m_location->isRelative();
}

bool
lyric_common::ModuleLocation::hasScheme() const
{
    return m_location->hasScheme();
}

bool
lyric_common::ModuleLocation::hasOrigin() const
{
    return m_location->toOrigin().isValid();
}

bool
lyric_common::ModuleLocation::hasAuthority() const
{
    return m_location->toAuthority().isValid();
}

bool
lyric_common::ModuleLocation::hasPathParts() const
{
    return !m_location->toPath().isEmpty();
}

std::string
lyric_common::ModuleLocation::getScheme() const
{
    return m_location->getScheme();
}

tempo_utils::UrlOrigin
lyric_common::ModuleLocation::getOrigin() const
{
    return m_location->toOrigin();
}

tempo_utils::UrlAuthority
lyric_common::ModuleLocation::getAuthority() const
{
    return m_location->toAuthority();
}

tempo_utils::UrlPath
lyric_common::ModuleLocation::getPath() const
{
    return m_location->toPath();
}

std::string
lyric_common::ModuleLocation::getModuleName() const
{
    return m_location->toPath().getLast().getPart();
}

lyric_common::ModuleLocation
lyric_common::ModuleLocation::resolve(const ModuleLocation &rel) const
{
    auto &base = m_location;
    auto &ref = rel.m_location;

    if (!base->isAbsolute())
        return {};
    if (!ref->isRelative())
        return {};

    auto resolved = base->resolve(*ref);
    if (!resolved.isValid())
        return {};
    return ModuleLocation(resolved);
}

std::string
lyric_common::ModuleLocation::toString() const
{
    if (!isValid())
        return {};
    return m_location->toString();
}

tempo_utils::Url
lyric_common::ModuleLocation::toUrl() const
{
    return *m_location;
}

bool
lyric_common::ModuleLocation::operator==(const ModuleLocation &other) const
{
    return m_location == other.m_location;
}

bool
lyric_common::ModuleLocation::operator!=(const ModuleLocation &other) const
{
    return !(*this == other);
}

lyric_common::ModuleLocation
lyric_common::ModuleLocation::fromString(std::string_view s)
{
    auto url = tempo_utils::Url::fromString(s);
    return fromUrl(url);
}

lyric_common::ModuleLocation
lyric_common::ModuleLocation::fromUrl(const tempo_utils::Url &url)
{
    if (url.isAbsolute() || url.isRelative()) {
        if (url.hasQuery() || url.hasFragment())
            return {};
        return ModuleLocation(url);
    }
    return {};
}

lyric_common::ModuleLocation
lyric_common::ModuleLocation::fromUrlPath(const tempo_utils::UrlPath &urlPath)
{
    return ModuleLocation(tempo_utils::Url::fromRelative(urlPath.toString()));
}

tempo_utils::LogMessage&&
lyric_common::operator<<(tempo_utils::LogMessage &&message, const lyric_common::ModuleLocation &location)
{
    std::forward<tempo_utils::LogMessage>(message)
        << "ModuleLocation("
        << location.toString()
        << ")";
    return std::move(message);
}