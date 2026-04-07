
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

bool
lyric_object::PluginWalker::isExactLinkage() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    return bool(pluginDescriptor->flags() & lyo1::PluginFlags::ExactLinkage);
}

lyric_object::HashType
lyric_object::PluginWalker::getHashType() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    switch (pluginDescriptor->hash_type()) {
        case lyo1::HashType::None:
            return HashType::None;
        case lyo1::HashType::Sha256:
            return HashType::Sha256;
        default:
            return HashType::Invalid;
    }
}

bool
lyric_object::PluginWalker::hasHash() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    auto *pluginhash = pluginDescriptor->plugin_hash();
    return pluginhash != nullptr && !pluginhash->empty();
}

std::span<const tu_uint8>
lyric_object::PluginWalker::hashValue() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    auto *pluginhash = pluginDescriptor->plugin_hash();
    if (pluginhash == nullptr || pluginhash->empty())
        return {};
    return std::span(pluginhash->data(), pluginhash->size());
}

tu_uint32
lyric_object::PluginWalker::numTraps() const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    if (pluginDescriptor->traps() == nullptr)
        return 0;
    return pluginDescriptor->traps()->size();
}

std::string
lyric_object::PluginWalker::getTrap(tu_uint32 index)
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    auto *traps = pluginDescriptor->traps();
    if (traps == nullptr)
        return {};
    if (traps->size() <= index)
        return {};
    auto *trapName = traps->GetAsString(index);
    if (trapName == nullptr)
        return {};
    return trapName->str();
}

std::string_view
lyric_object::PluginWalker::trapValue(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *pluginDescriptor = (const lyo1::PluginDescriptor *) m_pluginDescriptor;
    auto *traps = pluginDescriptor->traps();
    if (traps == nullptr)
        return {};
    if (traps->size() <= index)
        return {};
    auto *trapName = traps->GetAsString(index);
    if (trapName == nullptr)
        return {};
    return trapName->string_view();
}