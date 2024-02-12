
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

#include <lyric_common/common_types.h>
#include <lyric_packaging/package_specifier.h>
#include <lyric_packaging/package_types.h>

lyric_packaging::PackageSpecifier::PackageSpecifier()
    : m_majorVersion(0),
      m_minorVersion(0),
      m_patchVersion(0)
{
}

lyric_packaging::PackageSpecifier::PackageSpecifier(
    std::string_view packageName,
    std::string_view packageDomain,
    tu_uint32 majorVersion,
    tu_uint32 minorVersion,
    tu_uint32 patchVersion)
    : m_packageName(packageName),
      m_packageDomain(packageDomain),
      m_majorVersion(majorVersion),
      m_minorVersion(minorVersion),
      m_patchVersion(patchVersion)
{
    TU_ASSERT (!m_packageName.empty());
    TU_ASSERT (!m_packageDomain.empty());
}

lyric_packaging::PackageSpecifier::PackageSpecifier(const PackageSpecifier &other)
    : m_packageName(other.m_packageName),
      m_packageDomain(other.m_packageDomain),
      m_majorVersion(other.m_majorVersion),
      m_minorVersion(other.m_minorVersion),
      m_patchVersion(other.m_patchVersion)
{
}

bool
lyric_packaging::PackageSpecifier::isValid() const
{
    return !m_packageName.empty() && !m_packageDomain.empty();
}

std::string
lyric_packaging::PackageSpecifier::getPackageName() const
{
    return m_packageName;
}

std::string
lyric_packaging::PackageSpecifier::getPackageDomain() const
{
    return m_packageDomain;
}

tu_uint32
lyric_packaging::PackageSpecifier::getMajorVersion() const
{
    return m_majorVersion;
}

tu_uint32
lyric_packaging::PackageSpecifier::getMinorVersion() const
{
    return m_minorVersion;
}

tu_uint32
lyric_packaging::PackageSpecifier::getPatchVersion() const
{
    return m_patchVersion;
}

std::string
lyric_packaging::PackageSpecifier::toString() const
{
    std::vector<std::string> hostParts = absl::StrSplit(m_packageDomain, ".");
    std::reverse(hostParts.begin(), hostParts.end());
    return absl::StrCat(
        absl::StrJoin(hostParts, "."),
        "_",
        m_packageName,
        "-",
        m_majorVersion,
        ".",
        m_minorVersion,
        ".",
        m_patchVersion);
}

std::filesystem::path
lyric_packaging::PackageSpecifier::toFilesystemPath(const std::filesystem::path &base) const
{
    auto path = base / toString();
    path += lyric_common::kPackageFileDotSuffix;
    return path;
}

tempo_utils::Url
lyric_packaging::PackageSpecifier::toUrl() const
{
    auto username = absl::StrCat(
        m_packageName,
        "-",
        m_majorVersion,
        ".",
        m_minorVersion,
        ".",
        m_patchVersion);
    return tempo_utils::Url::fromOrigin(
        absl::StrCat("dev.zuri.pkg://", username, "@", m_packageDomain));
}

inline bool
parse_version_digit(std::string_view s, tu_uint32 &digit)
{
    if (s.empty())
        return false;
    auto first = s.front();
    if (s.size() == 1) {
        if (!std::isdigit(first))
            return false;
    } else {
        if (!std::isdigit(first) || first == '0')
            return false;
    }
    return absl::SimpleAtoi(s, &digit);
}

lyric_packaging::PackageSpecifier
lyric_packaging::PackageSpecifier::fromAuthority(const tempo_utils::UrlAuthority &authority)
{
    if (!authority.isValid())
        return {};
    if (!authority.hasHost())
        return {};
    if (!authority.hasUsername())
        return {};

    auto packageDomain = authority.getHost();

    // parse the name and version from userinfo
    auto nameAndVersion = authority.getUsername();
    auto lastDash = nameAndVersion.rfind('-');
    if (lastDash == std::string::npos)
        return {};
    auto packageName = nameAndVersion.substr(0, lastDash);
    auto versionString = nameAndVersion.substr(lastDash + 1);

    // parse the version
    std::vector<std::string> versionParts = absl::StrSplit(versionString, '.');
    if (versionParts.size() != 3)
        return {};

    tu_uint32 majorVersion;
    tu_uint32 minorVersion;
    tu_uint32 patchVersion;

    if (!parse_version_digit(versionParts.at(0), majorVersion))
        return {};
    if (!parse_version_digit(versionParts.at(1), minorVersion))
        return {};
    if (!parse_version_digit(versionParts.at(2), patchVersion))
        return {};

    return PackageSpecifier(packageName, packageDomain, majorVersion, minorVersion, patchVersion);
}

lyric_packaging::PackageSpecifier
lyric_packaging::PackageSpecifier::fromUrl(const tempo_utils::Url &uri)
{
    if (!uri.isValid())
        return {};
    if (uri.schemeView() != "dev.zuri.pkg")
        return {};
    return fromAuthority(uri.toAuthority());
}
