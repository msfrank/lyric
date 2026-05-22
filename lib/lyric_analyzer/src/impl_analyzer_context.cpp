
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_analyzer/impl_analyzer_context.h>
#include <lyric_analyzer/internal/analyzer_utils.h>
#include <lyric_analyzer/proc_analyzer_context.h>
#include <lyric_assembler/binding_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/impl_reifier.h>

lyric_analyzer::ImplAnalyzerContext::ImplAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::ImplHandle *implHandle,
    const lyric_common::TypeDef &implType)
    : m_driver(driver),
      m_implHandle(implHandle),
      m_implType(implType)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_implHandle != nullptr);
    TU_ASSERT (m_implType.isValid());
}

lyric_assembler::BlockHandle *
lyric_analyzer::ImplAnalyzerContext::getBlock() const
{
    return m_implHandle->implBlock();
}

tempo_utils::Status
lyric_analyzer::ImplAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Alias: {
            // alias statements were already handled in processAliases
            break;
        }
        case lyric_schema::LyricAstId::Def:
            return declareExtension(node);
        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_analyzer::ImplAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Impl:
            m_driver->popContext();
            break;
        default:
            break;
    }
    return {};
}

tempo_utils::Status
lyric_analyzer::ImplAnalyzerContext::declareExtension(const lyric_parser::ArchetypeNode *node)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    lyric_parser::ArchetypeNode *genericNode = nullptr;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
    }

    auto *block = getBlock();
    auto *typeSystem = m_driver->getTypeSystem();

    lyric_typing::TemplateSpec spec;
    if (genericNode != nullptr) {
        TU_ASSIGN_OR_RETURN (spec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    auto *resolver = getBlock();

    // determine the return type
    lyric_common::TypeDef returnType;
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *returnTypeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, returnTypeNode));
        lyric_typing::TypeSpec returnTypeSpec;
        TU_ASSIGN_OR_RETURN (returnTypeSpec, typeSystem->parseAssignable(block, returnTypeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnTypeSpec));
    }

    // determine the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, packNode->getArchetypeNode()));
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(getBlock(), packSpec));


    // define the method
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, m_implHandle->defineExtension(identifier, parameterPack, returnType));

    TU_LOG_V << "declared extension " << procHandle->getActivationUrl();

    // push the function context
    auto ctx = std::make_unique<ProcAnalyzerContext>(m_driver, procHandle);
    return m_driver->pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::ImplAnalyzerContext::processAliases(const lyric_parser::ArchetypeNode *node)
{
    if (m_finalized)
        return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant,
            "impl analyzer context is already finalized");

    auto *typeSystem = m_driver->getTypeSystem();

    lyric_typing::ImplReifier reifier(typeSystem);

    // reify the impl type
    TU_RETURN_IF_NOT_OK (reifier.initialize(m_implType));

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        if (astId != lyric_schema::LyricAstId::Alias)
            continue;
        lyric_assembler::BindingSymbol *bindingSymbol;
        TU_ASSIGN_OR_RETURN (bindingSymbol, declareAlias(child));
        TU_RETURN_IF_NOT_OK (reifier.reifyAliasArgument(bindingSymbol));
    }

    // after all aliases have been declared we can build the contract
    lyric_assembler::TypeContract contract;
    TU_ASSIGN_OR_RETURN (contract, reifier.reifyContract());

    // validate the implementation type is a subtype of the impl concept
    TU_RETURN_IF_NOT_OK (typeSystem->validateSubtype(contract.getImplementationType(), reifier.getConcept()));

    TU_RETURN_IF_NOT_OK (m_implHandle->finalizeContract(contract));
    m_finalized = true;
    return {};
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_analyzer::ImplAnalyzerContext::declareAlias(const lyric_parser::ArchetypeNode *node)
{
    TU_NOTNULL (node);

    // get binding name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get binding access level
    bool isHidden;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

    auto *block = getBlock();
    auto *typeSystem = m_driver->getTypeSystem();

    // if alias specifies a concept template parameter, then get the companion type
    lyric_common::TypeDef companionType;
    if (node->hasAttr(lyric_parser::kLyricAstSymbolPath)) {
        lyric_common::SymbolPath symbolPath;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
        std::string literalValue;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
        TU_ASSIGN_OR_RETURN (companionType, internal::resolve_companion(symbolPath, literalValue, block));
    }

    // if binding is generic, then compile the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    if (companionType.isValid() && !templateSpec.templateParameters.empty())
        return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant,
            "invalid companion alias; a companion alias cannot declare template parameters");

    // parse the target spec
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec targetSpec;
    TU_ASSIGN_OR_RETURN (targetSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));

    // declare binding symbol in the current block only
    lyric_assembler::BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RETURN (bindingSymbol, block->declareBinding(identifier, isHidden, templateSpec.templateParameters));

    auto *resolver = bindingSymbol->bindingResolver();

    // define the target type
    lyric_common::TypeDef targetType;
    TU_ASSIGN_OR_RETURN (targetType, typeSystem->resolveAssignable(resolver, targetSpec));

    // set the binding target
    TU_RETURN_IF_NOT_OK (bindingSymbol->finalizeBinding(targetType, companionType));

    return bindingSymbol;
}
