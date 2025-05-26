
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_build/build_types.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_common/common_types.h>
#include <lyric_common/plugin.h>
#include <tempo_utils/file_utilities.h>

/**
 * Private constructor.
 *
 * @param objects A map containing objects loaded from the cache.
 */
lyric_build::DependencyLoader::DependencyLoader(
    TempDirectory *tempDirectory,
    const absl::flat_hash_map<lyric_common::ModuleLocation, lyric_object::LyricObject> &objects,
    const absl::flat_hash_map<
        lyric_common::ModuleLocation,
        std::shared_ptr<const tempo_utils::ImmutableBytes>
    > &plugins)
    : m_tempDirectory(tempDirectory),
      m_objects(objects),
      m_plugins(plugins)
{
    TU_ASSERT (m_tempDirectory != nullptr);
}

tempo_utils::Result<bool>
lyric_build::DependencyLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    return m_objects.contains(location);
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_build::DependencyLoader::loadModule(const lyric_common::ModuleLocation &location)
{
    if (m_objects.contains(location))
        return Option(m_objects.at(location));

    return Option<lyric_object::LyricObject>();
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_build::DependencyLoader::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    auto libraryEntry = m_libraries.find(location);

    // if library is loaded already then return it
    if (libraryEntry != m_libraries.cend())
        return Option(std::static_pointer_cast<const lyric_runtime::AbstractPlugin>(libraryEntry->second));

    // if there is no matching plugin then indicate plugin not found
    auto pluginEntry = m_plugins.find(location);
    if (pluginEntry == m_plugins.cend())
        return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();

    // write the plugin to a file in the temp directory
    auto content = pluginEntry->second;
    auto moduleName = location.getModuleName();
    auto fileName = tempo_utils::generate_name(absl::StrCat(moduleName, ".XXXXXXXX"));
    auto pluginFilename = lyric_common::pluginFilename(fileName);
    auto pluginPath = tempo_utils::UrlPath::fromString(pluginFilename).toAbsolute();

    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, m_tempDirectory->putContent(pluginPath, content));

    // attempt to load the plugin
    auto loader = std::make_shared<tempo_utils::LibraryLoader>(absolutePath, "native_init");
    TU_RETURN_IF_NOT_OK (loader->getStatus());

    // cast raw pointer to native_init function pointer
    auto native_init = (lyric_runtime::NativeInitFunc) loader->symbolPointer();
    if (native_init == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to retrieve native_init symbol from plugin {}", absolutePath.string());

    // retrieve the plugin interface
    auto *iface = native_init();
    if (iface == nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "failed to retrieve interface for plugin {}", absolutePath.string());

    TU_LOG_INFO << "loaded plugin " << absolutePath;

    auto plugin = std::make_shared<const lyric_runtime::LibraryPlugin>(loader, iface);
    m_libraries[location] = plugin;

    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>(plugin);
}

/**
 * Find all assembly artifacts from the tasks specified by depStates and load them from the cache.
 *
 * @param depStates The map containing task states for all dependent tasks.
 * @param cache The cache containing task artifacts.
 * @return Result containing shared pointer to a new DependencyLoader, or status if creation failed.
 */
