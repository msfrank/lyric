
#include <lyric_build/base_task.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/internal/archive_task.h>
#include <lyric_build/task_hasher.h>
#include <lyric_common/common_types.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::ArchiveTask::ArchiveTask(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::ArchiveTask::configure(const ConfigStore *config)
{
    auto taskId = getId();

    tempo_config::StringParser archiveNameParser;
    TaskIdParser buildTargetParser;

    //
    // config below comes only from the task section, it is not resolved from domain or global sections
    //

    // determine the set of build targets
    auto buildTargetsNode = config->getTaskNode(taskId, "buildTargets");

    if (buildTargetsNode.getNodeType() == tempo_config::ConfigNodeType::kSeq) {
        // if buildTargets exists and is a seq, then add each element as a task key
        auto buildTargetsSeq = buildTargetsNode.toSeq();
        for (auto iterator = buildTargetsSeq.seqBegin(); iterator != buildTargetsSeq.seqEnd(); iterator++) {
            TaskId buildTarget;
            TU_RETURN_IF_NOT_OK(tempo_config::parse_config(buildTarget, buildTargetParser, iterator->toValue()));
            m_archiveTargets.insert(TaskKey(buildTarget.getDomain(), buildTarget.getId()));
        }
    } else if (buildTargetsNode.getNodeType() == tempo_config::ConfigNodeType::kNil) {
        // otherwise if buildTargets is nil or missing then add a compile: task
        m_archiveTargets.insert(TaskKey("compile", std::string{}));
    } else {
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "invalid value for buildTargets; expected seq");
    }

    if (m_archiveTargets.empty())
        return BuildStatus::forCondition(BuildCondition::kInvalidConfiguration,
            "task {} has no build targets", getKey().toString());

    // determine the archive name, which is required
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_archiveName, archiveNameParser,
        config->getTaskNode(taskId, "archiveName")));
    m_archiveName.append(lyric_common::kObjectFileDotSuffix);

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::ArchiveTask::configureTask(
    const ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    auto status = configure(&merged);
    if (!status.isOk())
        return status;
    return TaskHasher::uniqueHash();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::ArchiveTask::checkDependencies()
{
    return m_archiveTargets;
}

Option<tempo_utils::Status>
lyric_build::internal::ArchiveTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    BuildState *generation)
{
    auto status = BuildStatus::forCondition(BuildCondition::kBuildInvariant,
        "archive task is unimplemented");
    return Option<tempo_utils::Status>(status);
}

lyric_build::BaseTask *
lyric_build::internal::new_archive_task(
    const tempo_utils::UUID &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new ArchiveTask(generation, key, span);
}
