#ifndef LYRIC_BUILD_INTERNAL_SYMBOLIZE_MODULE_TASK_H
#define LYRIC_BUILD_INTERNAL_SYMBOLIZE_MODULE_TASK_H

#include <filesystem>

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/config_store.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_symbolizer/lyric_symbolizer.h>

namespace lyric_build::internal {

    class SymbolizeModuleTask : public BaseTask {

    public:
        SymbolizeModuleTask(
            const tempo_utils::UUID &generation,
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
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_parser::ParserOptions m_parserOptions;
        lyric_assembler::ObjectStateOptions m_objectStateOptions;
        lyric_symbolizer::SymbolizerOptions m_symbolizerOptions;
        TaskKey m_parseTarget;

        tempo_utils::Status configure(const ConfigStore *config);
    };

    BaseTask *new_symbolize_module_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_SYMBOLIZE_MODULE_TASK_H
