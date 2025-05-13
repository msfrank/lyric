
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
    const tempo_utils::UUID &generation,
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
    tempo_config::UrlPathParser sourceBasePathParser(tempo_utils::UrlPath{});
    TU_RETURN_IF_NOT_OK(parse_config(m_sourceBasePath, sourceBasePathParser,
        config, taskId, "sourceBasePath"));

    // if the task id is not empty then it overrides the source base url
    if (!taskId.getId().empty()) {
        m_sourceBasePath = tempo_utils::UrlPath::fromString(taskId.getId());
        if (!m_sourceBasePath.isValid())
            return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
                "task key id {} is not a valid path", taskId.getId());
    }

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    // determine the set of compile targets
    ResourceList resourceList;
    TU_ASSIGN_OR_RETURN (resourceList, virtualFilesystem->listResourcesRecursively(m_sourceBasePath,
        [](const tempo_utils::UrlPath &p) {
            return p.getLast().partView().ends_with(lyric_common::kSourceFileDotSuffix);
        },
        {}));

    std::filesystem::path basePath(m_sourceBasePath.toString());
    for (auto &resourcePath : resourceList.resources) {
        std::filesystem::path path(resourcePath.toString());
        auto sourcePath = std::filesystem::relative(path, basePath);
        std::filesystem::path modulePath = "/";
        modulePath /= sourcePath;
        modulePath.replace_extension();
        m_compileTargets.insert(TaskKey("compile_module", modulePath.string()));
    }

    if (m_compileTargets.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task {} has no compile targets", getKey().toString());

    TU_LOG_INFO << "compile targets:" << m_compileTargets;

    return {};
}

tempo_utils::Result<std::string>
lyric_build::internal::CompileTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});
    TU_RETURN_IF_NOT_OK (configure(&merged, virtualFilesystem));
    return TaskHasher::uniqueHash();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::CompileTask::checkDependencies()
{
    return m_compileTargets;
}

tempo_utils::Status
lyric_build::internal::CompileTask::compile(
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
lyric_build::internal::CompileTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *buildState)
{
    auto status = compile(taskHash, depStates, buildState);
    return Option(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_compile_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new CompileTask(generation, key, span);
}