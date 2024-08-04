#ifndef LYRIC_REWRITER_LYRIC_AST_MATCH_VISITOR_H
#define LYRIC_REWRITER_LYRIC_AST_MATCH_VISITOR_H

#include <lyric_schema/ast_schema.h>

#include "lyric_ast_base_visitor.h"

namespace lyric_rewriter {

    class LyricAstMatchVisitor : public LyricAstBaseVisitor {
    public:
        LyricAstMatchVisitor(lyric_schema::LyricAstId astId, LyricAstOptions *options);

        tempo_utils::Status enter(lyric_parser::ArchetypeNode *node, VisitorContext &ctx) override;
        tempo_utils::Status exit(lyric_parser::ArchetypeNode *node, const VisitorContext &ctx) override;

    private:
        lyric_schema::LyricAstId m_astId;
    };
}

#endif // LYRIC_REWRITER_LYRIC_AST_MATCH_VISITOR_H
