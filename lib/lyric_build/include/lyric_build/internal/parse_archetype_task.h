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
            std::weak_ptr<BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(TaskHash &taskHash) override;
        tempo_utils::Status runTask(TempDirectory *tempDirectory) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        tempo_utils::UrlPath m_sourcePath;
        lyric_parser::ParserOptions m_parserOptions;
        Resource m_resource;
    };

    BaseTask *new_parse_archetype_task(
        const BuildGeneration &generation,
        const TaskKey &key,
        std::weak_ptr<BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_PARSE_ARCHETYPE_TASK_H
