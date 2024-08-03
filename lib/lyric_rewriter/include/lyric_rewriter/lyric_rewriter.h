#ifndef LYRIC_REWRITER_LYRIC_REWRITER_H
#define LYRIC_REWRITER_LYRIC_REWRITER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/assembly_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/module_cache.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/trace_recorder.h>

#include "abstract_rewrite_driver.h"

namespace lyric_rewriter {

    /**
     *
     */
    struct RewriterOptions {

        lyric_common::AssemblyLocation preludeLocation;

        std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache;

        std::shared_ptr<AbstractNodeVisitor> unknownVisitor;
    };

    class LyricRewriter {

    public:
        explicit LyricRewriter(const RewriterOptions &options);
        LyricRewriter(const LyricRewriter &other);

        tempo_utils::Result<lyric_parser::LyricArchetype> rewriteArchetype(
            const lyric_parser::LyricArchetype &archetype,
            const tempo_utils::Url &sourceUrl,
            std::shared_ptr<AbstractRewriteDriver> rewriteDriver,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder);

    private:
        RewriterOptions m_options;
    };
}

#endif // LYRIC_REWRITER_LYRIC_REWRITER_H