#ifndef LYRIC_BUILD_BASE_TASK_H
#define LYRIC_BUILD_BASE_TASK_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <tempo_tracing/trace_span.h>

#include "build_state.h"
#include "build_types.h"
#include "task_settings.h"
#include "temp_directory.h"

namespace lyric_build {

    namespace internal {
        class RunnerWorker;
    }

    class BaseTask {

    public:
        BaseTask(
            const BuildGeneration &generation,
            const TaskKey &key,
            std::weak_ptr<BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);
        virtual ~BaseTask();

        /*
         * constant data accessor methods which are safe to call from any thread
         */

        BuildGeneration getGeneration() const;
        TaskKey getKey() const;
        TaskId getId() const;
        TaskReference getReference() const;
        tempo_config::ConfigMap getParams() const;

        /*
         * mutable data accessor methods which are safe to call from any thread
         */

        TaskData getData() const;
        TaskState getState() const;
        TaskData setState(TaskState state);
        bool hasHash() const;
        TaskHash getHash() const;
        TaskData setHash(const TaskHash &taskHash);

        /**
         *
         * @param taskSettings
         * @return
         */
        virtual tempo_utils::Status configureTask(const TaskSettings &taskSettings) = 0;

        /**
         * @param taskHash
         * @return
         */
        virtual tempo_utils::Status deduplicateTask(TaskHash &taskHash) = 0;

        /**
         *
         * @param tempDirectory
         * @return
         */
        virtual tempo_utils::Status runTask(TempDirectory *tempDirectory) = 0;

        tempo_utils::Status run(tempo_utils::Status &taskStatus);

        tempo_utils::Status cancel();
        tempo_utils::Status fail(const tempo_utils::Status &status);

    protected:

        /*
         *
         */

        std::shared_ptr<BuildState> getBuildState() const;
        std::shared_ptr<tempo_tracing::TraceRecorder> traceDiagnostics();

        void requestDependency(const TaskKey &depKey);
        bool hasDependency(const TaskKey &depKey) const;
        bool dependenciesEmpty() const;
        absl::flat_hash_set<TaskKey>::const_iterator dependenciesBegin() const;
        absl::flat_hash_set<TaskKey>::const_iterator dependenciesEnd() const;
        int numDependencies() const;

        void markCompleted(const TaskKey &depKey, const TaskData &depState);
        bool hasCompleted(const TaskKey &depKey) const;
        TaskData getCompleted(const TaskKey &depKey) const;
        absl::btree_map<TaskKey,TaskData>::const_iterator completedBegin() const;
        absl::btree_map<TaskKey,TaskData>::const_iterator completedEnd() const;
        int numCompleted() const;

        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> getContent(
            const TaskKey &key,
            const tempo_utils::UrlPath &path,
            bool followLinks = false);
        tempo_utils::Result<LyricMetadata> getMetadata(
            const TaskKey &key,
            const tempo_utils::UrlPath &path,
            bool followLinks = false);

        tempo_utils::Status storeArtifact(const tempo_utils::UrlPath &path,
            std::shared_ptr<const tempo_utils::ImmutableBytes> content,
            const LyricMetadata &metadata = {});

        tempo_utils::Status linkArtifact(
            const TaskKey &key,
            const tempo_utils::UrlPath &path,
            const tempo_utils::UrlPath &overrideDestinationPath = {});
        tempo_utils::Status linkArtifactOverridingMetadata(
            const TaskKey &key,
            const tempo_utils::UrlPath &path,
            const LyricMetadata &overrideMetadata,
            const tempo_utils::UrlPath &overrideDestinationPath = {});



        std::shared_ptr<tempo_tracing::SpanLog> logInfo(std::string_view message);
        std::shared_ptr<tempo_tracing::SpanLog> logWarn(std::string_view message);
        std::shared_ptr<tempo_tracing::SpanLog> logError(std::string_view message);
        std::shared_ptr<tempo_tracing::SpanLog> logStatus(const tempo_utils::Status &status);

    private:
        BuildGeneration m_generation;
        TaskKey m_key;
        std::weak_ptr<BuildState> m_buildState;
        std::shared_ptr<tempo_tracing::TraceSpan> m_span;
        std::shared_ptr<tempo_tracing::TraceRecorder> m_diagnostics;
        std::unique_ptr<TempDirectory> m_tempDirectory;

        absl::flat_hash_set<TaskKey> m_dependencies;
        absl::btree_map<TaskKey,TaskData> m_completed;

        std::unique_ptr<absl::Mutex> m_lock;
        TaskState m_state ABSL_GUARDED_BY(m_lock);
        std::optional<TaskHash> m_hash ABSL_GUARDED_BY(m_lock);

        tempo_utils::Status complete(const tempo_utils::Status &status);

        friend class BuildRunner;
        friend class BuildState;
        friend class DependencyLoader;
        friend class TaskHasher;
        friend class internal::RunnerWorker;

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
