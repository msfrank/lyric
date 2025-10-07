
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_analyzer/analyzer_scan_driver.h>
#include <lyric_analyzer/class_analyzer_context.h>
#include <lyric_analyzer/concept_analyzer_context.h>
#include <lyric_analyzer/entry_analyzer_context.h>
#include <lyric_analyzer/enum_analyzer_context.h>
#include <lyric_analyzer/instance_analyzer_context.h>
#include <lyric_analyzer/internal/analyzer_utils.h>
#include <lyric_analyzer/namespace_analyzer_context.h>
#include <lyric_analyzer/proc_analyzer_context.h>
#include <lyric_analyzer/struct_analyzer_context.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/typename_symbol.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::AnalyzerScanDriver::AnalyzerScanDriver(
    lyric_assembler::ObjectRoot *root,
    lyric_assembler::ObjectState *state)
    : m_root(root),
      m_state(state),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_root != nullptr);
    TU_ASSERT (m_state != nullptr);
    m_namespaces.push(m_root->globalNamespace());
}

lyric_analyzer::AnalyzerScanDriver::~AnalyzerScanDriver()
{
    delete m_typeSystem;
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::initialize()
{
    if (m_typeSystem != nullptr)
        return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant,
            "analyzer scan driver is already initialized");
    m_typeSystem = new lyric_typing::TypeSystem(m_state);

    // push the entry context
    auto ctx = std::make_unique<EntryAnalyzerContext>(this, m_root);
    return pushContext(std::move(ctx));
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

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::finish()
{
    return {};
}

lyric_typing::TypeSystem *
lyric_analyzer::AnalyzerScanDriver::getTypeSystem() const
{
    return m_typeSystem;
}

lyric_analyzer::AbstractAnalyzerContext *
lyric_analyzer::AnalyzerScanDriver::peekContext()
{
    if (m_handlers.empty())
        return nullptr;
    auto &top = m_handlers.back();
    return top.get();
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushContext(std::unique_ptr<AbstractAnalyzerContext> ctx)
{
    m_handlers.push_back(std::move(ctx));
    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::popContext()
{
    if (m_handlers.empty())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant, "handler stack is empty");
    m_handlers.pop_back();
    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::declareTypename(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_assembler::TypenameSymbol *typenameSymbol;
    TU_ASSIGN_OR_RETURN (typenameSymbol, block->declareTypename(identifier, isHidden));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(typenameSymbol->getSymbolUrl()));

    TU_LOG_V << "declared typename " << typenameSymbol->getSymbolUrl();

    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::declareBinding(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode = nullptr;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    // parse the target spec
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec targetSpec;
    TU_ASSIGN_OR_RETURN (targetSpec, m_typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RETURN (bindingSymbol, block->declareBinding(identifier, isHidden, templateSpec.templateParameters));

    auto *resolver = bindingSymbol->bindingResolver();

    // define the target type
    lyric_common::TypeDef targetType;
    TU_ASSIGN_OR_RETURN (targetType, m_typeSystem->resolveAssignable(resolver, targetSpec));
    TU_RETURN_IF_NOT_OK (bindingSymbol->defineTarget(targetType));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(bindingSymbol->getSymbolUrl()));

    TU_LOG_V << "declared binding " << bindingSymbol->getSymbolUrl();

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

    bool isHidden;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    bool isVariable;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIsVariable, isVariable));

    std::string identifier;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    lyric_parser::NodeWalker typeNode;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec staticSpec;
    TU_ASSIGN_OR_RETURN (staticSpec, m_typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef staticType;
    TU_ASSIGN_OR_RETURN (staticType, m_typeSystem->resolveAssignable(block, staticSpec));

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->declareStatic(
        identifier, isHidden, staticType, isVariable, /* declOnly= */ true));

    // add class to the current namespace if specified
    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(ref.symbolUrl));

    TU_LOG_V << "declared static " << ref.symbolUrl;

    return {};
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushFunction(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

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
        identifier, isHidden, spec.templateParameters, /* declOnly= */ true));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(callSymbol->getSymbolUrl()));

    auto *resolver = callSymbol->callResolver();

    // determine the return type
    lyric_common::TypeDef returnType;
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *returnTypeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, returnTypeNode));
        lyric_typing::TypeSpec returnTypeSpec;
        TU_ASSIGN_OR_RETURN (returnTypeSpec, m_typeSystem->parseAssignable(block, returnTypeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (returnType, m_typeSystem->resolveAssignable(resolver, returnTypeSpec));
    }

    // determine the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, m_typeSystem->parsePack(block, packNode->getArchetypeNode()));
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, m_typeSystem->resolvePack(resolver, packSpec));

    // define the call
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, returnType));

    TU_LOG_V << "declared function " << callSymbol->getSymbolUrl();

    // push the function context
    auto ctx = std::make_unique<ProcAnalyzerContext>(this, procHandle);
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

    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode = nullptr;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    lyric_common::TypeDef superClassType;

    // determine the superclass type
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec superClassSpec;
        TU_ASSIGN_OR_RETURN (superClassSpec, m_typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (superClassType, m_typeSystem->resolveAssignable(block, superClassSpec));
    } else {
        auto *fundamentalCache = m_state->fundamentalCache();
        superClassType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
    }

    // resolve the superclass
    TU_ASSIGN_OR_RETURN (superClass, block->resolveClass(superClassType));

    lyric_assembler::ClassSymbol *classSymbol;
    TU_ASSIGN_OR_RETURN (classSymbol, block->declareClass(
        identifier, superClass, isHidden, templateSpec.templateParameters,
        internal::convert_derive_type(derive), /* declOnly= */ true));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(classSymbol->getSymbolUrl()));

    TU_LOG_V << "declared class " << classSymbol->getSymbolUrl() << " from " << superClass->getSymbolUrl();

    // push the class context
    auto ctx = std::make_unique<ClassAnalyzerContext>(this, classSymbol);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushConcept(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    lyric_typing::TemplateSpec templateSpec;
    lyric_assembler::ConceptSymbol *superConcept = nullptr;

    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode = nullptr;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    lyric_common::TypeDef superConceptType;

    // determine the superconcept type
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec superConceptSpec;
        TU_ASSIGN_OR_RETURN (superConceptSpec, m_typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (superConceptType, m_typeSystem->resolveAssignable(block, superConceptSpec));
    } else {
        auto *fundamentalCache = m_state->fundamentalCache();
        superConceptType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);
    }

    // resolve the superconcept
    TU_ASSIGN_OR_RETURN (superConcept, block->resolveConcept(superConceptType));

    lyric_assembler::ConceptSymbol *conceptSymbol;
    TU_ASSIGN_OR_RETURN (conceptSymbol, block->declareConcept(
        identifier, superConcept, isHidden, templateSpec.templateParameters,
        internal::convert_derive_type(derive), /* declOnly= */ true));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(conceptSymbol->getSymbolUrl()));

    TU_LOG_V << "declared concept " << conceptSymbol->getSymbolUrl() << " from " << superConcept->getSymbolUrl();

    // push the concept context
    auto ctx = std::make_unique<ConceptAnalyzerContext>(this, conceptSymbol);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushEnum(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    lyric_assembler::EnumSymbol *superEnum = nullptr;
    lyric_object::DeriveType derive = lyric_object::DeriveType::Sealed;

    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_common::TypeDef superEnumType;

    // determine the superenum type
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec superEnumSpec;
        TU_ASSIGN_OR_RETURN (superEnumSpec, m_typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (superEnumType, m_typeSystem->resolveAssignable(block, superEnumSpec));
    } else {
        auto *fundamentalCache = m_state->fundamentalCache();
        superEnumType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Category);
    }

    // resolve the superenum
    TU_ASSIGN_OR_RETURN (superEnum, block->resolveEnum(superEnumType));

    lyric_assembler::EnumSymbol *enumSymbol;
    TU_ASSIGN_OR_RETURN (enumSymbol, block->declareEnum(
        identifier, superEnum, isHidden, derive, /* declOnly= */ true));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(enumSymbol->getSymbolUrl()));

    TU_LOG_V << "declared enum " << enumSymbol->getSymbolUrl() << " from " << superEnum->getSymbolUrl();

    // push the enum context
    auto ctx = std::make_unique<EnumAnalyzerContext>(this, enumSymbol);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushInstance(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    lyric_assembler::InstanceSymbol *superInstance = nullptr;
    lyric_object::DeriveType derive = lyric_object::DeriveType::Any;

    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_common::TypeDef superInstanceType;

    // determine the superinstance type
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec superInstanceSpec;
        TU_ASSIGN_OR_RETURN (superInstanceSpec, m_typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (superInstanceType, m_typeSystem->resolveAssignable(block, superInstanceSpec));
    } else {
        auto *fundamentalCache = m_state->fundamentalCache();
        superInstanceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Singleton);
    }

    // resolve the superinstance
    TU_ASSIGN_OR_RETURN (superInstance, block->resolveInstance(superInstanceType));

    lyric_assembler::InstanceSymbol *instanceSymbol;
    TU_ASSIGN_OR_RETURN (instanceSymbol, block->declareInstance(
        identifier, superInstance, isHidden, derive, /* declOnly= */ true));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(instanceSymbol->getSymbolUrl()));

    TU_LOG_V << "declared instance " << instanceSymbol->getSymbolUrl() << " from " << superInstance->getSymbolUrl();

    // push the instance context
    auto ctx = std::make_unique<InstanceAnalyzerContext>(this, instanceSymbol);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushStruct(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    lyric_assembler::StructSymbol *superStruct = nullptr;

    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    lyric_common::TypeDef superStructType;

    // determine the superstruct type
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec superStructSpec;
        TU_ASSIGN_OR_RETURN (superStructSpec, m_typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (superStructType, m_typeSystem->resolveAssignable(block, superStructSpec));
    } else {
        auto *fundamentalCache = m_state->fundamentalCache();
        superStructType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
    }

    // resolve the superstruct
    TU_ASSIGN_OR_RETURN (superStruct, block->resolveStruct(superStructType));

    lyric_assembler::StructSymbol *structSymbol;
    TU_ASSIGN_OR_RETURN (structSymbol, block->declareStruct(
        identifier, superStruct, isHidden, internal::convert_derive_type(derive), /* declOnly= */ true));

    auto *currentNamespace = m_namespaces.top();
    TU_ASSERT (currentNamespace != nullptr);
    TU_RETURN_IF_NOT_OK (currentNamespace->putTarget(structSymbol->getSymbolUrl()));

    TU_LOG_V << "declared struct " << structSymbol->getSymbolUrl() << " from " << superStruct->getSymbolUrl();

    // push the struct context
    auto ctx = std::make_unique<StructAnalyzerContext>(this, structSymbol);
    return pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::AnalyzerScanDriver::pushNamespace(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    if (m_namespaces.empty())
        return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant,
            "namespace stack is empty");
    auto *currentNamespace = m_namespaces.top();

    lyric_assembler::NamespaceSymbol *namespaceSymbol;
    TU_ASSIGN_OR_RETURN (namespaceSymbol, currentNamespace->declareSubspace(identifier, isHidden));

    // push the namespace context
    auto ctx = std::make_unique<NamespaceAnalyzerContext>(this, namespaceSymbol);
    return pushContext(std::move(ctx));
}

