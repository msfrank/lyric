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
            ANALYZE_IMPORTS,
            COMPILE_OBJECT,
            COMPLETE,
        };

    public:
        CompileObjectTask(
            const BuildGeneration &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<TaskHash> configureTask(
            const TaskSettings *config,
            AbstractVirtualFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            BuildState *generation) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_assembler::ObjectStateOptions m_objectStateOptions;
        lyric_compiler::CompilerOptions m_compilerOptions;
        TaskKey m_parseTarget;
        TaskKey m_symbolizeTarget;
        TaskKey m_pluginTarget;
        absl::flat_hash_set<TaskKey> m_compileTargets;
        Phase m_phase;

        tempo_utils::Status configure(const TaskSettings *config);
        tempo_utils::Status analyzeImports(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            BuildState *buildState);
        tempo_utils::Status compileModule(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            BuildState *buildState);
    };

    BaseTask *new_compile_object_task(
        const BuildGeneration &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_COMPILE_OBJECT_TASK_H
