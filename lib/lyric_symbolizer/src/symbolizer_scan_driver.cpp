
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_symbolizer/symbolizer_scan_driver.h>
#include <lyric_symbolizer/symbolizer_result.h>

lyric_symbolizer::SymbolizerScanDriver::SymbolizerScanDriver(lyric_assembler::ObjectState *state)
    : m_state(state),
      m_entry(nullptr)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::arrange(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children)
{
    children.clear();
    for (int i = node->numChildren() - 1; i >= 0; i--) {
        children.emplace_back(node->getChild(i), i);
    }
    return {};
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Decl:
            return pushDefinition(node, lyric_object::LinkageSection::Action);
        case lyric_schema::LyricAstId::Def:
            return pushDefinition(node, lyric_object::LinkageSection::Call);
        case lyric_schema::LyricAstId::DefClass:
            return pushDefinition(node, lyric_object::LinkageSection::Class);
        case lyric_schema::LyricAstId::DefConcept:
            return pushDefinition(node, lyric_object::LinkageSection::Concept);
        case lyric_schema::LyricAstId::DefEnum:
            return pushDefinition(node, lyric_object::LinkageSection::Enum);
        case lyric_schema::LyricAstId::DefInstance:
            return pushDefinition(node, lyric_object::LinkageSection::Instance);
        case lyric_schema::LyricAstId::DefStruct:
            return pushDefinition(node, lyric_object::LinkageSection::Struct);
        case lyric_schema::LyricAstId::Namespace:
            return pushDefinition(node, lyric_object::LinkageSection::Namespace);
        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var:
            return declareStatic(node);
        case lyric_schema::LyricAstId::Decl:
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::DefConcept:
        case lyric_schema::LyricAstId::DefEnum:
        case lyric_schema::LyricAstId::DefInstance:
        case lyric_schema::LyricAstId::DefStruct:
        case lyric_schema::LyricAstId::Namespace:
            return popDefinition();
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:
            return declareImport(node);
        default:
            break;
    }
    return {};
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::declareStatic(const lyric_parser::ArchetypeNode *node)
{
    if (!m_symbolPath.empty())
        return {};

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_common::SymbolPath symbolPath({identifier});
    lyric_common::SymbolUrl symbolUrl(symbolPath);
    auto *undecl = new lyric_assembler::UndeclaredSymbol(symbolUrl, lyric_object::LinkageSection::Static);

    auto status = m_state->appendUndeclared(undecl);
    if (status.notOk()) {
        delete undecl;
        return status;
    }
    TU_LOG_INFO << "declared static " << symbolUrl;

    if (m_entry == nullptr) {
        lyric_common::SymbolPath entryPath({"$entry"});
        lyric_common::SymbolUrl entryUrl(entryPath);
        m_entry = new lyric_assembler::UndeclaredSymbol(entryUrl, lyric_object::LinkageSection::Call);
        status = m_state->appendUndeclared(m_entry);
        if (status.notOk()) {
            delete m_entry;
            return status;
        }
        TU_LOG_INFO << "declared entry " << entryUrl;
    }

    return {};
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::pushDefinition(
    const lyric_parser::ArchetypeNode *node,
    lyric_object::LinkageSection section)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    m_symbolPath.push_back(identifier);

    lyric_common::SymbolPath symbolPath(m_symbolPath);
    lyric_common::SymbolUrl symbolUrl(symbolPath);
    auto *undecl = new lyric_assembler::UndeclaredSymbol(symbolUrl, section);

    auto status = m_state->appendUndeclared(undecl);
    if (status.notOk()) {
        delete undecl;
    }

    TU_LOG_INFO << "declared definition " << symbolUrl;
    return status;
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::popDefinition()
{
    m_symbolPath.pop_back();
    return {};
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::declareImport(const lyric_parser::ArchetypeNode *node)
{
    lyric_common::ModuleLocation importLocation;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstModuleLocation, importLocation));

    auto *importCache = m_state->importCache();
    if (importCache->hasImport(importLocation))
        return {};

    TU_RETURN_IF_NOT_OK (importCache->insertImport(importLocation, lyric_assembler::ImportFlags::ApiLinkage));

    TU_LOG_INFO << "imported module " << importLocation;
    return {};
}
