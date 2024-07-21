
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/internal/rewrite_archetype.h>
#include <lyric_schema/ast_schema.h>

tempo_utils::Status
lyric_rewriter::internal::rewrite_macro_list(
    lyric_parser::ArchetypeNode *blockNode,
    int index,
    lyric_parser::ArchetypeNode *macroListNode,
    lyric_rewriter::internal::EntryPoint &entryPoint)
{
    auto *registry = entryPoint.getRegistry();
    auto *state = entryPoint.getState();

    MacroBlock macroBlock(blockNode, index, state);

    for (int i = 0; i < macroListNode->numChildren(); i++) {
        auto *child = macroListNode->getChild(i);
        lyric_schema::LyricAstId nodeId{};
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, nodeId));
        if (nodeId != lyric_schema::LyricAstId::MacroCall)
            return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant, "expected MacroCall");
        auto value = child->getAttrValue(lyric_schema::kLyricAstIdentifierProperty);
        if (!value.isLiteral())
            return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant, "invalid MacroCall");
        auto literal = value.getLiteral();
        if (literal.getType() != tempo_utils::ValueType::String)
            return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant, "invalid MacroCall");
        auto identifier = literal.getString();
        auto macro = registry->getMacro(identifier);
        if (macro == nullptr)
            return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant, "unknown macro '{}'", identifier);
        TU_RETURN_IF_NOT_OK (macro->rewriteBlock(child, macroBlock, state));
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::internal::rewrite_block(
    lyric_parser::ArchetypeNode *blockNode,
    lyric_rewriter::internal::EntryPoint &entryPoint)
{
    TU_ASSERT (blockNode != nullptr);
    for (int i = 0; i < blockNode->numChildren(); i++) {
        auto *child = blockNode->getChild(i);
        lyric_schema::LyricAstId nodeId{};
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, nodeId));
        if (nodeId == lyric_schema::LyricAstId::MacroList) {
            TU_RETURN_IF_NOT_OK (rewrite_macro_list(blockNode, i, child, entryPoint));
        } else {
            TU_RETURN_IF_NOT_OK (rewrite_node(blockNode->getChild(i), entryPoint));
        }
    }
    return {};
}

static tempo_utils::Status
rewrite_namespace(
    lyric_parser::ArchetypeNode *namespaceNode,
    lyric_rewriter::internal::EntryPoint &entryPoint)
{
    TU_ASSERT (namespaceNode != nullptr);
    if (namespaceNode->numChildren() > 0)
        return rewrite_block(namespaceNode->getChild(0), entryPoint);
    return {};
}

tempo_utils::Status
lyric_rewriter::internal::rewrite_node(
    lyric_parser::ArchetypeNode *node,
    EntryPoint &entryPoint)
{
    TU_ASSERT (node != nullptr);

    lyric_schema::LyricAstId nodeId{};
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, nodeId));

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
            return rewrite_block(node, entryPoint);
        case lyric_schema::LyricAstId::Namespace:
            return rewrite_namespace(node, entryPoint);

        default:
            return {};
    }
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_rewriter::internal::rewrite_root(
    lyric_parser::ArchetypeNode *root,
    EntryPoint &entryPoint)
{
    auto *state = entryPoint.getState();

    // scan archetype starting at root node
    TU_RETURN_IF_NOT_OK (rewrite_node(root, entryPoint));

    // construct rewritten archetype from state and return it
    return state->toArchetype();
}
