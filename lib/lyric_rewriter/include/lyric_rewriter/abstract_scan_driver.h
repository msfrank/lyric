#ifndef LYRIC_REWRITER_ABSTRACT_SCAN_DRIVER_H
#define LYRIC_REWRITER_ABSTRACT_SCAN_DRIVER_H

#include <lyric_parser/archetype_node.h>

#include "rewrite_processor.h"

namespace lyric_rewriter {

    class AbstractScanDriver {
    public:
        virtual ~AbstractScanDriver() = default;

        virtual tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            VisitorContext &ctx) = 0;

        virtual tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const VisitorContext &ctx) = 0;

        virtual tempo_utils::Status finish() = 0;
    };
}

#endif // LYRIC_REWRITER_ABSTRACT_SCAN_DRIVER_H
