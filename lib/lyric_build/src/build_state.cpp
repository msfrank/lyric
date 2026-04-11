
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_registry.h>
#include <lyric_runtime/chain_loader.h>

lyric_build::BuildState::BuildState(std::unique_ptr<Priv> priv)
    : m_priv(std::move(priv))
{
    TU_ASSERT (m_priv->buildGen.isValid());
    TU_ASSERT (m_priv->artifactCache != nullptr);
    TU_ASSERT (m_priv->bootstrapLoader != nullptr);
    // fallbackLoader can be nullptr
    TU_ASSERT (m_priv->loaderChain != nullptr);
    TU_ASSERT (m_priv->sharedModuleCache != nullptr);
    TU_ASSERT (m_priv->shortcutResolver != nullptr);
    TU_ASSERT (m_priv->virtualFilesystem != nullptr);
    TU_ASSERT (!m_priv->tempRoot.empty());
}

lyric_build::BuildState::~BuildState()
{
    // clean up tasks
    for (const auto &entry : m_priv->tasks) {
        delete entry.second;
    }
}

std::shared_ptr<lyric_build::BuildState>
lyric_build::BuildState::create(
    const BuildGeneration &buildGen,
    std::shared_ptr<AbstractArtifactCache> artifactCache,
    std::shared_ptr<lyric_runtime::AbstractLoader> bootstrapLoader,
    std::shared_ptr<lyric_runtime::AbstractLoader> fallbackLoader,
    std::shared_ptr<lyric_importer::ModuleCache> sharedModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    std::shared_ptr<AbstractVirtualFilesystem> virtualFilesystem,
    const std::filesystem::path &tempRoot)
{
    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaders;
    loaders.push_back(bootstrapLoader);
    if (fallbackLoader != nullptr) {
        loaders.push_back(fallbackLoader);
    }
    auto loaderChain = std::make_shared<lyric_runtime::ChainLoader>(loaders);

    auto priv = std::make_unique<Priv>();
    priv->buildGen = buildGen;
    priv->artifactCache = std::move(artifactCache);
    priv->bootstrapLoader = std::move(bootstrapLoader);
    priv->fallbackLoader = std::move(fallbackLoader);
    priv->loaderChain = std::move(loaderChain);
    priv->sharedModuleCache = std::move(sharedModuleCache);
    priv->shortcutResolver = std::move(shortcutResolver);
    priv->virtualFilesystem = std::move(virtualFilesystem);
    priv->tempRoot = tempRoot;

    return std::make_shared<BuildState>(std::move(priv));
}

lyric_build::BuildGeneration
lyric_build::BuildState::getGeneration() const
{
    return m_priv->buildGen;
}

std::shared_ptr<lyric_build::AbstractArtifactCache>
lyric_build::BuildState::getArtifactCache() const
{
    return m_priv->artifactCache;
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::BuildState::getBootstrapLoader() const
{
    return m_priv->bootstrapLoader;
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::BuildState::getFallbackLoader() const
{
    return m_priv->fallbackLoader;
}

std::shared_ptr<lyric_runtime::AbstractLoader>
lyric_build::BuildState::getLoaderChain() const
{
    return m_priv->loaderChain;
}

std::shared_ptr<lyric_importer::ModuleCache>
lyric_build::BuildState::getSharedModuleCache() const
{
    return m_priv->sharedModuleCache;
}

std::shared_ptr<lyric_importer::ShortcutResolver>
lyric_build::BuildState::getShortcutResolver() const
{
    return m_priv->shortcutResolver;
}

std::shared_ptr<lyric_build::AbstractVirtualFilesystem>
lyric_build::BuildState::getVirtualFilesystem() const
{
    return m_priv->virtualFilesystem;
}

std::filesystem::path
lyric_build::BuildState::getTempRoot() const
{
    return m_priv->tempRoot;
}

lyric_build::TaskData
lyric_build::BuildState::loadState(const TaskKey &key)
{
    absl::ReaderMutexLock locker(&m_priv->lock);

    auto entry = m_priv->tasks.find(key);
    if (entry == m_priv->tasks.cend())
        return {};
    auto *task = entry->second;
    return task->getData();
}

absl::flat_hash_map<lyric_build::TaskKey, lyric_build::TaskData>
lyric_build::BuildState::loadStates(const absl::flat_hash_set<TaskKey> &keys)
{
    return loadStates(keys.cbegin(), keys.cend());
}

absl::flat_hash_map<lyric_build::TaskKey, lyric_build::TaskData>
lyric_build::BuildState::loadStates(
    absl::flat_hash_set<TaskKey>::const_iterator begin,
    absl::flat_hash_set<TaskKey>::const_iterator end)
{
    absl::ReaderMutexLock locker(&m_priv->lock);

    absl::flat_hash_map<TaskKey,TaskData> states;
    for (; begin != end; begin++) {
        const auto &key = *begin;
        auto entry = m_priv->tasks.find(key);
        if (entry != m_priv->tasks.cend()) {
            auto *task = entry->second;
            states[key] = task->getData();
        }
    }
    return states;
}

tempo_utils::Result<lyric_build::BaseTask *>
lyric_build::BuildState::getOrMakeTask(
    const TaskKey &key,
    TaskRegistry *registry,
    std::shared_ptr<tempo_tracing::TraceRecorder> &recorder)
{
    absl::WriterMutexLock locker(&m_priv->lock);

    auto entry = m_priv->tasks.find(key);
    if (entry != m_priv->tasks.cend())
        return entry->second;

    auto span = recorder->makeSpan();
    BaseTask *task;
    TU_ASSIGN_OR_RETURN (task, registry->makeTask(m_priv->buildGen, key, weak_from_this(), std::move(span)));
    m_priv->tasks[key] = task;

    return task;
}

lyric_build::TaskStatistics
lyric_build::BuildState::getTaskStatistics()
{
    absl::ReaderMutexLock locker(&m_priv->lock);

    TaskStatistics stats;
    for (const auto &entry : m_priv->tasks) {
        auto *task = entry.second;
        if (task->getGeneration() == m_priv->buildGen) {
            stats.totalTasksCreated++;
        } else {
            stats.totalTasksCached++;
        }
    }

    return stats;
}