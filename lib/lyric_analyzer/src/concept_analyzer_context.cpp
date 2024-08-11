
#include <lyric_analyzer/concept_analyzer_context.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_analyzer::ConceptAnalyzerContext::ConceptAnalyzerContext(
    AnalyzerScanDriver *driver,
    lyric_assembler::ConceptSymbol *conceptSymbol)
    : m_driver(driver),
      m_conceptSymbol(conceptSymbol)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_conceptSymbol != nullptr);
}

lyric_assembler::BlockHandle *
lyric_analyzer::ConceptAnalyzerContext::getBlock() const
{
    return m_conceptSymbol->conceptBlock();
}

tempo_utils::Status
lyric_analyzer::ConceptAnalyzerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    switch (resource->getId()) {
        case lyric_schema::LyricAstId::Def:
            return declareAction(node);
        case lyric_schema::LyricAstId::Impl:
        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_analyzer::ConceptAnalyzerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    if (resource->getId() == lyric_schema::LyricAstId::DefConcept)
        return m_driver->popContext();

    return {};
}

tempo_utils::Status
lyric_analyzer::ConceptAnalyzerContext::declareAction(const lyric_parser::ArchetypeNode *node)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

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

    lyric_assembler::ActionSymbol *actionSymbol;
    TU_ASSIGN_OR_RETURN (actionSymbol, m_conceptSymbol->declareAction(identifier, lyric_object::AccessType::Public));

    auto *resolver = actionSymbol->actionResolver();

    // determine the return type
    lyric_parser::ArchetypeNode *returnTypeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, returnTypeNode));
    lyric_typing::TypeSpec returnTypeSpec;
    TU_ASSIGN_OR_RETURN (returnTypeSpec, typeSystem->parseAssignable(block, returnTypeNode->getArchetypeNode()));
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnTypeSpec));

    // determine the parameter list
    auto *packNode = node->getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(block, packNode->getArchetypeNode()));
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // define the action
    TU_RETURN_IF_NOT_OK (actionSymbol->defineAction(parameterPack, returnType));

    TU_LOG_INFO << "declared action " << actionSymbol->getSymbolUrl() << " for " << m_conceptSymbol->getSymbolUrl();

    return {};
}
