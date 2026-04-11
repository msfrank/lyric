#ifndef LYRIC_BUILD_INTERNAL_COMPILE_OBJECT_TASK_H
#define LYRIC_BUILD_INTERNAL_COMPILE_OBJECT_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/lyric_parser.h>

namespace lyric_build::internal {

    class CompileObjectTask : public BaseTask {

        enum class Phase {
            Initial,
            AnalyzeImports,
            Complete,
        };

    public:
        CompileObjectTask(
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
        lyric_compiler::CompilerOptions m_compilerOptions;
        TaskKey m_parseTarget;
        TaskKey m_symbolizeTarget;
        Phase m_phase;

        tempo_utils::Status initial(const TaskSettings &settings);
        tempo_utils::Status analyzeImports();

        tempo_utils::Status compileModule(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskData> &depStates,
            BuildState *buildState);
    };

    BaseTask *new_compile_object_task(
        const BuildGeneration &generation,
        const TaskKey &key,
        std::weak_ptr<BuildState> buildState,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_COMPILE_OBJECT_TASK_H
