
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
    std::shared_ptr<AbstractCache> cache,
    TempDirectory *tempDirectory,
    const absl::flat_hash_map<lyric_common::ModuleLocation, lyric_object::LyricObject> &objects,
    const absl::flat_hash_map<lyric_common::ModuleLocation, ArtifactId> &plugins)
    : m_cache(std::move(cache)),
      m_tempDirectory(tempDirectory),
      m_objects(objects),
      m_plugins(plugins)
{
    TU_ASSERT (m_cache != nullptr);
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

    // construct the plugin path
    auto pluginRoot = tempo_utils::UrlPath::fromString("/")
        .traverse(
            tempo_utils::UrlPathPart(tempo_utils::generate_name("XXXXXXXX")));
    auto pluginFilename = lyric_common::pluginFilename(location.getModuleName());
    auto pluginPath = pluginRoot.traverse(
        tempo_utils::UrlPathPart("modules"),
        location.getPath().getInit().toRelative(),
        tempo_utils::UrlPathPart(pluginFilename));

    // write the plugin to a file in the temp directory
    std::shared_ptr<const tempo_utils::ImmutableBytes> content;
    TU_ASSIGN_OR_RETURN (content, m_cache->loadContentFollowingLinks(pluginEntry->second));
    std::filesystem::path absolutePath;
    TU_ASSIGN_OR_RETURN (absolutePath, m_tempDirectory->putContent(pluginPath, content));

    //
    LyricMetadata metadata;
    TU_ASSIGN_OR_RETURN (metadata, m_cache->loadMetadataFollowingLinks(pluginEntry->second));

    // if runtime lib directory attr is present then create directory symlink to it
    if (metadata.hasAttr(kLyricBuildRuntimeLibDirectory)) {
        std::filesystem::path runtimeLibDirectory;
        TU_RETURN_IF_NOT_OK (metadata.parseAttr(kLyricBuildRuntimeLibDirectory, runtimeLibDirectory));
        auto linkPath = pluginRoot.traverse(tempo_utils::UrlPathPart("runtime-lib"));
        TU_RETURN_IF_STATUS (m_tempDirectory->makeSymlink(linkPath, runtimeLibDirectory));
    }

    // if lib directory attr is present then create directory symlink to it
    if (metadata.hasAttr(kLyricBuildLibDirectory)) {
        std::filesystem::path LibDirectory;
        TU_RETURN_IF_NOT_OK (metadata.parseAttr(kLyricBuildLibDirectory, LibDirectory));
        auto linkPath = pluginRoot.traverse(tempo_utils::UrlPathPart("lib"));
        TU_RETURN_IF_STATUS (m_tempDirectory->makeSymlink(linkPath, LibDirectory));
    }

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

    TU_LOG_V << "loaded plugin " << absolutePath;

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
    const lyric_common::ModuleLocation &origin,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    std::shared_ptr<AbstractCache> cache,
    TempDirectory *tempDirectory)
{
    TU_ASSERT (cache != nullptr);

    absl::flat_hash_set<TaskKey> taskKeys;
    absl::flat_hash_set<lyric_common::ModuleLocation> objectLocations;
    absl::flat_hash_set<lyric_common::ModuleLocation> pluginLocations;

    absl::flat_hash_map<lyric_common::ModuleLocation,lyric_object::LyricObject> objects;
    absl::flat_hash_map<lyric_common::ModuleLocation,ArtifactId> plugins;

    for (const auto &entry : depStates) {
        const auto &taskKey = entry.first;
        const auto &taskState = entry.second;

        TraceId traceId(taskState.getHash(), taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(traceId);

        std::vector<ArtifactId> artifactsFound;

        MetadataWriter objectFilterWriter;
        TU_RETURN_IF_NOT_OK (objectFilterWriter.configure());
        objectFilterWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kObjectContentType));
        LyricMetadata objectFilter;
        TU_ASSIGN_OR_RETURN(objectFilter, objectFilterWriter.toMetadata());

        TU_ASSIGN_OR_RETURN (artifactsFound, cache->findArtifacts(
            generation, taskState.getHash(), {}, objectFilter));

        for (const auto &artifactId : artifactsFound) {
            LyricMetadata metadata;
            TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));

            lyric_common::ModuleLocation location;
            TU_RETURN_IF_NOT_OK (metadata.parseAttr(kLyricBuildModuleLocation, location));

            if (!location.isValid())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "invalid module location for artifact {}", artifactId.toString());

            // if module location is relative then construct an absolute location using origin as the base
            if (location.isRelative()) {
                location = origin.resolve(location);
            }

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
        TU_RETURN_IF_NOT_OK (pluginFilterWriter.configure());
        pluginFilterWriter.putAttr(kLyricBuildContentType, std::string(lyric_common::kPluginContentType));
        LyricMetadata pluginFilter;
        TU_ASSIGN_OR_RETURN(pluginFilter, pluginFilterWriter.toMetadata());

        TU_ASSIGN_OR_RETURN (artifactsFound, cache->findArtifacts(
            generation, taskState.getHash(), {}, pluginFilter));

        for (const auto &artifactId : artifactsFound) {
            LyricMetadata metadata;
            TU_ASSIGN_OR_RETURN (metadata, cache->loadMetadataFollowingLinks(artifactId));

            lyric_common::ModuleLocation location;
            TU_RETURN_IF_NOT_OK (metadata.parseAttr(kLyricBuildModuleLocation, location));
            if (!location.isValid())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "invalid module location for artifact {}", artifactId.toString());

            // if module location is relative then construct an absolute location using origin as the base
            if (location.isRelative()) {
                location = origin.resolve(location);
            }

            if (plugins.contains(location))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "loader found duplicate plugin {}", location.toString());

            //std::shared_ptr<const tempo_utils::ImmutableBytes> content;
            //TU_ASSIGN_OR_RETURN (content, cache->loadContentFollowingLinks(artifactId));

            plugins[location] = artifactId;
            pluginLocations.insert(location);
        }

        taskKeys.insert(taskKey);
    }

    TU_LOG_V << "built DependencyLoader from tasks=" << taskKeys
        << " objects=" << objectLocations
        << " plugins=" << pluginLocations;

    return std::shared_ptr<DependencyLoader>(
        new DependencyLoader(std::move(cache), tempDirectory, objects, plugins));
}

tempo_utils::Result<std::shared_ptr<lyric_build::DependencyLoader>>
lyric_build::DependencyLoader::create(
    const lyric_common::ModuleLocation &origin,
    const TargetComputation &targetComputation,
    std::shared_ptr<AbstractCache> cache,
    TempDirectory *tempDirectory)
{
    auto taskId = targetComputation.getId();
    TaskKey taskKey(taskId.getDomain(), taskId.getId());
    return create(origin, {{taskKey, targetComputation.getState()}}, cache, tempDirectory);
}

tempo_utils::Result<std::shared_ptr<lyric_build::DependencyLoader>>
lyric_build::DependencyLoader::create(
    const lyric_common::ModuleLocation &origin,
    const TargetComputationSet &targetComputationSet,
    std::shared_ptr<AbstractCache> cache,
    TempDirectory *tempDirectory)
{
    absl::flat_hash_map<TaskKey,TaskState> depStates;
    for (auto it = targetComputationSet.targetsBegin(); it != targetComputationSet.targetsEnd(); it++) {
        TaskKey taskKey(it->first.getDomain(), it->first.getId());
        depStates[taskKey] = it->second.getState();
    }
    return create(origin, depStates, cache, tempDirectory);
}
