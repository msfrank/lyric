
#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/internal/load_data_macro.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/assembler_schema.h>

lyric_assembler::internal::LoadDataMacro::LoadDataMacro()
{
}

tempo_utils::Status
lyric_assembler::internal::LoadDataMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    lyric_rewriter::PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "LoadData macro is not valid in pragma context");
}

tempo_utils::Status
lyric_assembler::internal::LoadDataMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "LoadData macro is not valid in definition context");
}
tempo_utils::Status
lyric_assembler::internal::LoadDataMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_rewriter::MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_VV << "rewrite LoadData macro";

    if (macroCallNode->numChildren() != 1)
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected 1 argument for LoadData macro ");
    auto *arg0 = macroCallNode->getChild(0);

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (arg0->parseId(lyric_schema::kLyricAstVocabulary, astId));
    switch (astId) {
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::DataDeref:
        case lyric_schema::LyricAstId::SymbolDeref:
            break;
        default:
            return lyric_rewriter::RewriterStatus::forCondition(
                lyric_rewriter::RewriterCondition::kSyntaxError,
                "invalid '{}' argument for LoadData macro",
                lyric_schema::kLyricAstVocabulary.getResource(astId)->getName());
    }

    lyric_parser::ArchetypeNode *loadDataNode;
    TU_ASSIGN_OR_RETURN (loadDataNode, state->appendNode(lyric_schema::kLyricAssemblerLoadDataClass, {}));
    loadDataNode->appendChild(arg0);

    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(loadDataNode));

    return {};
}
