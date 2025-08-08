
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/defconcept_handler.h>
#include <lyric_compiler/defconcept_utils.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::DefConceptHandler::DefConceptHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::DefConceptHandler::DefConceptHandler(
    bool isSideEffect,
    lyric_assembler::NamespaceSymbol *currentNamespace,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(currentNamespace)
{
    TU_ASSERT (m_currentNamespace != nullptr);
}

tempo_utils::Status
lyric_compiler::DefConceptHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    TU_LOG_VV << "before DefConceptHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeSystem = driver->getTypeSystem();

    if (!node->isClass(lyric_schema::kLyricAstDefConceptClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected DefConcept node");

    // get concept name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get concept access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // get concept derive type
    lyric_parser::DeriveType derive;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstDeriveType, derive));

    // if concept is generic, then compile the template parameter list
    if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        lyric_parser::ArchetypeNode *genericNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
        TU_ASSIGN_OR_RETURN (m_defconcept.templateSpec, typeSystem->parseTemplate(block, genericNode->getArchetypeNode()));
    }

    std::vector<lyric_parser::ArchetypeNode *> declNodes;
    std::vector<lyric_parser::ArchetypeNode *> implNodes;

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::Decl: {
                declNodes.push_back(child);
                break;
            }
            case lyric_schema::LyricAstId::Impl: {
                implNodes.push_back(child);
                break;
            }
            default:
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "unexpected AST node");
        }

        auto definition = std::make_unique<ConceptDefinition>(&m_defconcept, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    // resolve the super concept type if specified, otherwise derive from Idea
    auto superConceptType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Idea);

    // resolve the super class symbol
    TU_ASSIGN_OR_RETURN (m_defconcept.superconceptSymbol, block->resolveConcept(superConceptType));

    // declare the class
    TU_ASSIGN_OR_RETURN (m_defconcept.conceptSymbol, block->declareConcept(
        identifier, m_defconcept.superconceptSymbol, lyric_compiler::convert_access_type(access),
        m_defconcept.templateSpec.templateParameters, lyric_compiler::convert_derive_type(derive)));

    // add concept to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(m_defconcept.conceptSymbol->getSymbolUrl()));
    }

    // declare actions
    for (auto &declNode : declNodes) {
        Action action;
        TU_ASSIGN_OR_RETURN (action, declare_concept_action(
            declNode, m_defconcept.conceptSymbol, typeSystem));
        m_defconcept.actions[declNode] = action;
    }

    // declare impls
    for (auto &implNode : implNodes) {
        Impl impl;
        TU_ASSIGN_OR_RETURN (impl, declare_concept_impl(
            implNode, m_defconcept.conceptSymbol, typeSystem));
        m_defconcept.impls[implNode] = impl;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DefConceptHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_VV << "after DefConceptHandler@" << this;

    if (!m_isSideEffect) {
        auto *driver = getDriver();
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::ConceptDefinition::ConceptDefinition(
    DefConcept *defconcept,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_defconcept(defconcept)
{
    TU_ASSERT (m_defconcept != nullptr);
}

tempo_utils::Status
lyric_compiler::ConceptDefinition::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, astId));
    switch (astId) {
        case lyric_schema::LyricAstId::Decl: {
            auto action = m_defconcept->actions.at(node);
            auto handler = std::make_unique<ActionHandler>(action, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::Impl: {
            auto impl = m_defconcept->impls.at(node);
            auto handler = std::make_unique<ImplHandler>(impl, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "unexpected AST node");
    }
}