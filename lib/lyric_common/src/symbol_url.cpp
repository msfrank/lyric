
#include <absl/strings/str_split.h>

#include <lyric_common/symbol_url.h>
#include <tempo_utils/log_stream.h>

lyric_common::SymbolUrl::SymbolUrl()
    : m_location(),
      m_path()
{
}

lyric_common::SymbolUrl::SymbolUrl(const lyric_common::SymbolPath &path)
    : m_location(),
      m_path(path)
{
}

lyric_common::SymbolUrl::SymbolUrl(const ModuleLocation &location, const lyric_common::SymbolPath &path)
    : m_location(location),
      m_path(path)
{
}

lyric_common::SymbolUrl::SymbolUrl(const lyric_common::SymbolUrl &other)
    : m_location(other.m_location),
      m_path(other.m_path)
{
}

lyric_common::SymbolUrl::SymbolUrl(SymbolUrl &&other) noexcept
{
    m_location = std::move(other.m_location);
    m_path = std::move(other.m_path);
}

lyric_common::SymbolUrl &
lyric_common::SymbolUrl::operator=(const SymbolUrl &other)
{
    if (this != &other) {
        m_location = other.m_location;
        m_path = other.m_path;
    }
    return *this;
}

lyric_common::SymbolUrl &
lyric_common::SymbolUrl::operator=(SymbolUrl &&other) noexcept
{
    if (this != &other) {
        m_location = std::move(other.m_location);
        m_path = std::move(other.m_path);
    }
    return *this;
}

bool
lyric_common::SymbolUrl::isValid() const
{
    return m_path.isValid();
}

bool
lyric_common::SymbolUrl::isAbsolute() const
{
    return m_location.isValid();
}

bool
lyric_common::SymbolUrl::isRelative() const
{
    return !m_location.isValid();
}

lyric_common::SymbolPath
lyric_common::SymbolUrl::getSymbolPath() const
{
    return m_path;
}

std::string
lyric_common::SymbolUrl::getSymbolName() const
{
    return m_path.getName();
}

lyric_common::ModuleLocation
lyric_common::SymbolUrl::getModuleLocation() const
{
    return m_location;
}

lyric_common::SymbolUrl
lyric_common::SymbolUrl::resolve(const ModuleLocation &base) const
{
    if (m_location.isAbsolute())
        return *this;
    auto resolved = base.resolve(m_location);
    return SymbolUrl(resolved, m_path);
}

std::string
lyric_common::SymbolUrl::toString() const
{
    if (!m_location.isValid())
        return absl::StrCat("#", m_path.toString());
    return absl::StrCat(m_location.toString(), "#", m_path.toString());
}

tempo_utils::Url
lyric_common::SymbolUrl::toUrl() const
{
    return tempo_utils::Url::fromString(toString());
}

bool
lyric_common::SymbolUrl::operator==(const lyric_common::SymbolUrl &other) const
{
    return m_location == other.m_location && m_path == other.m_path;
}

bool
lyric_common::SymbolUrl::operator!=(const lyric_common::SymbolUrl &other) const
{
    return !(*this == other);
}

lyric_common::SymbolUrl
lyric_common::SymbolUrl::fromString(std::string_view s)
{
    auto url = tempo_utils::Url::fromString(s);
    return fromUrl(url);
}

lyric_common::SymbolUrl
lyric_common::SymbolUrl::fromUrl(const tempo_utils::Url &url)
{
    if (!url.isValid())
        return {};
    auto fragment = url.fragmentView();
    if (fragment.empty())
        return {};

    auto urlWithoutFragment = url.withFragment("");
    ModuleLocation location = urlWithoutFragment.isValid() ?
        ModuleLocation::fromUrl(urlWithoutFragment) : ModuleLocation() ;

    auto path = lyric_common::SymbolPath::fromString(fragment);
    return lyric_common::SymbolUrl(location, path);
}

bool
lyric_common::operator<(const lyric_common::SymbolUrl &lhs, const lyric_common::SymbolUrl &rhs)
{
    return lhs.toString() < rhs.toString();
}

tempo_utils::LogMessage&&
lyric_common::operator<<(tempo_utils::LogMessage &&message, const lyric_common::SymbolUrl &symbolUrl)
{
    std::forward<tempo_utils::LogMessage>(message) << symbolUrl.toString();
    return std::move(message);
}