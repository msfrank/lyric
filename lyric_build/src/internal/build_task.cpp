
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_types.h>
#include <lyric_build/internal/build_task.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::BuildTask::BuildTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::BuildTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    m_buildTargets.insert(TaskKey("compile", taskId.getId()));

    if (m_buildTargets.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task {} has no build targets", getKey().toString());

    TU_LOG_INFO << "build targets:" << m_buildTargets;

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::BuildTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    auto status = configure(&merged);
    if (status.notOk())
        return status;
    return TaskHasher::uniqueHash();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::BuildTask::checkDependencies()
{
    return m_buildTargets;
}

Option<tempo_utils::Status>
lyric_build::internal::BuildTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto cache = buildState->getCache();

    for (const auto &dep : depStates) {
        const auto &taskKey = dep.first;
        const auto &taskState = dep.second;

        // if the target state is not completed, then fail the task
        if (taskState.getStatus() != TaskState::Status::COMPLETED)
            return Option<tempo_utils::Status>(
                BuildStatus::forCondition(BuildCondition::kTaskFailure,
                    "dependent task {} did not complete", taskKey.toString()));

        auto hash = taskState.getHash();
        if (hash.empty())
            return Option<tempo_utils::Status>(
                BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "dependent task {} has invalid hash", taskKey.toString()));

        TraceId artifactTrace(hash, taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(artifactTrace);
        auto findTargetArtifactsResult = cache->findArtifacts(generation, hash, {}, {});
        if (findTargetArtifactsResult.isStatus())
            return Option<tempo_utils::Status>(
                findTargetArtifactsResult.getStatus());
        auto targetArtifacts = findTargetArtifactsResult.getResult();

        for (const auto &srcId : targetArtifacts) {
            ArtifactId dstId(getGeneration(), taskHash, srcId.getLocation());
            auto status = cache->linkArtifact(dstId, srcId);
            if (status.notOk())
                return Option<tempo_utils::Status>(status);
        }
    }

    return Option<tempo_utils::Status>(BuildStatus::ok());
}

lyric_build::BaseTask *
lyric_build::internal::new_build_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new BuildTask(generation, key, span);
}
