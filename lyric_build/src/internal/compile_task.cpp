
#include <lyric_build/base_task.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/internal/compile_task.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::CompileTask::CompileTask(
    const boost::uuids::uuid &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::CompileTask::configure(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto taskId = getId();

    // determine the base url containing source files
    tempo_config::UrlParser sourceBaseUrlParser;
    TU_RETURN_IF_NOT_OK(parse_config(m_baseUrl, sourceBaseUrlParser,
        config, taskId, "sourceBaseUrl"));

    // if the task id is not empty then it overrides the source base url
    if (!taskId.getId().empty()) {
        m_baseUrl = tempo_utils::Url::fromString(taskId.getId());
        if (!m_baseUrl.isValid())
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "task key id {} is not a valid url", taskId.getId());
    }

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    // determine the set of compile targets
    auto listResourcesResult = virtualFilesystem->listResourcesRecursively(m_baseUrl,
        [](const std::filesystem::path &p) {
            return p.extension() == lyric_common::kSourceFileDotSuffix;
        },
        {});
    if (listResourcesResult.isStatus())
        return listResourcesResult.getStatus();
    auto resourceList = listResourcesResult.getResult();
    for (auto &sourceUrl : resourceList.resources) {
        m_compileTargets.insert(TaskKey("compile_module", sourceUrl.toString()));
    }

    if (m_compileTargets.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task {} has no compile targets", getKey().toString());

    TU_LOG_INFO << "compile targets:" << m_compileTargets;

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::CompileTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    auto status = configure(&merged, virtualFilesystem);
    if (status.notOk())
        return status;
    return TaskHasher::uniqueHash();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::CompileTask::checkDependencies()
{
    return m_compileTargets;
}

Option<tempo_utils::Status>
lyric_build::internal::CompileTask::runTask(
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
            return Option<tempo_utils::Status>(findTargetArtifactsResult.getStatus());
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
lyric_build::internal::new_compile_task(
    const boost::uuids::uuid &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CompileTask(generation, key, span);
}