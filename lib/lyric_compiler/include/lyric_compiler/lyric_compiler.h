#ifndef LYRIC_COMPILER_LYRIC_COMPILER_H
#define LYRIC_COMPILER_LYRIC_COMPILER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_assembler/object_state.h>
#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_object/lyric_object.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_rewriter/visitor_registry.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/trace_recorder.h>

namespace lyric_compiler {

    struct CompilerOptions {
        /**
         *
         */
        std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver = {};
        /**
         *
         */
        std::shared_ptr<lyric_rewriter::VisitorRegistry> visitorRegistry = {};
    };

    class LyricCompiler {

    public:
        LyricCompiler(
            const lyric_common::ModuleLocation &origin,
            std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            const CompilerOptions &options);
        LyricCompiler(const LyricCompiler &other);

        tempo_utils::Result<lyric_object::LyricObject> compileModule(
            const lyric_common::ModuleLocation &location,
            const lyric_parser::LyricArchetype &archetype,
            const lyric_assembler::ObjectStateOptions &objectStateOptions,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder);

    private:
        lyric_common::ModuleLocation m_origin;
        std::shared_ptr<lyric_importer::ModuleCache> m_localModuleCache;
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        CompilerOptions m_options;
    };
}

#endif // LYRIC_COMPILER_LYRIC_COMPILER_H
