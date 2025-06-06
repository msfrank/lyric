#include <chrono>
#include <thread>

#include <lyric_build/base_task.h>
#include <lyric_build/build_conversions.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/task_hasher.h>
#include <tempo_config/base_conversions.h>
#include <tempo_config/parse_config.h>
#include <tempo_utils/log_message.h>

#include "test_task.h"

TestTask::TestTask(
    const tempo_utils::UUID &generation,
    const lyric_build::TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, span)
{
}

tempo_utils::Status
TestTask::configure(const lyric_build::TaskSettings *config)
{
    auto taskId = getId();

    auto taskSection = config->getTaskSection(taskId);

    tempo_config::LongParser sleepTimeoutParser(1);
    tempo_config::BooleanParser shouldFailParser(false);
    lyric_build::TaskIdParser testDependencyParser;

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_sleepTimeout, sleepTimeoutParser, taskSection, "sleepTimeout"));
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_shouldFail, shouldFailParser, taskSection, "shouldFail"));

    auto testDependencies = config->getTaskNode(taskId, "testDependencies").toSeq();
    for (auto iterator = testDependencies.seqBegin(); iterator != testDependencies.seqEnd(); iterator++) {
        lyric_build::TaskId dep;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(dep, testDependencyParser, iterator->toValue()));
        m_dependencies.insert(lyric_build::TaskKey(dep.getDomain(), dep.getId()));
    }

    return {};
}

tempo_utils::Result<std::string>
TestTask::configureTask(
    const lyric_build::TaskSettings *config,
    lyric_build::AbstractFilesystem *virtualFilesystem)
{
    auto key = getKey();
    auto merged = config->merge(lyric_build::TaskSettings({}, {}, {{getId(), getParams()}}));

    auto status = configure(&merged);
    if (!status.isOk())
        return status;

    lyric_build::TaskHasher configHasher(getKey());
    configHasher.hashValue(m_sleepTimeout);
    configHasher.hashValue(m_shouldFail);
    std::vector<lyric_build::TaskKey> sortedDeps(m_dependencies.cbegin(), m_dependencies.cend());
    std::sort(sortedDeps.begin(), sortedDeps.end());
    for (const auto &dep : sortedDeps) {
        configHasher.hashValue(dep.toString());
    }

    return configHasher.finish();
}

tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>>
TestTask::checkDependencies()
{
    return m_dependencies;
}

Option<tempo_utils::Status>
TestTask::runTask(
    const std::string &taskHash,
    const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
    lyric_build::BuildState *generation)
{
    TU_LOG_INFO << "sleeping for"<< m_sleepTimeout << "seconds";
    std::this_thread::sleep_for(std::chrono::seconds(m_sleepTimeout));

    if (m_shouldFail) {
//        auto ec = make_error_code(BuildCondition::TASK_FAILURE);
//        putError(ec, "task should fail");
        return Option<tempo_utils::Status>(
            lyric_build::BuildStatus::forCondition(lyric_build::BuildCondition::kTaskFailure, "task should fail"));
    }

    return Option(tempo_utils::Status{});
}

lyric_build::BaseTask *
new_test_task(
    const tempo_utils::UUID &generation,
    const lyric_build::TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new TestTask(generation, key, span);
}
