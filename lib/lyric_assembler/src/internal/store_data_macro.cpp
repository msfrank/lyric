
#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/internal/store_data_macro.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/assembler_schema.h>

lyric_assembler::internal::StoreDataMacro::StoreDataMacro()
{
}

tempo_utils::Status
lyric_assembler::internal::StoreDataMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    lyric_rewriter::PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "StoreData macro is not valid in pragma context");
}

tempo_utils::Status
lyric_assembler::internal::StoreDataMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "StoreData macro is not valid in definition context");
}
tempo_utils::Status
lyric_assembler::internal::StoreDataMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_rewriter::MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_VV << "rewrite StoreData macro";

    if (macroCallNode->numChildren() != 1)
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected 1 argument for StoreData macro ");
    auto *arg0 = macroCallNode->getChild(0);

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (arg0->parseId(lyric_schema::kLyricAstVocabulary, astId));
    switch (astId) {
        case lyric_schema::LyricAstId::Name:
            break;
        case lyric_schema::LyricAstId::DataDeref: {
            // rewrite DataDeref as Target
            lyric_parser::ArchetypeNode *targetNode;
            TU_ASSIGN_OR_RETURN (targetNode, state->appendNode(lyric_schema::kLyricAstTargetClass, {}));
            for (auto it = arg0->childrenBegin(); it != arg0->childrenEnd(); it++) {
                targetNode->appendChild(*it);
            }
            arg0 = targetNode;
            break;
        }
        default:
            return lyric_rewriter::RewriterStatus::forCondition(
                lyric_rewriter::RewriterCondition::kSyntaxError,
                "invalid '{}' argument for StoreData macro",
                lyric_schema::kLyricAstVocabulary.getResource(astId)->getName());
    }

    lyric_parser::ArchetypeNode *storeDataNode;
    TU_ASSIGN_OR_RETURN (storeDataNode, state->appendNode(lyric_schema::kLyricAssemblerStoreDataClass, {}));
    storeDataNode->appendChild(arg0);

    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(storeDataNode));

    return {};
}
