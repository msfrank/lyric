#ifndef LYRIC_BUILD_INTERNAL_ANALYZE_OUTLINE_TASK_H
#define LYRIC_BUILD_INTERNAL_ANALYZE_OUTLINE_TASK_H

#include <lyric_analyzer/lyric_analyzer.h>
#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/task_settings.h>
#include <lyric_build/build_types.h>

namespace lyric_build::internal {

    class AnalyzeOutlineTask : public BaseTask {

        enum class Phase {
            Initial,
            SymbolizeImports,
            Complete,
        };

    public:
        AnalyzeOutlineTask(
            const BuildGeneration &generation,
            const TaskKey &key,
            std::weak_ptr<BuildState> buildState,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Status configureTask(const TaskSettings &taskSettings) override;
        tempo_utils::Status deduplicateTask(TaskHash &taskHash) override;
        tempo_utils::Status runTask(TempDirectory *tempDirectory) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_assembler::ObjectStateOptions m_objectStateOptions;
        lyric_analyzer::AnalyzerOptions m_analyzerOptions;
        TaskKey m_parseTarget;
        TaskKey m_symbolizeTarget;
        Phase m_phase;

        tempo_utils::Status initial(const TaskSettings &settings);
        tempo_utils::Status symbolizeImports();
    };

    BaseTask *new_analyze_outline_task(
        const BuildGeneration &generation,
        const TaskKey &key,
        std::weak_ptr<BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_ANALYZE_OUTLINE_TASK_H
