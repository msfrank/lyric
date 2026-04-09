#ifndef LYRIC_BUILD_TEST_TASK_H
#define LYRIC_BUILD_TEST_TASK_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/build_types.h>

class TestTask : public lyric_build::BaseTask {

public:
    TestTask(
        const lyric_build::BuildGeneration &generation,
        const lyric_build::TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);

    tempo_utils::Result<lyric_build::TaskHash> configureTask(
        const lyric_build::TaskSettings *config,
        lyric_build::AbstractVirtualFilesystem *virtualFilesystem) override;
    tempo_utils::Result<absl::flat_hash_set<lyric_build::TaskKey>> checkDependencies() override;
    Option<tempo_utils::Status> runTask(
        const std::string &taskHash,
        const absl::flat_hash_map<lyric_build::TaskKey,lyric_build::TaskState> &depStates,
        lyric_build::BuildState *generation) override;

private:
    int64_t m_sleepTimeout;
    bool m_shouldFail;
    absl::flat_hash_set<lyric_build::TaskKey> m_dependencies;

    tempo_utils::Status configure(const lyric_build::TaskSettings *config);
};

lyric_build::BaseTask *new_test_task(
    const lyric_build::BuildGeneration &generation,
    const lyric_build::TaskKey &key,
    std::shared_ptr<tempo_tracing::TraceSpan> span);

#endif // LYRIC_BUILD_TEST_TASK_H
