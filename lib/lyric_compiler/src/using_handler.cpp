
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/using_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::UsingHandler::UsingHandler(
    bool isSideEffect,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
}

tempo_utils::Status
lyric_compiler::UsingHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isClass(lyric_schema::kLyricAstUsingClass))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "expected Using node");

    auto numChildren = node->numChildren();
    if (numChildren == 0)
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "empty using statement");

    // determine the using location if specified
    if (node->hasAttr(lyric_parser::kLyricAstModuleLocation)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstModuleLocation, m_usingLocation));
    }

    for (auto i = 0; i < numChildren; i++) {
        auto impl = std::make_unique<UsingImpl>(m_usingLocation, block, driver);
        ctx.appendChoice(std::move(impl));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::UsingHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    if (!m_isSideEffect) {
        auto *driver = getDriver();
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::UsingImpl::UsingImpl(
    const lyric_common::ModuleLocation &usingLocation,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_usingLocation(usingLocation)
{
}

tempo_utils::Status
lyric_compiler::UsingImpl::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();

    if (!node->isClass(lyric_schema::kLyricAstSymbolRefClass))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError,
            "expected SymbolRef node");

    lyric_common::SymbolPath symbolPath;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstSymbolPath, symbolPath));
    lyric_common::SymbolUrl usingUrl(m_usingLocation, symbolPath);

    return block->useSymbol(usingUrl);
}