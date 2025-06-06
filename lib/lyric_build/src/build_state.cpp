
#include <lyric_build/build_state.h>
#include <lyric_build/rocksdb_cache.h>
#include <lyric_build/build_types.h>
#include <lyric_runtime/chain_loader.h>
#include <tempo_utils/log_stream.h>

lyric_build::BuildState::BuildState(
    const BuildGeneration &buildGen,
    std::shared_ptr<AbstractCache> cache,
    std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
    std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
    std::shared_ptr<lyric_importer::ModuleCache> sharedModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    std::shared_ptr<AbstractFilesystem> virtualFilesystem,
    const std::filesystem::path &tempRoot)
    : m_buildGen(buildGen),
      m_cache(std::move(cache)),
      m_bootstrapLoader(std::move(bootstrapLoader)),
      m_fallbackLoader(std::move(fallbackLoader)),
      m_sharedModuleCache(std::move(sharedModuleCache)),
      m_shortcutResolver(std::move(shortcutResolver)),
      m_virtualFilesystem(std::move(virtualFilesystem)),
      m_tempRoot(tempRoot)
{
    TU_ASSERT (m_buildGen.isValid());
    TU_ASSERT (m_cache != nullptr);
    TU_ASSERT (m_bootstrapLoader != nullptr);
    TU_ASSERT (m_sharedModuleCache != nullptr);
    TU_ASSERT (m_shortcutResolver != nullptr);
    TU_ASSERT (m_virtualFilesystem != nullptr);
    TU_ASSERT (!m_tempRoot.empty());

    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaders;
    loaders.push_back(m_bootstrapLoader);
    if (m_fallbackLoader != nullptr) {
        loaders.push_back(m_fallbackLoader);
    }
    m_loaderChain = std::make_shared<lyric_runtime::ChainLoader>(loaders);
}

lyric_build::BuildGeneration
lyric_build::BuildState::getGeneration() const
{
    return m_buildGen;
}

std::shared_ptr<lyric_build::AbstractCache>
lyric_build::BuildState::getCache() const
{
    return m_cache;
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::BuildState::getBootstrapLoader() const
{
    return m_bootstrapLoader;
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::BuildState::getFallbackLoader() const
{
    return m_fallbackLoader;
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::BuildState::getLoaderChain() const
{
    return m_loaderChain;
}

std::shared_ptr<lyric_importer::ModuleCache>
lyric_build::BuildState::getSharedModuleCache() const
{
    return m_sharedModuleCache;
}

std::shared_ptr<lyric_importer::ShortcutResolver>
lyric_build::BuildState::getShortcutResolver() const
{
    return m_shortcutResolver;
}

std::shared_ptr<lyric_build::AbstractFilesystem>
lyric_build::BuildState::getVirtualFilesystem() const
{
    return m_virtualFilesystem;
}

std::filesystem::path
lyric_build::BuildState::getTempRoot() const
{
    return m_tempRoot;
}

lyric_build::TaskState
lyric_build::BuildState::loadState(const TaskKey &key)
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    if (m_states.contains(key))
        return m_states[key];
    return {};
}

absl::flat_hash_map<lyric_build::TaskKey, lyric_build::TaskState>
lyric_build::BuildState::loadStates(const absl::flat_hash_set<TaskKey> &keys)
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    absl::flat_hash_map<TaskKey,TaskState> states;
    for (const auto &key : keys) {
        if (m_states.contains(key))
            states[key] = m_states[key];
    }
    return states;
}

void
lyric_build::BuildState::storeState(const TaskKey &key, const TaskState &state)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_states[key] = state;
}