#ifndef LYRIC_BUILD_BASE_TASK_H
#define LYRIC_BUILD_BASE_TASK_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_build/build_state.h>
#include <lyric_build/build_result.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <tempo_tracing/trace_span.h>
#include <tempo_utils/attr.h>
#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

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
        //void setSpan(std::shared_ptr<tempo_tracing::TraceSpan> span);

//        void putTag(const tempo_utils::Attr &tag);
//        void putTags(std::initializer_list<tempo_utils::Attr> tags);
//        void putLog(std::initializer_list<tempo_utils::Attr> fields);
//        void putError(std::error_code ec, std::string_view message);

        virtual tempo_utils::Result<std::string> configureTask(
            const ConfigStore *configStore,
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
        putTag(const tempo_utils::AttrSerde<T> &serde, const T &value)
        {
            if (m_span == nullptr)
                return;
            m_span->putTag(serde, value);
        }
    };
}

#endif // LYRIC_BUILD_BASE_TASK_H
