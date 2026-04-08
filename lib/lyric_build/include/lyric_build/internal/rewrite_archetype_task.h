#ifndef LYRIC_BUILD_INTERNAL_REWRITE_MODULE_TASK_H
#define LYRIC_BUILD_INTERNAL_REWRITE_MODULE_TASK_H

#include <lyric_assembler/object_state.h>
#include <lyric_build/base_task.h>
#include <lyric_build/build_state.h>
#include <lyric_build/build_types.h>
#include <lyric_build/task_settings.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_rewriter/lyric_rewriter.h>

namespace lyric_build::internal {

    class RewriteArchetypeTask : public BaseTask {

    public:
        RewriteArchetypeTask(
            const tempo_utils::UUID &generation,
            const TaskKey &key,
            std::shared_ptr<tempo_tracing::TraceSpan> span);

        tempo_utils::Result<std::string> configureTask(
            const TaskSettings *config,
            AbstractVirtualFilesystem *virtualFilesystem) override;
        tempo_utils::Result<absl::flat_hash_set<TaskKey>> checkDependencies() override;
        Option<tempo_utils::Status> runTask(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation) override;

    private:
        lyric_common::ModuleLocation m_moduleLocation;
        lyric_rewriter::RewriterOptions m_rewriterOptions;
        TaskKey m_parseTarget;

        tempo_utils::Status configure(const TaskSettings *config);
        tempo_utils::Status rewriteModule(
            const std::string &taskHash,
            const absl::flat_hash_map<TaskKey,TaskState> &depStates,
            BuildState *generation);
    };

    BaseTask *new_rewrite_archetype_task(
        const tempo_utils::UUID &generation,
        const TaskKey &key,
        std::shared_ptr<tempo_tracing::TraceSpan> span);
}

#endif // LYRIC_BUILD_INTERNAL_REWRITE_MODULE_TASK_H
