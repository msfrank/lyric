#ifndef LYRIC_REWRITER_LYRIC_AST_CLASS_VISITOR_H
#define LYRIC_REWRITER_LYRIC_AST_CLASS_VISITOR_H

#include <lyric_schema/ast_schema.h>

#include "schema_class_node_visitor.h"

namespace lyric_rewriter {

    class LyricAstClassVisitor : public SchemaClassNodeVisitor<lyric_schema::LyricAstNs,lyric_schema::LyricAstId> {
    public:
        LyricAstClassVisitor();
    };
}

#endif // LYRIC_REWRITER_LYRIC_AST_CLASS_VISITOR_H
