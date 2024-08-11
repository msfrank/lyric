
#include <lyric_analyzer/entry_analyzer_context.h>
#include <lyric_analyzer/function_analyzer_context.h>
#include <lyric_analyzer/namespace_analyzer_context.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::EntryAnalyzerContext::EntryAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::CallSymbol *entry,
    lyric_assembler::NamespaceSymbol *root)
    : m_driver(driver),
      m_entry(entry),
      m_root(root)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_entry != nullptr);
    TU_ASSERT (m_root != nullptr);
}

lyric_assembler::BlockHandle *
lyric_analyzer::EntryAnalyzerContext::getBlock() const
{
    return m_root->namespaceBlock();
}

tempo_utils::Status
lyric_analyzer::EntryAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Def:
            return m_driver->pushFunction(node, getBlock());
        case lyric_schema::LyricAstId::DefClass:
            return m_driver->pushClass(node, getBlock());
        case lyric_schema::LyricAstId::DefConcept:
            return m_driver->pushConcept(node, getBlock());
//        case lyric_schema::LyricAstId::DefEnum:
//        case lyric_schema::LyricAstId::DefInstance:
//        case lyric_schema::LyricAstId::DefStruct:
        case lyric_schema::LyricAstId::Namespace:
            return m_driver->pushNamespace(node, getBlock());
        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_analyzer::EntryAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Block:
            m_driver->popContext();
            break;
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var:
            return m_driver->declareStatic(node, getBlock());
        default:
            break;
    }
    return {};
}