#ifndef LYRIC_COMPILER_MACRO_VISITOR_H
#define LYRIC_COMPILER_MACRO_VISITOR_H

#include <lyric_schema/compiler_schema.h>

#include "rewrite_processor.h"

namespace lyric_rewriter {

    class CompilerMacroVisitor : public AbstractNodeVisitor {
    public:
        explicit CompilerMacroVisitor(AbstractProcessorState *state);

        tempo_utils::Status enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) override;
        tempo_utils::Status exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx) override;

    private:
        AbstractProcessorState *m_state;
    };
}

#endif //LYRIC_COMPILER_MACRO_VISITOR_H
