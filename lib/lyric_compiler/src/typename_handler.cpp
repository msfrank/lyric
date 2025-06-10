
#include <lyric_assembler/typename_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/typename_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::TypenameHandler::TypenameHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(nullptr)
{
}

lyric_compiler::TypenameHandler::TypenameHandler(
    bool isSideEffect,
    lyric_assembler::NamespaceSymbol *currentNamespace,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_isSideEffect(isSideEffect),
      m_currentNamespace(currentNamespace)
{
    TU_ASSERT (m_currentNamespace != nullptr);
}

tempo_utils::Status
lyric_compiler::TypenameHandler::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    TU_LOG_INFO << "decide TypenameHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isClass(lyric_schema::kLyricAstTypeNameClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected TypeName node");

    // get global name
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // get global access level
    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // declare static symbol
    lyric_assembler::TypenameSymbol *typenameSymbol;
    TU_ASSIGN_OR_RETURN (typenameSymbol, block->declareTypename(identifier, convert_access_type(access)));

    // add global to the current namespace if specified
    if (m_currentNamespace != nullptr) {
        TU_RETURN_IF_NOT_OK (m_currentNamespace->putTarget(typenameSymbol->getSymbolUrl()));
    }

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}