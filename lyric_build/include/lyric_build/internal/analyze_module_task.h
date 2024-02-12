#ifndef LYRIC_BUILD_INTERNAL_ANALYZE_MODULE_TASK_H
#define LYRIC_BUILD_INTERNAL_ANALYZE_MODULE_TASK_H

#include <filesystem>

#include <absl/container/flat_hash_set.h>
#include <boost/uuid/uuid.hpp>

#include <lyric_analyzer/lyric_analyzer.h>
#include <lyric_assembler/assembly_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/config_store.h>
#include <lyric_build/build_types.h>
#include <lyric_parser/lyric_parser.h>

namespace lyric_build::internal {

    class AnalyzeModuleTask : public BaseTask {

        enum class AnalyzeModulePhase {
            SYMBOLIZE_IMPORTS,
            ANALYZE_MODULE,
            COMPLETE,
        };

    public:
        AnalyzeModuleTask(
            const boost::uuids::uuid &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<std::string> configureTask(
            const ConfigStore *config,
            AbstractFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation) override;

    private:
        tempo_utils::Url m_sourceUrl;
        lyric_common::AssemblyLocation m_moduleLocation;
        lyric_assembler::AssemblyStateOptions m_assemblyStateOptions;
        lyric_analyzer::AnalyzerOptions m_analyzerOptions;
        TaskKey m_parseTarget;
        TaskKey m_symbolizeTarget;
        absl::flat_hash_set<TaskKey> m_analyzeTargets;
        AnalyzeModulePhase m_phase;

        tempo_utils::Status configure(const ConfigStore *config);
        tempo_utils::Status symbolizeImports(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *buildState);
        tempo_utils::Status analyzeModule(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *buildState);
    };

    BaseTask *new_analyze_module_task(
        const boost::uuids::uuid &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_ANALYZE_MODULE_TASK_H
