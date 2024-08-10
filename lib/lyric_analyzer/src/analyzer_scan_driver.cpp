
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_analyzer/analyzer_scan_driver.h>
#include <lyric_analyzer/class_analyzer_context.h>
#include <lyric_analyzer/entry_analyzer_context.h>
#include <lyric_analyzer/function_analyzer_context.h>
#include <lyric_analyzer/namespace_analyzer_context.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::AnalyzerScanDriver::AnalyzerScanDriver(lyric_assembler::AssemblyState *state)
    : m_state(state),
      m_root(nullptr),
      m_entry(nullptr),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_analyzer::AnalyzerScanDriver::~AnalyzerScanDriver()
{
    delete m_typeSystem;
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

    auto location = m_state->getLocation();

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
    lyric_common::SymbolUrl entryUrl(location, lyric_common::SymbolPath({"$entry"}));
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
    lyric_common::SymbolUrl rootUrl(location, lyric_common::SymbolPath({"$root"}));
    auto nsAddress = lyric_assembler::NamespaceAddress::near(m_state->numNamespaces());
    m_root = new lyric_assembler::NamespaceSymbol(rootUrl, nsAddress, namespaceTypeHandle,
        m_entry->callProc(), m_state);
    status = m_state->appendNamespace(m_root);
    if (status.notOk()) {
        delete m_root;
        return status;
    }

    // push the entry context
    auto ctx = std::make_unique<EntryAnalyzerContext>(this, m_entry, m_root);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::arrange(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children)
{
    return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant);
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    auto *analyzerContext = peekContext();
    return analyzerContext->enter(state, node, ctx);
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    auto *analyzerContext = peekContext();
    return analyzerContext->exit(state, node, ctx);
}

lyric_typing::TypeSystem *
lyric_analyzer::AnalyzerScanDriver::getTypeSystem() const
{
    return m_typeSystem;
}

lyric_analyzer::AbstractAnalyzerContext *
lyric_analyzer::AnalyzerScanDriver::peekContext()
{
    if (m_stack.empty())
        return nullptr;
    auto &top = m_stack.back();
    return top.get();
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushContext(std::unique_ptr<AbstractAnalyzerContext> ctx)
{
    m_stack.push_back(std::move(ctx));
    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::popContext()
{
    if (m_stack.empty())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant, "context stack is empty");
    m_stack.pop_back();
    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::declareStatic(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    auto walker = node->getArchetypeNode();

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (walker.parseId(lyric_schema::kLyricAstVocabulary, astId));

    lyric_parser::BindingType binding;
    switch (astId) {
        case lyric_schema::LyricAstId::Val:
            binding = lyric_parser::BindingType::VALUE;
            break;
        case lyric_schema::LyricAstId::Var:
            binding = lyric_parser::BindingType::VARIABLE;
            break;
        default:
            return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant);
    }

    std::string identifier;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    lyric_parser::NodeWalker typeNode;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec staticSpec;
    TU_ASSIGN_OR_RETURN (staticSpec, m_typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef staticType;
    TU_ASSIGN_OR_RETURN (staticType, m_typeSystem->resolveAssignable(block, staticSpec));

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->declareStatic(identifier, staticType, binding, true));

    TU_LOG_INFO << "declared static " << ref.symbolUrl;

    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushFunction(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_parser::ArchetypeNode *genericNode = nullptr;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
    }

    lyric_typing::TemplateSpec spec;
    if (genericNode != nullptr) {
        TU_ASSIGN_OR_RETURN (spec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
        identifier, lyric_object::AccessType::Public, spec.templateParameters, /* declOnly= */ true));

    auto *resolver = callSymbol->callResolver();

    // determine the return type
    lyric_parser::ArchetypeNode *returnTypeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, returnTypeNode));
    lyric_typing::TypeSpec returnTypeSpec;
    TU_ASSIGN_OR_RETURN (returnTypeSpec, m_typeSystem->parseAssignable(block, returnTypeNode->getArchetypeNode()));
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_typeSystem->resolveAssignable(resolver, returnTypeSpec));

    // determine the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, m_typeSystem->parsePack(block, packNode->getArchetypeNode()));
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, m_typeSystem->resolvePack(resolver, packSpec));

    // define the call
    TU_RETURN_IF_STATUS (callSymbol->defineCall(parameterPack, returnType));

    TU_LOG_INFO << "declared function " << callSymbol->getSymbolUrl();

    // push the function context
    auto ctx = std::make_unique<FunctionAnalyzerContext>(this, callSymbol);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushClass(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    lyric_typing::TemplateSpec templateSpec;
    lyric_assembler::ClassSymbol *superClass = nullptr;
    lyric_object::DeriveType derive = lyric_object::DeriveType::Any;
    bool isAbstract = false;

    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode = nullptr;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    lyric_parser::ArchetypeNode *initNode = nullptr;
    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;
        lyric_schema::LyricAstId childId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
        if (childId == lyric_schema::LyricAstId::Init) {
            if (initNode != nullptr)
                return AnalyzerStatus::forCondition(AnalyzerCondition::kSyntaxError);
            initNode = child;
        }
    }

    // determine the superclass type
    lyric_common::TypeDef superClassType;
    if (initNode != nullptr) {
        if (initNode->numChildren() < 1)
            return AnalyzerStatus::forCondition(AnalyzerCondition::kSyntaxError);
        if (initNode->numChildren() > 1) {
            auto *superNode = initNode->getChild(1);
            if (!superNode->isClass(lyric_schema::kLyricAstSuperClass))
                return AnalyzerStatus::forCondition(AnalyzerCondition::kSyntaxError);
            lyric_parser::ArchetypeNode *superTypeNode;
            TU_RETURN_IF_NOT_OK (superNode->parseAttr(lyric_parser::kLyricAstTypeOffset, superTypeNode));
            lyric_typing::TypeSpec superTypeSpec;
            TU_ASSIGN_OR_RETURN (superTypeSpec, m_typeSystem->parseAssignable(block, superTypeNode->getArchetypeNode()));
            TU_ASSIGN_OR_RETURN (superClassType, m_typeSystem->resolveAssignable(block, superTypeSpec));
        }
    } else {
        auto *fundamentalCache = m_state->fundamentalCache();
        superClassType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    }

    // resolve the superclass
    TU_ASSIGN_OR_RETURN (superClass, block->resolveClass(superClassType));

    lyric_assembler::ClassSymbol *classSymbol;
    TU_ASSIGN_OR_RETURN (classSymbol, block->declareClass(
        identifier, superClass, lyric_object::AccessType::Public, templateSpec.templateParameters,
        derive, isAbstract, /* declOnly= */ true));

    TU_LOG_INFO << "declared class " << classSymbol->getSymbolUrl() << " from " << superClass->getSymbolUrl();

    // push the class context
    auto ctx = std::make_unique<ClassAnalyzerContext>(this, classSymbol, initNode);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushNamespace(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_assembler::NamespaceSymbol *namespaceSymbol;
    TU_ASSIGN_OR_RETURN (namespaceSymbol, block->declareNamespace(
        identifier, lyric_object::AccessType::Public, /* declOnly= */ true));

    // push the namespace context
    auto ctx = std::make_unique<NamespaceAnalyzerContext>(this, namespaceSymbol);
    return pushContext(std::move(ctx));
}
