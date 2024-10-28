
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_symbolizer/symbolizer_scan_driver.h>
#include <lyric_symbolizer/symbolizer_result.h>

lyric_symbolizer::SymbolizerScanDriver::SymbolizerScanDriver(
    lyric_assembler::ObjectRoot *root,
    lyric_assembler::ObjectState *state)
    : m_root(root),
      m_state(state)
{
    TU_ASSERT (m_root != nullptr);
    TU_ASSERT (m_state != nullptr);
    m_namespaces.push(m_root->globalNamespace());
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
            return pushNamespace(node);
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
        case lyric_schema::LyricAstId::DefStatic:
            return declareStatic(node);
        case lyric_schema::LyricAstId::Decl:
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::DefConcept:
        case lyric_schema::LyricAstId::DefEnum:
        case lyric_schema::LyricAstId::DefInstance:
        case lyric_schema::LyricAstId::DefStruct:
            return popDefinition();
        case lyric_schema::LyricAstId::Namespace:
            return popNamespace();
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
lyric_symbolizer::SymbolizerScanDriver::finish()
{
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
    auto undecl = std::make_unique<lyric_assembler::UndeclaredSymbol>(
        symbolUrl, lyric_object::LinkageSection::Static);

    TU_RETURN_IF_NOT_OK (m_state->appendUndeclared(undecl.get()));
    undecl.release();
    TU_LOG_INFO << "declared static " << symbolUrl;

    return putNamespaceBinding(identifier, symbolUrl, lyric_object::AccessType::Public);
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
    auto undecl = std::make_unique<lyric_assembler::UndeclaredSymbol>(symbolUrl, section);

    TU_RETURN_IF_NOT_OK (m_state->appendUndeclared(undecl.get()));
    undecl.release();
    TU_LOG_INFO << "declared definition " << symbolUrl;

    return putNamespaceBinding(identifier, symbolUrl, lyric_object::AccessType::Public);
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

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::pushNamespace(const lyric_parser::ArchetypeNode *node)
{
    if (m_namespaces.empty())
        return SymbolizerStatus::forCondition(SymbolizerCondition::kSymbolizerInvariant,
            "namespace stack is empty");
    auto *currentNamespace = m_namespaces.top();

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    m_symbolPath.push_back(identifier);

    lyric_assembler::NamespaceSymbol *subspace;
    TU_ASSIGN_OR_RETURN (subspace, currentNamespace->declareSubspace(identifier, lyric_object::AccessType::Public));
    m_namespaces.push(subspace);

    TU_LOG_INFO << "declared namespace " << subspace->getSymbolUrl().toString();

    return {};
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::putNamespaceBinding(
    const std::string &name,
    const lyric_common::SymbolUrl &symbolUrl,
    lyric_object::AccessType access)
{
    if (m_namespaces.empty())
        return SymbolizerStatus::forCondition(SymbolizerCondition::kSymbolizerInvariant,
            "namespace stack is empty");
    auto *currentNamespace = m_namespaces.top();
    return currentNamespace->putBinding(name, symbolUrl, access);
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriver::popNamespace()
{
    m_namespaces.pop();
    if (m_namespaces.empty())
        return SymbolizerStatus::forCondition(SymbolizerCondition::kSymbolizerInvariant,
            "namespace stack is empty");
    return {};
}
