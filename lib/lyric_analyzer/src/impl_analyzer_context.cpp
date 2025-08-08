
#include <lyric_analyzer/impl_analyzer_context.h>
#include <lyric_analyzer/internal/analyzer_utils.h>
#include <lyric_analyzer/proc_analyzer_context.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::ImplAnalyzerContext::ImplAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::ImplHandle *implHandle)
    : m_driver(driver),
      m_implHandle(implHandle)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_implHandle != nullptr);
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
