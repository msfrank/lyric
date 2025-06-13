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

        tempo_utils::Status run(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *buildState,
            bool &complete);

        tempo_utils::Status cancel(const std::string &taskHash, BuildState *buildState);

        std::shared_ptr<tempo_tracing::SpanLog> logInfo(std::string_view message);
        std::shared_ptr<tempo_tracing::SpanLog> logWarn(std::string_view message);
        std::shared_ptr<tempo_tracing::SpanLog> logError(std::string_view message);
        std::shared_ptr<tempo_tracing::SpanLog> logStatus(const tempo_utils::Status &status);

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

        tempo_utils::Status complete(
            const std::string &taskHash,
            BuildState *buildState,
            const tempo_utils::Status &status);

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
        /**
         *
         * @tparam Args
         * @param messageFmt
         * @param messageArgs
         */
        template<typename... Args>
        std::shared_ptr<tempo_tracing::SpanLog> logInfo(fmt::string_view messageFmt, Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return logInfo(message);
        }
        /**
         *
         * @tparam Args
         * @param messageFmt
         * @param messageArgs
         */
        template<typename... Args>
        std::shared_ptr<tempo_tracing::SpanLog> logWarn(fmt::string_view messageFmt, Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return logWarn(message);
        }
        /**
         *
         * @tparam Args
         * @param messageFmt
         * @param messageArgs
         */
        template<typename... Args>
        std::shared_ptr<tempo_tracing::SpanLog> logError(fmt::string_view messageFmt, Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return logError(message);
        }
    };
}

#endif // LYRIC_BUILD_BASE_TASK_H
