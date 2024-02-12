#include <chrono>
#include <thread>

#include <lyric_build/base_task.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_build/internal/test_task.h>
#include <lyric_build/task_hasher.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>

lyric_build::internal::TestTask::TestTask(
    const boost::uuids::uuid &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
lyric_build::internal::TestTask::configure(const lyric_build::ConfigStore *config)
{
    auto taskId = getId();

    auto taskSection = config->getTaskSection(taskId);

    tempo_config::LongParser sleepTimeoutParser(1);
    tempo_config::BooleanParser shouldFailParser(false);
    TaskIdParser testDependencyParser;

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_sleepTimeout, sleepTimeoutParser, taskSection, "sleepTimeout"));
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_shouldFail, shouldFailParser, taskSection, "shouldFail"));

    auto testDependencies = config->getTaskNode(taskId, "testDependencies").toSeq();
    for (auto iterator = testDependencies.seqBegin(); iterator != testDependencies.seqEnd(); iterator++) {
        TaskId dep;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(dep, testDependencyParser, iterator->toValue()));
        m_dependencies.insert(TaskKey(dep.getDomain(), dep.getId()));
    }

    return BuildStatus::ok();
}

tempo_utils::Result<std::string>
lyric_build::internal::TestTask::configureTask(
    const lyric_build::ConfigStore *config,
    AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge({}, {}, {{getId(), getParams()}});

    auto status = configure(&merged);
    if (!status.isOk())
        return status;

    lyric_build::TaskHasher configHasher(getKey());
    configHasher.hashValue(m_sleepTimeout);
    configHasher.hashValue(m_shouldFail);
    std::vector<TaskKey> sortedDeps(m_dependencies.cbegin(), m_dependencies.cend());
    std::sort(sortedDeps.begin(), sortedDeps.end());
    for (const auto &dep : sortedDeps) {
        configHasher.hashValue(dep.toString());
    }

    return configHasher.finish();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
lyric_build::internal::TestTask::checkDependencies()
{
    return m_dependencies;
}

Option<tempo_utils::Status>
lyric_build::internal::TestTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<TaskKey,TaskState> &depStates,
    lyric_build::BuildState *generation)
{
    TU_LOG_INFO << "sleeping for"<< m_sleepTimeout << "seconds";
    std::this_thread::sleep_for(std::chrono::seconds(m_sleepTimeout));

    if (m_shouldFail) {
//        auto ec = make_error_code(BuildCondition::TASK_FAILURE);
//        putError(ec, "task should fail");
        return Option<tempo_utils::Status>(
            BuildStatus::forCondition(BuildCondition::kTaskFailure, "task should fail"));
    }

    return Option<tempo_utils::Status>(BuildStatus::ok());
}

lyric_build::BaseTask *
lyric_build::internal::new_test_task(
    const boost::uuids::uuid &generation,
    const TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new TestTask(generation, key, span);
}
