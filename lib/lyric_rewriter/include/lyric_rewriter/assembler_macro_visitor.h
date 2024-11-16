#ifndef LYRIC_ASSEMBLER_MACRO_VISITOR_H
#define LYRIC_ASSEMBLER_MACRO_VISITOR_H

#include <lyric_schema/assembler_schema.h>

#include "rewrite_processor.h"

namespace lyric_rewriter {

    class AssemblerMacroVisitor : public AbstractNodeVisitor {
    public:
        explicit AssemblerMacroVisitor(AbstractProcessorState *state);

        tempo_utils::Status enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) override;
        tempo_utils::Status exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx) override;

    private:
        AbstractProcessorState *m_state;
    };
}

#endif //LYRIC_ASSEMBLER_MACRO_VISITOR_H
