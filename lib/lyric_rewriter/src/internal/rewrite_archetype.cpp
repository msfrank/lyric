
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/internal/rewrite_archetype.h>
#include <lyric_schema/ast_schema.h>

tempo_utils::Status
lyric_rewriter::internal::rewrite_block(
    const lyric_parser::NodeWalker &walker,
    lyric_rewriter::internal::EntryPoint &entryPoint)
{
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstBlockClass, 1);

    for (int i = 0; i < walker.numChildren(); i++) {
        auto status = rewrite_node(walker.getChild(i), entryPoint);
        if (!status.isOk())
            return status;
    }

    return lyric_rewriter::RewriterStatus::ok();
}

static tempo_utils::Status
rewrite_namespace(
    const lyric_parser::NodeWalker &walker,
    lyric_rewriter::internal::EntryPoint &entryPoint)
{
    TU_ASSERT (walker.isValid());
    entryPoint.checkClassAndChildCountOrThrow(walker, lyric_schema::kLyricAstNamespaceClass, 1);
    return rewrite_block(walker.getChild(0), entryPoint);
}

tempo_utils::Status
lyric_rewriter::internal::rewrite_node(
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT (walker.isValid());

    lyric_schema::LyricAstId nodeId{};
    entryPoint.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, nodeId);

    switch (nodeId) {

        // ignored terminal forms
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef:
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::Target:
        case lyric_schema::LyricAstId::Using:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:
            return {};

        // handled forms
        case lyric_schema::LyricAstId::Block:
            return rewrite_block(walker, entryPoint);
        case lyric_schema::LyricAstId::Namespace:
            return rewrite_namespace(walker, entryPoint);

        default:
            return {};
    }

    auto *state = entryPoint.getState();
    state->throwParseInvariant(walker.getLocation(), "invalid node");
    TU_UNREACHABLE();
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_rewriter::internal::rewrite_root(
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    TU_ASSERT (walker.isValid());
    auto *state = entryPoint.getState();

    // scan assembly starting at entry node
    TU_RETURN_IF_NOT_OK (rewrite_node(walker, entryPoint));

    // construct assembly from assembly state and return it
    return state->toArchetype();
}
