#ifndef LYRIC_REWRITER_AST_BASE_VISITOR_H
#define LYRIC_REWRITER_AST_BASE_VISITOR_H

#include <lyric_schema/ast_schema.h>

#include "rewrite_processor.h"

namespace lyric_rewriter {

    class AstBaseVisitor : public AbstractNodeVisitor {
    public:
        explicit AstBaseVisitor(AbstractProcessorState *state);
        ~AstBaseVisitor() override;

    protected:
        tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>> makeVisitor(
            const lyric_parser::ArchetypeNode *node);

        tempo_utils::Status invokeEnter(
            lyric_schema::LyricAstId astId,
            lyric_parser::ArchetypeNode *node,
            VisitorContext &ctx);
        tempo_utils::Status invokeExit(
            lyric_schema::LyricAstId astId,
            lyric_parser::ArchetypeNode *node,
            const VisitorContext &ctx);

    private:
        AbstractProcessorState *m_state;
    };
}

#endif // LYRIC_REWRITER_AST_BASE_VISITOR_H
