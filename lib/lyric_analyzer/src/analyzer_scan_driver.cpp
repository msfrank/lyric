
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_analyzer/analyzer_scan_driver.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::AnalyzerScanDriver::AnalyzerScanDriver(
    const lyric_common::AssemblyLocation &location,
    lyric_assembler::AssemblyState *state)
    : m_location(location),
      m_state(state),
      m_root(nullptr),
      m_entry(nullptr),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_analyzer::AnalyzerScanDriver::~AnalyzerScanDriver()
{
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::initialize()
{
    if (m_typeSystem != nullptr)
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant,"module entry is already initialized");
    m_typeSystem = new lyric_typing::TypeSystem(m_state);

    auto *fundamentalCache = m_state->fundamentalCache();
    auto *symbolCache = m_state->symbolCache();
    auto *typeCache = m_state->typeCache();

    // lookup the Function0 class and get its type handle
    auto functionClassUrl = fundamentalCache->getFunctionUrl(0);
    if (!functionClassUrl.isValid())
        m_state->throwAssemblerInvariant("missing Function0 symbol");
    symbolCache->touchSymbol(functionClassUrl);

    tempo_utils::Status status;

    // ensure that NoReturn is in the type cache
    auto returnType = lyric_common::TypeDef::noReturn();
    lyric_assembler::TypeHandle *returnTypeHandle;
    TU_ASSIGN_OR_RETURN (returnTypeHandle, typeCache->getOrMakeType(returnType));
    returnTypeHandle->touch();

    lyric_assembler::TypeHandle *entryTypeHandle;
    TU_ASSIGN_OR_RETURN (entryTypeHandle, typeCache->declareFunctionType(returnType, {}, {}));
    entryTypeHandle->touch();

    // create the $entry call
    lyric_common::SymbolUrl entryUrl(m_location, lyric_common::SymbolPath({"$entry"}));
    auto entryAddress = lyric_assembler::CallAddress::near(m_state->numCalls());
    m_entry = new lyric_assembler::CallSymbol(entryUrl, returnType, entryAddress, entryTypeHandle, m_state);
    status = m_state->appendCall(m_entry);
    if (status.notOk()) {
        delete m_entry;
        return status;
    }
    m_entry->touch();

    // resolve the Namespace type
    auto namespaceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Namespace);
    lyric_assembler::TypeHandle *namespaceTypeHandle;
    TU_ASSIGN_OR_RETURN (namespaceTypeHandle, typeCache->getOrMakeType(namespaceType));

    // create the root namespace
    lyric_common::SymbolUrl rootUrl(m_location, lyric_common::SymbolPath({"$root"}));
    auto nsAddress = lyric_assembler::NamespaceAddress::near(m_state->numNamespaces());
    m_root = new lyric_assembler::NamespaceSymbol(rootUrl, nsAddress, namespaceTypeHandle,
        m_entry->callProc(), m_state);
    status = m_state->appendNamespace(m_root);
    if (status.notOk()) {
        delete m_root;
        return status;
    }

    // push the namespace block
    m_blocks.push_back(m_root->namespaceBlock());

    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::enter(
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
lyric_analyzer::AnalyzerScanDriver::exit(
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
            return declareStatic(node, lyric_parser::BindingType::VALUE);
        case lyric_schema::LyricAstId::Var:
            return declareStatic(node, lyric_parser::BindingType::VARIABLE);
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::DefConcept:
        case lyric_schema::LyricAstId::DefEnum:
        case lyric_schema::LyricAstId::DefInstance:
        case lyric_schema::LyricAstId::DefStruct:
        case lyric_schema::LyricAstId::Namespace:
            return popDefinition();
        default:
            break;
    }
    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::declareStatic(
    const lyric_parser::ArchetypeNode *node,
    lyric_parser::BindingType binding)
{
    if (m_blocks.empty())
        return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant);

    auto *block = m_blocks.back();
    if (!block->isRoot())
        return {};

    auto walker = node->getArchetypeNode();

    std::string identifier;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    lyric_parser::NodeWalker typeNode;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec staticSpec;
    TU_ASSIGN_OR_RETURN (staticSpec, m_typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef staticType;
    TU_ASSIGN_OR_RETURN (staticType, m_typeSystem->resolveAssignable(block, staticSpec));

    TU_RETURN_IF_STATUS (block->declareStatic(identifier, staticType, binding, true));
    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushDefinition(
    const lyric_parser::ArchetypeNode *node,
    lyric_object::LinkageSection section)
{
//    std::string identifier;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//    m_symbolPath.push_back(identifier);
//
//    lyric_common::SymbolPath symbolPath(m_symbolPath);
//    lyric_common::SymbolUrl symbolUrl(symbolPath);
//    auto *undecl = new lyric_assembler::UndeclaredSymbol(symbolUrl, section);
//
//    auto status = m_state->appendUndeclared(undecl);
//    if (status.notOk()) {
//        delete undecl;
//    }
//
//    TU_LOG_INFO << "declared definition " << symbolUrl;
//    return status;
    TU_UNREACHABLE();
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::popDefinition()
{
//    m_symbolPath.pop_back();
//    return {};
    TU_UNREACHABLE();
}