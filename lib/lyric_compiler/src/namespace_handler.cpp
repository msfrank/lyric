
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/defclass_handler.h>
#include <lyric_compiler/defconcept_handler.h>
#include <lyric_compiler/defstatic_handler.h>
#include <lyric_compiler/defstruct_handler.h>
#include <lyric_compiler/def_handler.h>
#include <lyric_compiler/namespace_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::NamespaceHandler::NamespaceHandler(
    lyric_assembler::NamespaceSymbol *parentNamespace,
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_parentNamespace(parentNamespace),
      m_isSideEffect(isSideEffect),
      m_namespace(nullptr)
{
    TU_ASSERT (m_parentNamespace != nullptr);
}

tempo_utils::Status
lyric_compiler::NamespaceHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_INFO << "before NamespaceHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isClass(lyric_schema::kLyricAstNamespaceClass))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "expected Namespace node");

    // get namespace identifer
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get namespace access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // declare the namespace
    TU_ASSIGN_OR_RETURN (m_namespace, m_parentNamespace->declareSubspace(identifier, convert_access_type(access)));

    // handle the namespace definitions
    for (int i = 0; i < node->numChildren(); i++) {
        auto definition = std::make_unique<NamespaceDefinition>(m_namespace, block, driver);
        ctx.appendChoice(std::move(definition));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::NamespaceHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_INFO << "after NamespaceHandler@" << this;

    if (!m_isSideEffect) {
        auto *driver = getDriver();
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::NamespaceDefinition::NamespaceDefinition(
    lyric_assembler::NamespaceSymbol *namespaceSymbol,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_namespaceSymbol(namespaceSymbol)
{
    TU_ASSERT (m_namespaceSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::NamespaceDefinition::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *driver = getDriver();
    auto *namespaceBlock = m_namespaceSymbol->namespaceBlock();

    lyric_schema::LyricAstId astId;
    TU_RETURN_IF_NOT_OK (node->parseId(lyric_schema::kLyricAstVocabulary, astId));
    switch (astId) {
        case lyric_schema::LyricAstId::Def: {
            auto handler = std::make_unique<DefHandler>(
                /* isSideEffect= */ true, namespaceBlock, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::DefClass: {
            auto handler = std::make_unique<DefClassHandler>(
                /* isSideEffect= */ true, namespaceBlock, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::DefConcept: {
            auto handler = std::make_unique<DefConceptHandler>(
                /* isSideEffect= */ true, namespaceBlock, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::DefStatic: {
            auto handler = std::make_unique<DefStaticHandler>(
                /* isSideEffect= */ true, namespaceBlock, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        case lyric_schema::LyricAstId::DefStruct: {
            auto handler = std::make_unique<DefStructHandler>(
                /* isSideEffect= */ true, namespaceBlock, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }
        default:
            return namespaceBlock->logAndContinue(CompilerCondition::kSyntaxError,
                tempo_tracing::LogSeverity::kError,
                "unexpected AST node");
    }
}