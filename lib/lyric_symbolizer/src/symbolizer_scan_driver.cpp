
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/linkage_symbol.h>
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
        case lyric_schema::LyricAstId::DefAlias:
            return pushDefinition(node, lyric_object::LinkageSection::Binding);
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
        case lyric_schema::LyricAstId::TypeName:
            return declareTypename(node);
        case lyric_schema::LyricAstId::DefStatic:
            return declareStatic(node);
        case lyric_schema::LyricAstId::Decl:
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::DefAlias:
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
lyric_symbolizer::SymbolizerScanDriver::declareTypename(const lyric_parser::ArchetypeNode *node)
{
    if (!m_symbolPath.empty())
        return {};

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    auto *symbolCache = m_state->symbolCache();

    lyric_common::SymbolPath symbolPath({identifier});
    lyric_common::SymbolUrl symbolUrl(symbolPath);
    TU_RETURN_IF_STATUS (symbolCache->putTypename(symbolUrl));

    TU_LOG_INFO << "declared typename " << symbolUrl;

    return putNamespaceTarget(symbolUrl);
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
    auto linkage = std::make_unique<lyric_assembler::LinkageSymbol>(
        symbolUrl, lyric_object::LinkageSection::Static);

    TU_RETURN_IF_STATUS (m_state->appendLinkage(std::move(linkage)));
    TU_LOG_INFO << "declared static " << symbolUrl;

    return putNamespaceTarget(symbolUrl);
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
    auto linkage = std::make_unique<lyric_assembler::LinkageSymbol>(symbolUrl, section);

    TU_RETURN_IF_STATUS (m_state->appendLinkage(std::move(linkage)));
    TU_LOG_INFO << "declared definition " << symbolUrl;

    auto *currentNamespace = m_namespaces.top();
    auto namespacePath = currentNamespace->getSymbolUrl().getSymbolPath();
    if (symbolPath.getEnclosure() == namespacePath.getPath()) {
        TU_RETURN_IF_NOT_OK (putNamespaceTarget(symbolUrl));
    }

    return {};
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
    tempo_utils::Url importLocation;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstImportLocation, importLocation));

    auto *importCache = m_state->importCache();

    lyric_common::ModuleLocation moduleLocation;
    TU_ASSIGN_OR_RETURN (moduleLocation, importCache->resolveImportLocation(importLocation));
    if (importCache->hasImport(moduleLocation))
        return {};

    TU_RETURN_IF_NOT_OK (importCache->insertImport(moduleLocation, lyric_assembler::ImportFlags::ApiLinkage));

    TU_LOG_INFO << "imported module " << moduleLocation;
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
lyric_symbolizer::SymbolizerScanDriver::putNamespaceTarget(const lyric_common::SymbolUrl &symbolUrl)
{
    if (m_namespaces.empty())
        return SymbolizerStatus::forCondition(SymbolizerCondition::kSymbolizerInvariant,
            "namespace stack is empty");
    auto *currentNamespace = m_namespaces.top();
    return currentNamespace->putTarget(symbolUrl);
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

lyric_symbolizer::SymbolizerScanDriverBuilder::SymbolizerScanDriverBuilder(
    const lyric_common::ModuleLocation &location,
    const lyric_common::ModuleLocation &origin,
    std::shared_ptr<lyric_importer::ModuleCache> localModuleCache,
    std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
    std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver,
    const lyric_assembler::ObjectStateOptions &objectStateOptions)
    : m_location(location),
      m_origin(origin),
      m_localModuleCache(std::move(localModuleCache)),
      m_systemModuleCache(std::move(systemModuleCache)),
      m_shortcutResolver(std::move(shortcutResolver)),
      m_objectStateOptions(objectStateOptions)
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_origin.isValid());
}

tempo_utils::Status
lyric_symbolizer::SymbolizerScanDriverBuilder::applyPragma(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node)
{
    return {};
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractScanDriver>>
lyric_symbolizer::SymbolizerScanDriverBuilder::makeScanDriver()
{
    // construct the object state
    m_state = std::make_unique<lyric_assembler::ObjectState>(m_location, m_origin,
        m_localModuleCache, m_systemModuleCache, m_shortcutResolver, m_objectStateOptions);

    // define the object root
    lyric_assembler::ObjectRoot *root;
    TU_ASSIGN_OR_RETURN (root, m_state->defineRoot());

    auto driver = std::make_shared<SymbolizerScanDriver>(root, m_state.get());

    return std::static_pointer_cast<lyric_rewriter::AbstractScanDriver>(driver);
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_symbolizer::SymbolizerScanDriverBuilder::toObject() const
{
    return m_state->toObject();
}