tempo_utils::Result<std::shared_ptr<lyric_build::DependencyLoader>>
lyric_build::DependencyLoader::create(
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    std::shared_ptr<AbstractCache> cache,
    TempDirectory *tempDirectory)
{
    TU_ASSERT (cache != nullptr);

    absl::flat_hash_set<TaskKey> taskKeys;
    absl::flat_hash_set<lyric_common::ModuleLocation> objectLocations;
    absl::flat_hash_set<lyric_common::ModuleLocation> pluginLocations;

    absl::flat_hash_map<lyric_common::ModuleLocation,lyric_object::LyricObject> objects;
    absl::flat_hash_map<
        lyric_common::ModuleLocation,
        std::shared_ptr<const tempo_utils::ImmutableBytes>
    > plugins;

    for (const auto &entry : depStates) {
        const auto &taskKey = entry.first;
        const auto &taskState = entry.second;

        TraceId traceId(taskState.getHash(), taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(traceId);

        std::vector<ArtifactId> artifactsFound;

        MetadataWriter objectFilterWriter;
        objectFilterWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
        LyricMetadata objectFilter;
        TU_ASSIGN_OR_RETURN(objectFilter, objectFilterWriter.toMetadata());

        TU_ASSIGN_OR_RETURN (artifactsFound, cache->findArtifacts(
            generation, taskState.getHash(), {}, objectFilter));

        for (const auto &artifactId : artifactsFound) {
            LyricMetadata metadata;
            TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));
            auto objectMetadata = metadata.getMetadata();

            lyric_common::ModuleLocation location;
            TU_RETURN_IF_NOT_OK (objectMetadata.parseAttr(kLyricBuildModuleLocation, location));

            if (!location.isValid())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "invalid module location for artifact {}", artifactId.toString());
            if (objects.contains(location))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "loader found duplicate object {}", location.toString());

            std::shared_ptr<const tempo_utils::ImmutableBytes> content;
            TU_ASSIGN_OR_RETURN (content,  cache->loadContentFollowingLinks(artifactId));

            auto span = content->getSpan();
            if (!lyric_object::LyricObject::verify(span))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "loader found invalid object {}", location.toString());

            lyric_object::LyricObject object(content);
            objects[location] = object;
            objectLocations.insert(location);
        }

        MetadataWriter pluginFilterWriter;
        pluginFilterWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
        LyricMetadata pluginFilter;
        TU_ASSIGN_OR_RETURN(pluginFilter, pluginFilterWriter.toMetadata());

        TU_ASSIGN_OR_RETURN (artifactsFound, cache->findArtifacts(
            generation, taskState.getHash(), {}, pluginFilter));

        for (const auto &artifactId : artifactsFound) {
            LyricMetadata metadata;
            TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));
            auto pluginMetadata = metadata.getMetadata();

            lyric_common::ModuleLocation location;
            TU_RETURN_IF_NOT_OK (pluginMetadata.parseAttr(kLyricBuildModuleLocation, location));

            if (!location.isValid())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "invalid module location for artifact {}", artifactId.toString());
            if (plugins.contains(location))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "loader found duplicate plugin {}", location.toString());

            std::shared_ptr<const tempo_utils::ImmutableBytes> content;
            TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(artifactId));

            plugins[location] = content;
            pluginLocations.insert(location);
        }

        taskKeys.insert(taskKey);
    }

    TU_LOG_INFO << "built DependencyLoader from tasks=" << taskKeys
        << " objects=" << objectLocations
        << " plugins=" << pluginLocations;

    return std::shared_ptr<DependencyLoader>(
        new DependencyLoader(tempDirectory, objects, plugins));
}

tempo_utils::Result<std::shared_ptr<lyric_build::DependencyLoader>>
lyric_build::DependencyLoader::create(
    const TargetComputation &targetComputation,
    std::shared_ptr<AbstractCache> cache,
    TempDirectory *tempDirectory)
{
    auto taskId = targetComputation.getId();
    TaskKey taskKey(taskId.getDomain(), taskId.getId());
    return create({{taskKey, targetComputation.getState()}}, cache, tempDirectory);
}

tempo_utils::Result<std::shared_ptr<lyric_build::DependencyLoader>>
lyric_build::DependencyLoader::create(
    const TargetComputationSet &targetComputationSet,
    std::shared_ptr<AbstractCache> cache,
    TempDirectory *tempDirectory)
{
    absl::flat_hash_map<TaskKey,TaskState> depStates;
    for (auto it = targetComputationSet.targetsBegin(); it != targetComputationSet.targetsEnd(); it++) {
        TaskKey taskKey(it->first.getDomain(), it->first.getId());
        depStates[taskKey] = it->second.getState();
    }
    return create(depStates, cache, tempDirectory);
}
