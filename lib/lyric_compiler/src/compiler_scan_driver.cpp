
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/undeclared_symbol.h>
#include <lyric_compiler/compiler_scan_driver.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/block_compiler_context.h>
#include <lyric_compiler/entry_compiler_context.h>
#include <lyric_compiler/internal/compiler_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::CompilerScanDriver::CompilerScanDriver(lyric_assembler::ObjectState *state)
    : m_state(state),
      m_root(nullptr),
      m_entry(nullptr),
      m_typeSystem(nullptr)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_compiler::CompilerScanDriver::~CompilerScanDriver()
{
    delete m_typeSystem;
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::initialize()
{
    if (m_typeSystem != nullptr)
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant,"module entry is already initialized");
    m_typeSystem = new lyric_typing::TypeSystem(m_state);

    auto *fundamentalCache = m_state->fundamentalCache();
    auto *typeCache = m_state->typeCache();

    auto location = m_state->getLocation();

    // lookup the Function0 class and get its type handle
    auto functionClassUrl = fundamentalCache->getFunctionUrl(0);
    if (!functionClassUrl.isValid())
        m_state->throwAssemblerInvariant("missing Function0 symbol");

    tempo_utils::Status status;

    // ensure that NoReturn is in the type cache
    auto returnType = lyric_common::TypeDef::noReturn();
    TU_RETURN_IF_STATUS (typeCache->getOrMakeType(returnType));

    lyric_assembler::TypeHandle *entryTypeHandle;
    TU_ASSIGN_OR_RETURN (entryTypeHandle, typeCache->declareFunctionType(returnType, {}, {}));

    // create the $entry call
    lyric_common::SymbolUrl entryUrl(location, lyric_common::SymbolPath({"$entry"}));
    auto entryAddress = lyric_assembler::CallAddress::near(m_state->numCalls());
    m_entry = new lyric_assembler::CallSymbol(entryUrl, returnType, entryAddress, entryTypeHandle, m_state);
    status = m_state->appendCall(m_entry);
    if (status.notOk()) {
        delete m_entry;
        return status;
    }

    // resolve the Namespace type
    auto namespaceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Namespace);
    lyric_assembler::TypeHandle *namespaceTypeHandle;
    TU_ASSIGN_OR_RETURN (namespaceTypeHandle, typeCache->getOrMakeType(namespaceType));

    // create the root namespace
    auto *entryProc = m_entry->callProc();
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
    auto entry = std::make_unique<EntryCompilerContext>(this, m_entry, m_root);
    TU_RETURN_IF_NOT_OK (pushContext(std::move(entry)));

    // push the block context
    auto *entryBlock = entryProc->procBlock();
    auto block = std::make_unique<BlockCompilerContext>(this, entryBlock);
    return pushContext(std::move(block));
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::arrange(
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
lyric_compiler::CompilerScanDriver::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    auto *analyzerContext = peekContext();
    return analyzerContext->enter(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    auto *analyzerContext = peekContext();
    return analyzerContext->exit(state, node, ctx);
}

lyric_typing::TypeSystem *
lyric_compiler::CompilerScanDriver::getTypeSystem() const
{
    return m_typeSystem;
}

lyric_compiler::AbstractCompilerContext *
lyric_compiler::CompilerScanDriver::peekContext()
{
    if (m_contexts.empty())
        return nullptr;
    auto &top = m_contexts.back();
    return top.get();
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::pushContext(std::unique_ptr<AbstractCompilerContext> ctx)
{
    m_contexts.push_back(std::move(ctx));
    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::popContext()
{
    if (m_contexts.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "context stack is empty");
    m_contexts.pop_back();
    return {};
}

lyric_common::TypeDef
lyric_compiler::CompilerScanDriver::peekResult()
{
    if (!m_results.empty())
        return m_results.top();
    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::pushResult(const lyric_common::TypeDef &result)
{
    m_results.push(result);
    return {};
}

tempo_utils::Status
lyric_compiler::CompilerScanDriver::popResult()
{
    if (m_results.empty())
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "results stack is empty");
    m_results.pop();
    return {};
}

//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::declareStatic(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    auto walker = node->getArchetypeNode();
//
//    lyric_schema::LyricAstId astId;
//    TU_RETURN_IF_NOT_OK (walker.parseId(lyric_schema::kLyricAstVocabulary, astId));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    bool isVariable;
//    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIsVariable, isVariable));
//
//    std::string identifier;
//    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//    lyric_parser::NodeWalker typeNode;
//    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
//    lyric_typing::TypeSpec staticSpec;
//    TU_ASSIGN_OR_RETURN (staticSpec, m_typeSystem->parseAssignable(block, typeNode));
//    lyric_common::TypeDef staticType;
//    TU_ASSIGN_OR_RETURN (staticType, m_typeSystem->resolveAssignable(block, staticSpec));
//
//    lyric_assembler::DataReference ref;
//    TU_ASSIGN_OR_RETURN (ref, block->declareStatic(
//        identifier, internal::convert_access_type(access), staticType, isVariable, /* declOnly= */ true));
//
//    TU_LOG_INFO << "declared static " << ref.symbolUrl;
//
//    return {};
//}
//
//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::pushFunction(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    std::string identifier;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    lyric_parser::ArchetypeNode *genericNode = nullptr;
//    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
//        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
//    }
//
//    lyric_typing::TemplateSpec spec;
//    if (genericNode != nullptr) {
//        TU_ASSIGN_OR_RETURN (spec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
//    }
//
//    lyric_assembler::CallSymbol *callSymbol;
//    TU_ASSIGN_OR_RETURN (callSymbol, block->declareFunction(
//        identifier, internal::convert_access_type(access), spec.templateParameters, /* declOnly= */ true));
//
//    auto *resolver = callSymbol->callResolver();
//
//    // determine the return type
//    lyric_parser::ArchetypeNode *returnTypeNode;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, returnTypeNode));
//    lyric_typing::TypeSpec returnTypeSpec;
//    TU_ASSIGN_OR_RETURN (returnTypeSpec, m_typeSystem->parseAssignable(block, returnTypeNode->getArchetypeNode()));
//    lyric_common::TypeDef returnType;
//    TU_ASSIGN_OR_RETURN (returnType, m_typeSystem->resolveAssignable(resolver, returnTypeSpec));
//
//    // determine the parameter list
//    auto *packNode = node->getChild(0);
//    lyric_typing::PackSpec packSpec;
//    TU_ASSIGN_OR_RETURN (packSpec, m_typeSystem->parsePack(block, packNode->getArchetypeNode()));
//    lyric_assembler::ParameterPack parameterPack;
//    TU_ASSIGN_OR_RETURN (parameterPack, m_typeSystem->resolvePack(resolver, packSpec));
//
//    // define the call
//    lyric_assembler::ProcHandle *procHandle;
//    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, returnType));
//
//    TU_LOG_INFO << "declared function " << callSymbol->getSymbolUrl();
//
//    // push the function context
//    auto ctx = std::make_unique<ProcCompilerContext>(this, procHandle);
//    return pushContext(std::move(ctx));
//}
//
//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::pushClass(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    std::string identifier;
//    lyric_typing::TemplateSpec templateSpec;
//    lyric_assembler::ClassSymbol *superClass = nullptr;
//    bool isAbstract = false;
//
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    lyric_parser::DeriveType derive;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));
//
//    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
//        lyric_parser::ArchetypeNode *genericNode = nullptr;
//        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
//        TU_ASSIGN_OR_RETURN (templateSpec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
//    }
//
//    lyric_parser::ArchetypeNode *initNode = nullptr;
//    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
//        auto *child = *it;
//        lyric_schema::LyricAstId childId;
//        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
//        if (childId == lyric_schema::LyricAstId::Init) {
//            if (initNode != nullptr)
//                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//            initNode = child;
//        }
//    }
//
//    // determine the superclass type
//    lyric_common::TypeDef superClassType;
//    if (initNode != nullptr) {
//        if (initNode->numChildren() < 1)
//            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//        if (initNode->numChildren() > 1) {
//            auto *superNode = initNode->getChild(1);
//            if (!superNode->isClass(lyric_schema::kLyricAstSuperClass))
//                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//            lyric_parser::ArchetypeNode *superTypeNode;
//            TU_RETURN_IF_NOT_OK (superNode->parseAttr(lyric_parser::kLyricAstTypeOffset, superTypeNode));
//            lyric_typing::TypeSpec superTypeSpec;
//            TU_ASSIGN_OR_RETURN (superTypeSpec, m_typeSystem->parseAssignable(block, superTypeNode->getArchetypeNode()));
//            TU_ASSIGN_OR_RETURN (superClassType, m_typeSystem->resolveAssignable(block, superTypeSpec));
//        }
//    } else {
//        auto *fundamentalCache = m_state->fundamentalCache();
//        superClassType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Object);
//    }
//
//    // resolve the superclass
//    TU_ASSIGN_OR_RETURN (superClass, block->resolveClass(superClassType));
//
//    lyric_assembler::ClassSymbol *classSymbol;
//    TU_ASSIGN_OR_RETURN (classSymbol, block->declareClass(
//        identifier, superClass, internal::convert_access_type(access), templateSpec.templateParameters,
//        internal::convert_derive_type(derive), isAbstract, /* declOnly= */ true));
//
//    TU_LOG_INFO << "declared class " << classSymbol->getSymbolUrl() << " from " << superClass->getSymbolUrl();
//
//    // push the class context
//    auto ctx = std::make_unique<ClassCompilerContext>(this, classSymbol, initNode);
//    return pushContext(std::move(ctx));
//}
//
//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::pushConcept(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    std::string identifier;
//    lyric_typing::TemplateSpec templateSpec;
//    lyric_assembler::ConceptSymbol *superConcept = nullptr;
//
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    lyric_parser::DeriveType derive;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));
//
//    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
//        lyric_parser::ArchetypeNode *genericNode = nullptr;
//        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
//        TU_ASSIGN_OR_RETURN (templateSpec, m_typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
//    }
//
//    // determine the superconcept type
//    lyric_common::TypeDef superConceptType;
//    auto *fundamentalCache = m_state->fundamentalCache();
//    superConceptType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);
//
//    // resolve the superconcept
//    TU_ASSIGN_OR_RETURN (superConcept, block->resolveConcept(superConceptType));
//
//    lyric_assembler::ConceptSymbol *conceptSymbol;
//    TU_ASSIGN_OR_RETURN (conceptSymbol, block->declareConcept(
//        identifier, superConcept, internal::convert_access_type(access), templateSpec.templateParameters,
//        internal::convert_derive_type(derive), /* declOnly= */ true));
//
//    TU_LOG_INFO << "declared concept " << conceptSymbol->getSymbolUrl() << " from " << superConcept->getSymbolUrl();
//
//    // push the concept context
//    auto ctx = std::make_unique<ConceptCompilerContext>(this, conceptSymbol);
//    return pushContext(std::move(ctx));
//}
//
//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::pushEnum(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    std::string identifier;
//    lyric_assembler::EnumSymbol *superEnum = nullptr;
//    lyric_object::DeriveType derive = lyric_object::DeriveType::Sealed;
//    bool isAbstract = false;
//
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    lyric_parser::ArchetypeNode *initNode = nullptr;
//    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
//        auto *child = *it;
//        lyric_schema::LyricAstId childId;
//        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
//        if (childId == lyric_schema::LyricAstId::Init) {
//            if (initNode != nullptr)
//                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//            initNode = child;
//        }
//    }
//
//    // determine the superenum type
//    auto *fundamentalCache = m_state->fundamentalCache();
//    auto superEnumType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Category);
//
//    // resolve the superenum
//    TU_ASSIGN_OR_RETURN (superEnum, block->resolveEnum(superEnumType));
//
//    lyric_assembler::EnumSymbol *enumSymbol;
//    TU_ASSIGN_OR_RETURN (enumSymbol, block->declareEnum(
//        identifier, superEnum, internal::convert_access_type(access),
//        derive, isAbstract, /* declOnly= */ true));
//
//    TU_LOG_INFO << "declared enum " << enumSymbol->getSymbolUrl() << " from " << superEnum->getSymbolUrl();
//
//    // push the enum context
//    auto ctx = std::make_unique<EnumCompilerContext>(this, enumSymbol, initNode);
//    return pushContext(std::move(ctx));
//}
//
//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::pushInstance(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    std::string identifier;
//    lyric_assembler::InstanceSymbol *superInstance = nullptr;
//    lyric_object::DeriveType derive = lyric_object::DeriveType::Any;
//    bool isAbstract = false;
//
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    lyric_parser::ArchetypeNode *initNode = nullptr;
//    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
//        auto *child = *it;
//        lyric_schema::LyricAstId childId;
//        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
//        if (childId == lyric_schema::LyricAstId::Init) {
//            if (initNode != nullptr)
//                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//            initNode = child;
//        }
//    }
//
//    // determine the superinstance type
//    auto *fundamentalCache = m_state->fundamentalCache();
//    auto superInstanceType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Singleton);
//
//    // resolve the superinstance
//    TU_ASSIGN_OR_RETURN (superInstance, block->resolveInstance(superInstanceType));
//
//    lyric_assembler::InstanceSymbol *instanceSymbol;
//    TU_ASSIGN_OR_RETURN (instanceSymbol, block->declareInstance(
//        identifier, superInstance, internal::convert_access_type(access),
//        derive, isAbstract, /* declOnly= */ true));
//
//    TU_LOG_INFO << "declared instance " << instanceSymbol->getSymbolUrl() << " from " << superInstance->getSymbolUrl();
//
//    // push the instance context
//    auto ctx = std::make_unique<InstanceCompilerContext>(this, instanceSymbol, initNode);
//    return pushContext(std::move(ctx));
//}
//
//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::pushStruct(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    std::string identifier;
//    lyric_assembler::StructSymbol *superStruct = nullptr;
//    bool isAbstract = false;
//
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    lyric_parser::DeriveType derive;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));
//
//    lyric_parser::ArchetypeNode *initNode = nullptr;
//    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
//        auto *child = *it;
//        lyric_schema::LyricAstId childId;
//        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, childId));
//        if (childId == lyric_schema::LyricAstId::Init) {
//            if (initNode != nullptr)
//                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//            initNode = child;
//        }
//    }
//
//    // determine the superstruct type
//    lyric_common::TypeDef superStructType;
//    if (initNode != nullptr) {
//        if (initNode->numChildren() < 1)
//            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//        if (initNode->numChildren() > 1) {
//            auto *superNode = initNode->getChild(1);
//            if (!superNode->isClass(lyric_schema::kLyricAstSuperClass))
//                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError);
//            lyric_parser::ArchetypeNode *superTypeNode;
//            TU_RETURN_IF_NOT_OK (superNode->parseAttr(lyric_parser::kLyricAstTypeOffset, superTypeNode));
//            lyric_typing::TypeSpec superTypeSpec;
//            TU_ASSIGN_OR_RETURN (superTypeSpec, m_typeSystem->parseAssignable(block, superTypeNode->getArchetypeNode()));
//            TU_ASSIGN_OR_RETURN (superStructType, m_typeSystem->resolveAssignable(block, superTypeSpec));
//        }
//    } else {
//        auto *fundamentalCache = m_state->fundamentalCache();
//        superStructType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Record);
//    }
//
//    // resolve the superstruct
//    TU_ASSIGN_OR_RETURN (superStruct, block->resolveStruct(superStructType));
//
//    lyric_assembler::StructSymbol *classSymbol;
//    TU_ASSIGN_OR_RETURN (classSymbol, block->declareStruct(
//        identifier, superStruct, internal::convert_access_type(access),
//        internal::convert_derive_type(derive), isAbstract, /* declOnly= */ true));
//
//    TU_LOG_INFO << "declared struct " << classSymbol->getSymbolUrl() << " from " << superStruct->getSymbolUrl();
//
//    // push the struct context
//    auto ctx = std::make_unique<StructCompilerContext>(this, classSymbol, initNode);
//    return pushContext(std::move(ctx));
//}
//
//tempo_utils::Status
//lyric_compiler::CompilerScanDriver::pushNamespace(
//    const lyric_parser::ArchetypeNode *node,
//    lyric_assembler::BlockHandle *block)
//{
//    std::string identifier;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
//
//    lyric_parser::AccessType access;
//    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));
//
//    lyric_assembler::NamespaceSymbol *namespaceSymbol;
//    TU_ASSIGN_OR_RETURN (namespaceSymbol, block->declareNamespace(
//        identifier, internal::convert_access_type(access), /* declOnly= */ true));
//
//    // push the namespace context
//    auto ctx = std::make_unique<NamespaceCompilerContext>(this, namespaceSymbol);
//    return pushContext(std::move(ctx));
//}
