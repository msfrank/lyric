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
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : BaseTask(generation, key, std::move(buildState), std::move(span))
{
}

tempo_utils::Status
TestTask::configureTask(const lyric_build::TaskSettings &taskSettings)
{
    auto taskId = getId();
    auto settings = taskSettings.merge(lyric_build::TaskSettings({}, {}, {{taskId, getParams()}}));

    auto taskSection = settings.getTaskSection(taskId);

    tempo_config::LongParser sleepTimeoutParser(1);
    tempo_config::BooleanParser shouldFailParser(false);
    lyric_build::TaskIdParser testDependencyParser;

    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_sleepTimeout, sleepTimeoutParser, taskSection, "sleepTimeout"));
    TU_RETURN_IF_NOT_OK(tempo_config::parse_config(m_shouldFail, shouldFailParser, taskSection, "shouldFail"));

    auto testDependencies = settings.getTaskNode(taskId, "testDependencies").toSeq();
    for (auto iterator = testDependencies.seqBegin(); iterator != testDependencies.seqEnd(); iterator++) {
        lyric_build::TaskId dep;
        TU_RETURN_IF_NOT_OK(tempo_config::parse_config(dep, testDependencyParser, iterator->toValue()));
        requestDependency(lyric_build::TaskKey(dep.getDomain(), dep.getId()));
    }

    return {};
}

tempo_utils::Status
TestTask::deduplicateTask(lyric_build::TaskHash &taskHash)
{
    lyric_build::TaskHasher hasher(getKey());
    hasher.hashValue(m_sleepTimeout);
    hasher.hashValue(m_shouldFail);
    hasher.hashTask(this);
    taskHash = hasher.finish();
    return {};
}

tempo_utils::Status
TestTask::runTask(lyric_build::TempDirectory *tempDirectory)
{
    TU_LOG_INFO << "sleeping for"<< m_sleepTimeout << "seconds";
    std::this_thread::sleep_for(std::chrono::seconds(m_sleepTimeout));

    if (m_shouldFail) {
        return lyric_build::BuildStatus::forCondition(
            lyric_build::BuildCondition::kTaskFailure, "task should fail");
    }
    return {};
}

lyric_build::BaseTask *
new_test_task(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::weak_ptr<lyric_build::BuildState> buildState,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
{
    return new TestTask(generation, key, std::move(buildState), std::move(span));
}
