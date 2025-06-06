
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_analyzer/class_analyzer_context.h>
#include <lyric_analyzer/impl_analyzer_context.h>
#include <lyric_analyzer/internal/analyzer_utils.h>
#include <lyric_analyzer/proc_analyzer_context.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::ClassAnalyzerContext::ClassAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::ClassSymbol *classSymbol,
    const lyric_parser::ArchetypeNode *initNode)
    : m_driver(driver),
      m_classSymbol(classSymbol),
      m_initNode(initNode)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_classSymbol != nullptr);
}

lyric_assembler::BlockHandle *
lyric_analyzer::ClassAnalyzerContext::getBlock() const
{
    return m_classSymbol->classBlock();
}

tempo_utils::Status
lyric_analyzer::ClassAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    switch (resource->getId()) {
        case lyric_schema::LyricAstId::Val:
            return declareMember(node, /* isVariable= */ false);
        case lyric_schema::LyricAstId::Var:
            return declareMember(node, /* isVariable= */ true);
        case lyric_schema::LyricAstId::Def:
            return declareMethod(node);
        case lyric_schema::LyricAstId::Impl:
            return declareImpl(node);
        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_analyzer::ClassAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    if (resource->getId() == lyric_schema::LyricAstId::DefClass) {
        // define the constructor
        if (m_initNode != nullptr) {
            auto *block = getBlock();
            auto *typeSystem = m_driver->getTypeSystem();
            auto *packNode = m_initNode->getChild(0);
            lyric_typing::PackSpec packSpec;
            TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, packNode->getArchetypeNode()));
            lyric_assembler::ParameterPack parameterPack;
            TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(block, packSpec));
            lyric_assembler::CallSymbol *ctorSymbol;
            TU_ASSIGN_OR_RETURN (ctorSymbol, m_classSymbol->declareCtor(lyric_object::AccessType::Public));
            TU_RETURN_IF_STATUS (ctorSymbol->defineCall(parameterPack, lyric_common::TypeDef::noReturn()));
        } else {
            lyric_assembler::CallSymbol *ctorSymbol;
            TU_ASSIGN_OR_RETURN (ctorSymbol, m_classSymbol->declareCtor(lyric_object::AccessType::Public));
            TU_RETURN_IF_STATUS (ctorSymbol->defineCall({}, lyric_common::TypeDef::noReturn()));
        }
        return m_driver->popContext();
    }

    return {};
}

tempo_utils::Status
lyric_analyzer::ClassAnalyzerContext::declareMember(const lyric_parser::ArchetypeNode *node, bool isVariable)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    auto walker = node->getArchetypeNode();
    auto *block = getBlock();
    auto *typeSystem = m_driver->getTypeSystem();

    lyric_parser::NodeWalker typeNode;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec memberSpec;
    TU_ASSIGN_OR_RETURN (memberSpec, typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef memberType;
    TU_ASSIGN_OR_RETURN (memberType, typeSystem->resolveAssignable(block, memberSpec));

    TU_RETURN_IF_STATUS(m_classSymbol->declareMember(
        identifier, memberType, isVariable, internal::convert_access_type(access)));

    TU_LOG_INFO << "declared member " << identifier << " on " << m_classSymbol->getSymbolUrl();

    return {};
}

tempo_utils::Status
lyric_analyzer::ClassAnalyzerContext::declareMethod(const lyric_parser::ArchetypeNode *node)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

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

    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, m_classSymbol->declareMethod(
        identifier, internal::convert_access_type(access)));

    auto *resolver = callSymbol->callResolver();

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
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // define the method
    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, returnType));

    TU_LOG_INFO << "declared method " << callSymbol->getSymbolUrl() << " for " << m_classSymbol->getSymbolUrl();

    // push the proc context
    auto ctx = std::make_unique<ProcAnalyzerContext>(m_driver, procHandle);
    return m_driver->pushContext(std::move(ctx));
}

tempo_utils::Status
lyric_analyzer::ClassAnalyzerContext::declareImpl(const lyric_parser::ArchetypeNode *node)
{
    auto walker = node->getArchetypeNode();
    auto *block = getBlock();
    auto *typeSystem = m_driver->getTypeSystem();

    lyric_parser::NodeWalker typeNode;
    TU_RETURN_IF_NOT_OK (walker.parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec implSpec;
    TU_ASSIGN_OR_RETURN (implSpec, typeSystem->parseAssignable(block, typeNode));
    lyric_common::TypeDef implType;
    TU_ASSIGN_OR_RETURN (implType, typeSystem->resolveAssignable(block, implSpec));

    lyric_assembler::ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, m_classSymbol->declareImpl(implType));

    TU_LOG_INFO << "declared impl " << implType << " on " << m_classSymbol->getSymbolUrl();

    // push the impl context
    auto ctx = std::make_unique<ImplAnalyzerContext>(m_driver, implHandle);
    return m_driver->pushContext(std::move(ctx));
}
