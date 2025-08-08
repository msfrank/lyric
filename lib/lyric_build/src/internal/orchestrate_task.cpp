
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_types.h>
#include <lyric_build/internal/orchestrate_task.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/container_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::OrchestrateTask::OrchestrateTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::OrchestrateTask::configure(const TaskSettings *config)
{
    auto taskId = getId();

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    // determine the set of build targets
    auto taskSection = config->getTaskSection(taskId);

    TaskIdParser targetIdParser;
    tempo_config::SetTParser<TaskId> orchestrateTargetsParser(&targetIdParser);

    absl::flat_hash_set<TaskId> orchestrateTargets;
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(orchestrateTargets, orchestrateTargetsParser,
        taskSection, "orchestrateTargets"));

    for (const auto &target : orchestrateTargets) {
        m_orchestrateTargets.insert(TaskKey(target.getDomain(), target.getId()));
    }

    if (m_orchestrateTargets.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task {} has no orchestrate targets declared", getKey().toString());

    TU_LOG_V << "orchestrate targets:" << m_orchestrateTargets;

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::OrchestrateTask::configureTask(
    const TaskSettings *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge(TaskSettings({}, {}, {{getId(), getParams()}}));
    TU_RETURN_IF_NOT_OK (configure(&merged));
    return TaskHasher::uniqueHash();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::OrchestrateTask::checkDependencies()
{
    return m_orchestrateTargets;
}

tempo_utils::Status
lyric_build::internal::OrchestrateTask::orchestrate(
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
            return BuildStatus::forCondition(BuildCondition::kTaskFailure,
                "dependent task {} did not complete", taskKey.toString());

        auto hash = taskState.getHash();
        if (hash.empty())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "dependent task {} has invalid hash", taskKey.toString());

        TraceId artifactTrace(hash, taskKey.getDomain(), taskKey.getId());
        auto generation = cache->loadTrace(artifactTrace);

        std::vector<ArtifactId> targetArtifacts;
        TU_ASSIGN_OR_RETURN (targetArtifacts, cache->findArtifacts(generation, hash, {}, {}));

        for (const auto &srcId : targetArtifacts) {
            ArtifactId dstId(getGeneration(), taskHash, srcId.getLocation());
            TU_RETURN_IF_NOT_OK (cache->linkArtifact(dstId, srcId));
        }
    }

    return {};
}

Option<tempo_utils::Status>
lyric_build::internal::OrchestrateTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = orchestrate(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_orchestrate_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new OrchestrateTask(generation, key, span);
}
