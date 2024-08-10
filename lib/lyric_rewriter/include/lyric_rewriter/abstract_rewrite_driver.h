#ifndef LYRIC_REWRITER_ABSTRACT_REWRITE_DRIVER_H
#define LYRIC_REWRITER_ABSTRACT_REWRITE_DRIVER_H

#include <lyric_parser/archetype_node.h>

#include "rewrite_processor.h"

namespace lyric_rewriter {

    class AbstractRewriteDriver {
    public:
        virtual ~AbstractRewriteDriver() = default;

        virtual tempo_utils::Status arrange(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children) = 0;

        virtual tempo_utils::Status enter(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            VisitorContext &ctx) = 0;

        virtual tempo_utils::Status exit(
            lyric_parser::ArchetypeState *state,
            lyric_parser::ArchetypeNode *node,
            const VisitorContext &ctx) = 0;
    };
}

#endif // LYRIC_REWRITER_ABSTRACT_REWRITE_DRIVER_H
