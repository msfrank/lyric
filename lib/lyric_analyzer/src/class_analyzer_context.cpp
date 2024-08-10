
#include <lyric_analyzer/class_analyzer_context.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::ClassAnalyzerContext::ClassAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::ClassSymbol *classSymbol,
    const lyric_parser::ArchetypeNode *initNode)
    : m_driver(driver),
      m_classSymbol(classSymbol),
      m_initNode(initNode)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_classSymbol != nullptr);
}

lyric_assembler::BlockHandle *
lyric_analyzer::ClassAnalyzerContext::getBlock() const
{
    return m_classSymbol->classBlock();
}

tempo_utils::Status
lyric_analyzer::ClassAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    return {};
}

tempo_utils::Status
lyric_analyzer::ClassAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    if (resource->getId() == lyric_schema::LyricAstId::DefClass) {
        // define the constructor
        if (m_initNode != nullptr) {
            TU_UNREACHABLE();
        } else {
            lyric_assembler::CallSymbol *ctorSymbol;
            TU_ASSIGN_OR_RETURN (ctorSymbol, m_classSymbol->declareCtor(lyric_object::AccessType::Public));
            TU_RETURN_IF_STATUS (ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));
        }
        return m_driver->popContext();
    }

    return {};
}