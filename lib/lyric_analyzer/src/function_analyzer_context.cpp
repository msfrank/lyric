
#include <lyric_analyzer/function_analyzer_context.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::FunctionAnalyzerContext::FunctionAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::CallSymbol *callSymbol)
    : m_driver(driver),
      m_call(callSymbol)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_call != nullptr);
}

lyric_assembler::BlockHandle *
lyric_analyzer::FunctionAnalyzerContext::getBlock() const
{
    return m_call->callProc()->procBlock();
}

tempo_utils::Status
lyric_analyzer::FunctionAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    return {};
}

tempo_utils::Status
lyric_analyzer::FunctionAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    if (resource->getId() == lyric_schema::LyricAstId::Def) {
        return m_driver->popContext();
    }

    return {};
}