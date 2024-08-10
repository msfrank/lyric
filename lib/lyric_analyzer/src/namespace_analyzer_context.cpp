
#include <lyric_analyzer/namespace_analyzer_context.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::NamespaceAnalyzerContext::NamespaceAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::NamespaceSymbol *namespaceSymbol)
    : m_driver(driver),
      m_namespace(namespaceSymbol)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_namespace != nullptr);
}

lyric_assembler::BlockHandle *
lyric_analyzer::NamespaceAnalyzerContext::getBlock() const
{
    return m_namespace->namespaceBlock();
}

tempo_utils::Status
lyric_analyzer::NamespaceAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    return {};
}

tempo_utils::Status
lyric_analyzer::NamespaceAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    if (resource->getId() == lyric_schema::LyricAstId::Namespace) {
        return m_driver->popContext();
    }

    return {};
}
