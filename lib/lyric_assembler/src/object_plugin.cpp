
#include <lyric_assembler/object_plugin.h>

lyric_assembler::ObjectPlugin::ObjectPlugin(
    const lyric_common::ModuleLocation &pluginLocation,
    std::unique_ptr<lyric_runtime::TrapIndex> trapIndex)
    : m_pluginLocation(pluginLocation),
      m_trapIndex(std::move(trapIndex))
{
    TU_ASSERT (m_pluginLocation.isValid());
    TU_ASSERT (m_trapIndex != nullptr);
}

lyric_common::ModuleLocation
lyric_assembler::ObjectPlugin::getLocation() const
{
    return m_pluginLocation;
}

tu_uint32
lyric_assembler::ObjectPlugin::lookupTrap(std::string_view name) const
{
    return m_trapIndex->lookupTrap(name);
}