
#include <lyric_object/plugin_walker.h>

#include "lyric_object/internal/object_reader.h"

lyric_object::PluginWalker::PluginWalker()
{
}

lyric_object::PluginWalker::PluginWalker(std::shared_ptr<const internal::ObjectReader> reader, void *pluginDescriptor)
    : m_reader(reader),
      m_pluginDescriptor(pluginDescriptor)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_pluginDescriptor != nullptr);
}

lyric_object::PluginWalker::PluginWalker(const PluginWalker &other)
    : m_reader(other.m_reader)
{
}

bool
lyric_object::PluginWalker::isValid() const
{
    return m_reader != nullptr && m_reader->isValid() && m_pluginDescriptor != nullptr;
}

lyric_common::ModuleLocation
lyric_object::PluginWalker::getPluginLocation() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    auto *pluginLocation = pluginDescriptor->plugin_location();
    if (pluginLocation == nullptr)
        return {};
    return lyric_common::ModuleLocation::fromString(pluginLocation->string_view());
}