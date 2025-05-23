#ifndef LYRIC_BUILD_INTERNAL_COMPILE_MODULE_TASK_H
#define LYRIC_BUILD_INTERNAL_COMPILE_MODULE_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_compiler/lyric_compiler.h>
#include <lyric_parser/lyric_parser.h>

namespace lyric_build::internal {

    class CompileModuleTask : public BaseTask {

        enum class CompileModulePhase {
            ANALYZE_IMPORTS,
            COMPILE_MODULE,
            COMPLETE,
        };

    public:
        CompileModuleTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<std::string> configureTask(
            const lyric_build::TaskSettings *config,
            AbstractFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            lyric_build::BuildState *generation) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_assembler::ObjectStateOptions m_objectStateOptions;
        lyric_compiler::CompilerOptions m_compilerOptions;
        TaskKey m_parseTarget;
        TaskKey m_symbolizeTarget;
        TaskKey m_pluginTarget;
        absl::flat_hash_set<TaskKey> m_compileTargets;
        CompileModulePhase m_phase;

        tempo_utils::Status configure(const lyric_build::TaskSettings *config);
        tempo_utils::Status analyzeImports(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            lyric_build::BuildState *buildState);
        tempo_utils::Status compileModule(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey, TaskState> &depStates,
            lyric_build::BuildState *buildState);
    };

    BaseTask *new_compile_module_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_COMPILE_MODULE_TASK_H
