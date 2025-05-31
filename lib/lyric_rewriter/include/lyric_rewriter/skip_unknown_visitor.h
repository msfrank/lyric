#ifndef LYRIC_REWRITER_SKIP_UNKNOWN_VISITOR_H
#define LYRIC_REWRITER_SKIP_UNKNOWN_VISITOR_H

#include "rewrite_processor.h"

namespace lyric_rewriter {

    class SkipUnknownVisitor : public AbstractNodeVisitor {
    public:
        explicit SkipUnknownVisitor(AbstractProcessorState *state);

        tempo_utils::Status enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) override;
        tempo_utils::Status exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx) override;

    private:
        AbstractProcessorState *m_state;
    };
}

#endif // LYRIC_REWRITER_SKIP_UNKNOWN_VISITOR_H
