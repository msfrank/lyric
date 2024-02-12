
#include <lyric_object/plugin_specifier.h>
#include <absl/strings/str_cat.h>

lyric_object::PluginSpecifier::PluginSpecifier()
{
}

lyric_object::PluginSpecifier::PluginSpecifier(
    std::string_view systemName,
    std::string_view architecture,
    std::string_view systemVersion,
    std::string_view compilerId)
    : m_systemName(systemName),
      m_architecture(architecture),
      m_systemVersion(systemVersion),
      m_compilerId(compilerId)
{
}

lyric_object::PluginSpecifier::PluginSpecifier(const PluginSpecifier &other)
    : m_systemName(other.m_systemName),
      m_architecture(other.m_architecture),
      m_systemVersion(other.m_systemVersion),
      m_compilerId(other.m_compilerId)
{
}

bool
lyric_object::PluginSpecifier::isValid() const
{
    return !m_systemName.empty() && !m_architecture.empty();
}

std::string
lyric_object::PluginSpecifier::getSystemName() const
{
    return m_systemName;
}

std::string
lyric_object::PluginSpecifier::getArchitecture() const
{
    return m_architecture;
}

std::string
lyric_object::PluginSpecifier::getSystemVersion() const
{
    return m_systemVersion;
}

std::string
lyric_object::PluginSpecifier::getCompilerId() const
{
    return m_compilerId;
}

std::string
lyric_object::PluginSpecifier::toString() const
{
    if (!isValid())
        return {};
    auto specifier = absl::StrCat(m_systemName, "-", m_architecture);
    return specifier;
}

lyric_object::PluginSpecifier
lyric_object::PluginSpecifier::systemDefault()
{
    return PluginSpecifier(OBJECT_PLUGIN_SYSTEM_NAME, OBJECT_PLUGIN_ARCHITECTURE);
}