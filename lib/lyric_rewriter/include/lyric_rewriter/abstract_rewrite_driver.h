#ifndef LYRIC_REWRITER_ABSTRACT_REWRITE_DRIVER_H
#define LYRIC_REWRITER_ABSTRACT_REWRITE_DRIVER_H

#include <lyric_parser/archetype_node.h>

#include "pragma_context.h"
#include "rewrite_processor.h"

namespace lyric_rewriter {

    class AbstractRewriteDriver {
    public:
        virtual ~AbstractRewriteDriver() = default;

        virtual tempo_utils::Status enter(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            VisitorContext &ctx) = 0;

        virtual tempo_utils::Status exit(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            const VisitorContext &ctx) = 0;

        virtual tempo_utils::Status finish() = 0;
    };

    class AbstractRewriteDriverBuilder {
    public:
        virtual ~AbstractRewriteDriverBuilder() = default;

        virtual tempo_utils::Status rewritePragma(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            PragmaContext &ctx) = 0;

        virtual tempo_utils::Result<std::shared_ptr<AbstractRewriteDriver>> makeRewriteDriver() = 0;
    };
}

#endif // LYRIC_REWRITER_ABSTRACT_REWRITE_DRIVER_H
