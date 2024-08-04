
#include <lyric_build/build_attrs.h>
#include <lyric_build/build_result.h>
#include <lyric_build/build_types.h>
#include <lyric_build/dependency_loader.h>
#include <lyric_build/metadata_writer.h>
#include <lyric_common/common_types.h>
#include <lyric_packaging/package_attrs.h>

/**
 * Private constructor.
 *
 * @param assemblies A map containing assemblies loaded from the cache.
 */
lyric_build::DependencyLoader::DependencyLoader(
    const absl::flat_hash_map<lyric_common::AssemblyLocation, lyric_object::LyricObject> &assemblies)
    : m_assemblies(assemblies)
{
}

tempo_utils::Result<bool>
lyric_build::DependencyLoader::hasAssembly(const lyric_common::AssemblyLocation &location) const
{
    return m_assemblies.contains(location);
}

tempo_utils::Result<Option<lyric_common::AssemblyLocation>>
lyric_build::DependencyLoader::resolveAssembly(const lyric_common::AssemblyLocation &location) const
{
    return Option(location);
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_build::DependencyLoader::loadAssembly(const lyric_common::AssemblyLocation &location)
{
    if (m_assemblies.contains(location))
        return Option(m_assemblies.at(location));

    return Option<lyric_object::LyricObject>();
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_build::DependencyLoader::loadPlugin(
    const lyric_common::AssemblyLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
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
            std::shared_ptr<AbstractCache> cache)
{
    TU_ASSERT (cache != nullptr);

    absl::flat_hash_map<lyric_common::AssemblyLocation,lyric_object::LyricObject> assemblies;

    for (const auto &entry : depStates) {
        const auto &taskKey = entry.first;
        const auto &taskState = entry.second;

        TraceId traceId(taskState.getHash(), taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(traceId);

        MetadataWriter writer;
        writer.putAttr(lyric_packaging::kLyricPackagingContentType, std::string(lyric_common::kObjectContentType));
        LyricMetadata filter;
        TU_ASSIGN_OR_RETURN(filter, writer.toMetadata());

        auto findArtifactsResult = cache->findArtifacts(generation, taskState.getHash(), {}, filter);
        if (findArtifactsResult.isStatus())
            return findArtifactsResult.getStatus();
        auto artifactsFound = findArtifactsResult.getResult();

        for (const auto &artifactId : artifactsFound) {
            auto loadMetadataResult = cache->loadMetadataFollowingLinks(artifactId);
            if (loadMetadataResult.isStatus())
                return loadMetadataResult.getStatus();
            auto metadata = loadMetadataResult.getResult().getMetadata();

            lyric_common::AssemblyLocation location;
            TU_RETURN_IF_NOT_OK (metadata.parseAttr(kLyricBuildAssemblyLocation, location));

            if (!location.isValid())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "invalid assembly location for artifact {}", artifactId.toString());
            if (assemblies.contains(location))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "loader found duplicate assembly {}", location.toString());

            auto loadContentResult = cache->loadContentFollowingLinks(artifactId);
            if (loadContentResult.isStatus())
                return loadContentResult.getStatus();
            auto bytes = loadContentResult.getResult();

            std::span<const tu_uint8> span(bytes->getData(), bytes->getSize());
            if (!lyric_object::LyricObject::verify(span))
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "loader found duplicate assembly {}", location.toString());

            lyric_object::LyricObject assembly(bytes);
            assemblies[location] = assembly;
        }
    }

    return std::shared_ptr<DependencyLoader>(new DependencyLoader(assemblies));
}

tempo_utils::Result<std::shared_ptr<lyric_build::DependencyLoader>>
lyric_build::DependencyLoader::create(
    const TargetComputation &targetComputation,
    std::shared_ptr<AbstractCache> cache)
{
    auto taskId = targetComputation.getId();
    TaskKey taskKey(taskId.getDomain(), taskId.getId());
    return create({{taskKey, targetComputation.getState()}}, cache);
}

tempo_utils::Result<std::shared_ptr<lyric_build::DependencyLoader>>
lyric_build::DependencyLoader::create(
    const TargetComputationSet &targetComputationSet,
    std::shared_ptr<AbstractCache> cache)
{
    absl::flat_hash_map<TaskKey,TaskState> depStates;
    for (auto it = targetComputationSet.targetsBegin(); it != targetComputationSet.targetsEnd(); it++) {
        TaskKey taskKey(it->first.getDomain(), it->first.getId());
        depStates[taskKey] = it->second.getState();
    }
    return create(depStates, cache);
}
