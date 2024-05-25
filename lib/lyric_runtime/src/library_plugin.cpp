
#include <lyric_runtime/library_plugin.h>

lyric_runtime::LibraryPlugin::LibraryPlugin(
    std::shared_ptr<tempo_utils::LibraryLoader> loader,
    const NativeInterface *iface)
    : m_loader(loader),
      m_iface(iface)
{
    TU_ASSERT (m_loader != nullptr);
    TU_ASSERT (m_iface != nullptr);
}

bool
lyric_runtime::LibraryPlugin::load(BytecodeSegment *segment) const
{
    return m_iface->load(segment);
}

void
lyric_runtime::LibraryPlugin::unload() const
{
    m_iface->unload();
}

lyric_runtime::NativeFunc
lyric_runtime::LibraryPlugin::getTrap(tu_uint32 index) const
{
    return m_iface->getTrap(index);
}

tu_uint32
lyric_runtime::LibraryPlugin::numTraps() const
{
    return m_iface->numTraps();
}
