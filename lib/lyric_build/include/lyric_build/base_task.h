#ifndef LYRIC_BUILD_BASE_TASK_H
#define LYRIC_BUILD_BASE_TASK_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <tempo_tracing/trace_span.h>

#include "build_state.h"
#include "build_result.h"
#include "build_types.h"
#include "task_settings.h"
#include "temp_directory.h"

namespace lyric_build {

    class BaseTask {

    public:
        BaseTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);
        virtual ~BaseTask();

        tempo_utils::UUID getGeneration() const;
        TaskKey getKey() const;
        TaskId getId() const;
        tempo_config::ConfigMap getParams() const;

        std::shared_ptr<tempo_tracing::TraceSpan> getSpan() const;
        std::shared_ptr<tempo_tracing::TraceRecorder> traceDiagnostics();
        TempDirectory *tempDirectory();

        virtual tempo_utils::Result<std::string> configureTask(
            const TaskSettings *configStore,
            AbstractFilesystem *virtualFilesystem) = 0;
        virtual tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() = 0;
        virtual Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *buildState) = 0;

        Option<tempo_utils::Status> run(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *buildState);

    private:
        tempo_utils::UUID m_generation;
        TaskKey m_key;
        std::shared_ptr<tempo_tracing::TraceSpan> m_span;
        std::shared_ptr<tempo_tracing::TraceRecorder> m_diagnostics;
        std::unique_ptr<TempDirectory> m_tempDirectory;

        enum class State {
            READY,
            ACTIVE,
            DONE,
        };
        State m_state;

    public:
        /**
         *
         * @tparam T
         * @param serde
         * @param value
         * @return
         */
        template <typename T>
        void
        putTag(const tempo_schema::AttrSerde<T> &serde, const T &value)
        {
            if (m_span == nullptr)
                return;
            m_span->putTag(serde, value);
        }
    };
}

#endif // LYRIC_BUILD_BASE_TASK_H
