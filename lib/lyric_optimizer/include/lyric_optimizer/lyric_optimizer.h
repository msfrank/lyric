#ifndef LYRIC_OPTIMIZER_LYRIC_OPTIMIZER_H
#define LYRIC_OPTIMIZER_LYRIC_OPTIMIZER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_assembler/object_state.h>
#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/module_cache.h>
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/scope_manager.h>

namespace lyric_optimizer {

    /**
     *
     */
    struct OptimizerOptions {

        lyric_common::ModuleLocation preludeLocation;
    };

    class LyricOptimizer {

    public:
        LyricOptimizer(
            std::unique_ptr<lyric_assembler::ObjectState> &&objectState,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
            const OptimizerOptions &options);

        LyricOptimizer(
            const lyric_common::ModuleLocation &location,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder,
            const OptimizerOptions &options);

        tempo_utils::Status initialize();

        tempo_utils::Status optimizeCall(const lyric_common::SymbolPath &callPath);

        tempo_utils::Result<lyric_object::LyricObject> toObject() const;

    private:
        std::unique_ptr<lyric_assembler::ObjectState> m_objectState;
        std::shared_ptr<tempo_tracing::TraceRecorder> m_recorder;
        std::unique_ptr<tempo_tracing::ScopeManager> m_scopeManager;
        OptimizerOptions m_options;
    };
}

#endif // LYRIC_OPTIMIZER_LYRIC_OPTIMIZER_H
