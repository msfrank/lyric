
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_schema/ast_schema.h>

lyric_rewriter::MacroRewriteDriver::MacroRewriteDriver(std::shared_ptr<MacroRegistry> registry)
    : m_registry(std::move(registry)),
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
    auto astId = resource->getId();

    TU_LOG_INFO << "enter " << resource->getName() << " node with child index " << ctx.childIndex();

    if (astId == lyric_schema::LyricAstId::MacroList) {
        TU_LOG_INFO << "skipping macro list children";
        ctx.setSkipChildren(true);
        m_macroList = node;
    } else if (node->hasAttr(lyric_parser::kLyricAstMacroListOffset)) {
        lyric_parser::ArchetypeNode *macroListNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstMacroListOffset, macroListNode));
        TU_RETURN_IF_NOT_OK (rewriteMacroDefinition(macroListNode, node, state));
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriver::finish()
{
    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriver::rewriteMacroDefinition(
    lyric_parser::ArchetypeNode *macroListNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    for (int i = 0; i < macroListNode->numChildren(); i++) {
        auto *macroCallNode = macroListNode->getChild(i);

        lyric_schema::LyricAstId nodeId{};
        TU_RETURN_IF_NOT_OK (macroCallNode->parseId(lyric_schema::kLyricAstVocabulary, nodeId));
        if (nodeId != lyric_schema::LyricAstId::MacroCall)
            return RewriterStatus::forCondition(RewriterCondition::kRewriterInvariant, "expected MacroCall");

        std::string id;
        TU_RETURN_IF_NOT_OK (macroCallNode->parseAttr(lyric_parser::kLyricAstIdentifier, id));

        std::shared_ptr<AbstractMacro> macro;
        TU_ASSIGN_OR_RETURN (macro, m_registry->makeMacro(id));

        TU_RETURN_IF_NOT_OK (macro->rewriteDefinition(macroCallNode, definitionNode, state));
    }

    return {};
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriver::rewriteMacroBlock(
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

        std::shared_ptr<AbstractMacro> macro;
        TU_ASSIGN_OR_RETURN (macro, m_registry->makeMacro(id));

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
        lyric_parser::ArchetypeNode *macroListNode;
        TU_ASSIGN_OR_RETURN (macroListNode, parentNode->removeChild(ctx.childIndex()));
        MacroBlock macroBlock(parentNode, ctx.childIndex(), state);
        TU_RETURN_IF_NOT_OK (rewriteMacroBlock(macroListNode, macroBlock));
    }

    return {};
}

lyric_rewriter::MacroRewriteDriverBuilder::MacroRewriteDriverBuilder(std::shared_ptr<MacroRegistry> registry)
    : m_registry(std::move(registry))
{
    TU_ASSERT (m_registry != nullptr);
}

tempo_utils::Status
lyric_rewriter::MacroRewriteDriverBuilder::rewritePragma(
    lyric_parser::ArchetypeState *state,
    lyric_parser::ArchetypeNode *node,
    PragmaContext &ctx)
{
    lyric_schema::LyricAstId nodeId{};
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, nodeId));
    if (nodeId != lyric_schema::LyricAstId::Pragma)
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "expected Pragma");

    std::string id;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, id));

    std::shared_ptr<AbstractMacro> macro;
    TU_ASSIGN_OR_RETURN (macro, m_registry->makeMacro(id));

    return macro->rewritePragma(node, ctx, state);
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractRewriteDriver>>
lyric_rewriter::MacroRewriteDriverBuilder::makeRewriteDriver()
{
    auto driver = std::make_shared<MacroRewriteDriver>(m_registry);
    return std::static_pointer_cast<AbstractRewriteDriver>(driver);
}
