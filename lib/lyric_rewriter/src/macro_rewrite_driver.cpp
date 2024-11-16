
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/ast_schema.h>

lyric_rewriter::MacroRewriteDriver::MacroRewriteDriver(MacroRegistry *registry)
    : m_registry(registry),
      m_macroList(nullptr)
{
    TU_ASSERT (m_registry != nullptr);
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriver::enter(
    lyric_parser::ArchetypeState *state,
    lyric_parser::ArchetypeNode *node,
    VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    TU_LOG_INFO << "enter " << resource->getName() << " node with child index " << ctx.childIndex();

    auto astId = resource->getId();
    if (astId == lyric_schema::LyricAstId::MacroList) {
        TU_LOG_INFO << "skipping macro list children";
        ctx.setSkipChildren(true);
        m_macroList = node;
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriver::finish()
{
    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriver::rewriteMacroList(
    lyric_parser::ArchetypeNode *macroListNode,
    MacroBlock &macroBlock)
{

    for (int i = 0; i < macroListNode->numChildren(); i++) {
        auto *macroCallNode = macroListNode->getChild(i);

        lyric_schema::LyricAstId nodeId{};
        TU_RETURN_IF_NOT_OK (macroCallNode->parseId(lyric_schema::kLyricAstVocabulary, nodeId));
        if (nodeId != lyric_schema::LyricAstId::MacroCall)
            return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant, "expected MacroCall");

        std::string id;
        TU_RETURN_IF_NOT_OK (macroCallNode->parseAttr(lyric_parser::kLyricAstIdentifier, id));

        auto macro = m_registry->getMacro(id);
        if (macro == nullptr)
            return RewriterStatus::forCondition(
                RewriterCondition::kRewriterInvariant, "unknown macro '{}'", id);

        TU_RETURN_IF_NOT_OK (macro->rewriteBlock(macroCallNode, macroBlock, macroBlock.archetypeState()));
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriver::exit(
    lyric_parser::ArchetypeState *state,
    lyric_parser::ArchetypeNode *node,
    const VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    TU_LOG_INFO << "exit " << resource->getName() << " node with child index " << ctx.childIndex();

    if (m_macroList != nullptr) {
        m_macroList = nullptr;
        auto *parentNode = ctx.parentNode();
        auto *macroListNode = parentNode->removeChild(ctx.childIndex());
        MacroBlock macroBlock(parentNode, ctx.childIndex(), state);
        TU_RETURN_IF_NOT_OK (rewriteMacroList(macroListNode, macroBlock));
    }

    return {};
}