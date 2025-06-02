
#include <lyric_analyzer/entry_analyzer_context.h>
#include <lyric_analyzer/namespace_analyzer_context.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::EntryAnalyzerContext::EntryAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::ObjectRoot *root)
    : m_driver(driver),
      m_root(root)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_root != nullptr);
}

lyric_assembler::BlockHandle *
lyric_analyzer::EntryAnalyzerContext::getBlock() const
{
    return m_root->rootBlock();
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
        case lyric_schema::LyricAstId::TypeName:
            return m_driver->declareTypename(node, getBlock());
        case lyric_schema::LyricAstId::DefAlias:
            return m_driver->declareBinding(node, getBlock());
        case lyric_schema::LyricAstId::Def:
            return m_driver->pushFunction(node, getBlock());
        case lyric_schema::LyricAstId::DefClass:
            return m_driver->pushClass(node, getBlock());
        case lyric_schema::LyricAstId::DefConcept:
            return m_driver->pushConcept(node, getBlock());
        case lyric_schema::LyricAstId::DefEnum:
            return m_driver->pushEnum(node, getBlock());
        case lyric_schema::LyricAstId::DefInstance:
            return m_driver->pushInstance(node, getBlock());
        case lyric_schema::LyricAstId::DefStruct:
            return m_driver->pushStruct(node, getBlock());
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
        case lyric_schema::LyricAstId::DefStatic:
            return m_driver->declareStatic(node, getBlock());
        default:
            break;
    }
    return {};
}