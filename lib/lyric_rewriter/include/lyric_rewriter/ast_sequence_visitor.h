#ifndef LYRIC_REWRITER_AST_SEQUENCE_VISITOR_H
#define LYRIC_REWRITER_AST_SEQUENCE_VISITOR_H

#include <lyric_schema/ast_schema.h>

#include "ast_base_visitor.h"

namespace lyric_rewriter {

    class AstSequenceVisitor : public AstBaseVisitor {
    public:
        AstSequenceVisitor(lyric_schema::LyricAstId astId, AbstractProcessorState *state);

        tempo_utils::Status enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) override;
        tempo_utils::Status exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx) override;

    private:
        lyric_schema::LyricAstId m_astId;
    };
}

#endif // LYRIC_REWRITER_AST_SEQUENCE_VISITOR_H
