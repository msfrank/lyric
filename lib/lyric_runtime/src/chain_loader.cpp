
#include <lyric_runtime/chain_loader.h>

lyric_runtime::ChainLoader::ChainLoader()
{
}

lyric_runtime::ChainLoader::ChainLoader(const std::vector<std::shared_ptr<AbstractLoader>> &loaderChain)
    : m_chain(loaderChain)
{
}

lyric_runtime::ChainLoader::ChainLoader(const ChainLoader &other)
    : m_chain(other.m_chain)
{
}

tempo_utils::Result<bool>
lyric_runtime::ChainLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    for (const auto &loader : m_chain) {
        bool exists;
        TU_ASSIGN_OR_RETURN (exists, loader->hasModule(location));
        if (exists)
            return exists;
    }
    return false;
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_runtime::ChainLoader::loadModule(const lyric_common::ModuleLocation &location)
{
    for (auto &loader : m_chain) {
        auto loadModuleResult = loader->loadModule(location);
        if (loadModuleResult.isStatus())
            return loadModuleResult;
        auto objectOption = loadModuleResult.getResult();
        if (!objectOption.isEmpty())
            return loadModuleResult;
    }
    return Option<lyric_object::LyricObject>();
}

tempo_utils::Result<bool>
lyric_runtime::ChainLoader::hasPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier) const
{
    for (auto &loader : m_chain) {
        bool exists;
        TU_ASSIGN_OR_RETURN (exists, loader->hasPlugin(location, specifier));
        if (exists)
            return exists;
    }
    return false;
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_runtime::ChainLoader::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    for (auto &loader : m_chain) {
        auto loadPluginResult = loader->loadPlugin(location, specifier);
        if (loadPluginResult.isStatus())
            return loadPluginResult;
        auto pluginOption = loadPluginResult.getResult();
        if (!pluginOption.isEmpty())
            return loadPluginResult;
    }
    return Option<std::shared_ptr<const AbstractPlugin>>();
}

tempo_utils::Result<bool>
lyric_runtime::ChainLoader::hasResource(const lyric_common::ModuleLocation &location) const
{
    return false;
}

tempo_utils::Result<Option<std::shared_ptr<const tempo_utils::ImmutableBytes>>>
lyric_runtime::ChainLoader::loadResource(const lyric_common::ModuleLocation &location)
{
    return Option<std::shared_ptr<const tempo_utils::ImmutableBytes>>();
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_runtime::ChainLoader::getLoader(int index) const
{
    if (0 <= index && index < m_chain.size())
        return m_chain.at(index);
    return {};
}

int
lyric_runtime::ChainLoader::numLoaders() const
{
    return m_chain.size();
}