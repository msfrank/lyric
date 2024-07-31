
#include <lyric_rewriter/lyric_ast_class_visitor.h>

lyric_rewriter::LyricAstClassVisitor::LyricAstClassVisitor()
    : SchemaClassNodeVisitor<lyric_schema::LyricAstNs, lyric_schema::LyricAstId>(lyric_schema::kLyricAstVocabulary)
{
}
