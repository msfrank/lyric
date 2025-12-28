
#include <lyric_bootstrap/bootstrap_plugin.h>

lyric_bootstrap::BootstrapPlugin::BootstrapPlugin(const lyric_runtime::NativeInterface *iface)
    : m_iface(iface)
{
    TU_ASSERT (m_iface != nullptr);
}

bool
lyric_bootstrap::BootstrapPlugin::load(lyric_runtime::BytecodeSegment *segment) const
{
    return m_iface->load(segment);
}

void
lyric_bootstrap::BootstrapPlugin::unload(lyric_runtime::BytecodeSegment *segment) const
{
    m_iface->unload(segment);
}

const lyric_runtime::NativeTrap *
lyric_bootstrap::BootstrapPlugin::getTrap(tu_uint32 index) const
{
    return m_iface->getTrap(index);
}

tu_uint32
lyric_bootstrap::BootstrapPlugin::numTraps() const
{
    return m_iface->numTraps();
}