lyric_analyzer::AnalyzerScanDriverBuilder::AnalyzerScanDriverBuilder(
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
lyric_analyzer::AnalyzerScanDriverBuilder::applyPragma(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node)
{
    return {};
}

tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractScanDriver>>
lyric_analyzer::AnalyzerScanDriverBuilder::makeScanDriver()
{
    // construct the object state
    m_state = std::make_unique<lyric_assembler::ObjectState>(m_location, m_origin,
        m_localModuleCache, m_systemModuleCache, m_shortcutResolver, m_objectStateOptions);

    // define the object root
    lyric_assembler::ObjectRoot *root;
    TU_ASSIGN_OR_RETURN (root, m_state->defineRoot());

    auto *entryCall = root->entryCall();
    auto *entryProc = entryCall->callProc();
    auto *entryFragment = entryProc->procFragment();

    // analyzer output is not meant to be executed
    TU_RETURN_IF_NOT_OK (entryFragment->invokeAbort());

    // initialize the driver
    auto driver = std::make_shared<AnalyzerScanDriver>(root, m_state.get());
    TU_RETURN_IF_NOT_OK (driver->initialize());

    return std::static_pointer_cast<lyric_rewriter::AbstractScanDriver>(driver);
}

tempo_utils::Result<lyric_object::LyricObject>
lyric_analyzer::AnalyzerScanDriverBuilder::toObject() const
{
    return m_state->toObject();
}
