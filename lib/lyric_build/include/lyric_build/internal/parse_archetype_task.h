#ifndef LYRIC_BUILD_INTERNAL_PARSE_ARCHETYPE_TASK_H
#define LYRIC_BUILD_INTERNAL_PARSE_ARCHETYPE_TASK_H

#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_parser/lyric_parser.h>

namespace lyric_build::internal {

    class ParseArchetypeTask : public BaseTask {

    public:
        ParseArchetypeTask(
            const BuildGeneration &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<TaskHash> configureTask(
            const TaskSettings *config,
            AbstractVirtualFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        tempo_utils::UrlPath m_sourcePath;
        lyric_parser::ParserOptions m_parserOptions;
        std::string m_resourceId;

        tempo_utils::Status configure(const TaskSettings *config);
        tempo_utils::Status parseModule(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation);
    };

    BaseTask *new_parse_archetype_task(
        const BuildGeneration &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_PARSE_ARCHETYPE_TASK_H
