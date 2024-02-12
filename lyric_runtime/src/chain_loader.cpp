
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
lyric_runtime::ChainLoader::hasAssembly(const lyric_common::AssemblyLocation &location) const
{
    for (const auto &loader : m_chain) {
        auto hasAssemblyResult = loader->hasAssembly(location);
        if (hasAssemblyResult.isStatus())
            return hasAssemblyResult;
        auto canLoad = hasAssemblyResult.getResult();
        if (canLoad)
            return hasAssemblyResult;
    }
    return false;
}

tempo_utils::Result<Option<lyric_common::AssemblyLocation>>
lyric_runtime::ChainLoader::resolveAssembly(const lyric_common::AssemblyLocation &location) const
{
    for (const auto &loader : m_chain) {
        auto resolveAssemblyResult = loader->resolveAssembly(location);
        if (resolveAssemblyResult.isStatus())
            return resolveAssemblyResult;
        auto resolvedLocationOption = resolveAssemblyResult.getResult();
        if (!resolvedLocationOption.isEmpty())
            return resolveAssemblyResult;
    }
    return Option<lyric_common::AssemblyLocation>();
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_runtime::ChainLoader::loadAssembly(const lyric_common::AssemblyLocation &location)
{
    for (auto &loader : m_chain) {
        auto loadAssemblyResult = loader->loadAssembly(location);
        if (loadAssemblyResult.isStatus())
            return loadAssemblyResult;
        auto assemblyOption = loadAssemblyResult.getResult();
        if (!assemblyOption.isEmpty())
            return loadAssemblyResult;
    }
    return Option<lyric_object::LyricObject>();
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_runtime::ChainLoader::loadPlugin(
    const lyric_common::AssemblyLocation &location,
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