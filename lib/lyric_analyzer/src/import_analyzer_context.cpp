
#include <lyric_analyzer/import_analyzer_context.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::ImportAnalyzerContext::ImportAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::BlockHandle *importBlock,
    const tempo_utils::Url &importLocation)
    : m_driver(driver),
      m_importBlock(importBlock),
      m_importLocation(importLocation)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_importBlock != nullptr);
    TU_ASSERT (m_importLocation.isValid());
}

lyric_assembler::BlockHandle *
lyric_analyzer::ImportAnalyzerContext::getBlock() const
{
    return m_importBlock;
}

tempo_utils::Status
lyric_analyzer::ImportAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    switch (resource->getId()) {
        case lyric_schema::LyricAstId::SymbolRef:
            return addSymbol(node);
        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_analyzer::ImportAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    switch (resource->getId()) {
        case lyric_schema::LyricAstId::ImportAll:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
            break;
        default:
            return {};
    }

    auto *objectState = m_importBlock->blockState();
    auto *importCache = objectState->importCache();

    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, importCache->resolveImportLocation(m_importLocation));
    TU_RETURN_IF_NOT_OK (importCache->importModule(moduleLocation, m_importBlock, m_importRefs));

    return m_driver->popContext();
}

tempo_utils::Status
lyric_analyzer::ImportAnalyzerContext::addSymbol(const lyric_parser::ArchetypeNode *node)
{
    lyric_common::SymbolPath symbolPath;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
    m_importRefs.insert(lyric_assembler::ImportRef(symbolPath));
    return {};
}
