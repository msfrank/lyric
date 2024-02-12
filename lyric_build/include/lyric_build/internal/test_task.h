#ifndef LYRIC_BUILD_INTERNAL_TEST_TASK_H
#define LYRIC_BUILD_INTERNAL_TEST_TASK_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <boost/uuid/uuid.hpp>

#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/config_store.h>
#include <lyric_build/build_types.h>

namespace lyric_build::internal {

    class TestTask : public BaseTask {

    public:
        TestTask(
            const boost::uuids::uuid &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<std::string> configureTask(
            const ConfigStore *config,
            AbstractFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation) override;

    private:
        int64_t m_sleepTimeout;
        bool m_shouldFail;
        absl::flat_hash_set<TaskKey> m_dependencies;

        tempo_utils::Status configure(const ConfigStore *config);
    };

    BaseTask *new_test_task(
        const boost::uuids::uuid &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_TEST_TASK_H