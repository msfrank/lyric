#ifndef LYRIC_BUILD_INTERNAL_SYMBOLIZE_LINKAGE_TASK_H
#define LYRIC_BUILD_INTERNAL_SYMBOLIZE_LINKAGE_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_symbolizer/lyric_symbolizer.h>

namespace lyric_build::internal {

    class SymbolizeLinkageTask : public BaseTask {

    public:
        SymbolizeLinkageTask(
            const BuildGeneration &generation,
            const TaskKey &key,
            std::weak_ptr<BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(TaskHash &taskHash) override;
        tempo_utils::Status runTask(TempDirectory *tempDirectory) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_parser::ParserOptions m_parserOptions;
        lyric_assembler::ObjectStateOptions m_objectStateOptions;
        lyric_symbolizer::SymbolizerOptions m_symbolizerOptions;
        TaskKey m_parseTarget;
    };

    BaseTask *new_symbolize_linkage_task(
        const BuildGeneration &generation,
        const TaskKey &key,
        std::weak_ptr<BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_SYMBOLIZE_LINKAGE_TASK_H
