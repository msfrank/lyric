#ifndef LYRIC_REWRITER_LYRIC_REWRITER_H
#define LYRIC_REWRITER_LYRIC_REWRITER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_object/lyric_object.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/trace_recorder.h>

#include "abstract_rewrite_driver.h"
#include "abstract_scan_driver.h"
#include "visitor_registry.h"

namespace lyric_rewriter {

    /**
     *
     */
    struct RewriterOptions {
        /**
         *
         */
        std::shared_ptr<VisitorRegistry> visitorRegistry = {};
    };

    class LyricRewriter {

    public:
        explicit LyricRewriter(const RewriterOptions &options);
        LyricRewriter(const LyricRewriter &other);

        tempo_utils::Result<lyric_parser::LyricArchetype> rewriteArchetype(
            const lyric_parser::LyricArchetype &archetype,
            const tempo_utils::Url &sourceUrl,
            std::shared_ptr<AbstractRewriteDriverBuilder> rewriteDriverBuilder,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder);

        tempo_utils::Status scanArchetype(
            const lyric_parser::LyricArchetype &archetype,
            const tempo_utils::Url &sourceUrl,
            std::shared_ptr<AbstractScanDriverBuilder> scanDriverBuilder,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder);

    private:
        RewriterOptions m_options;
    };
}

#endif // LYRIC_REWRITER_LYRIC_REWRITER_H