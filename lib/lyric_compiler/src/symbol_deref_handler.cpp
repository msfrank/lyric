
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/symbol_deref_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>

lyric_compiler::SymbolDerefHandler::SymbolDerefHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
    m_deref.fragment = fragment;
    TU_ASSERT (m_deref.fragment != nullptr);
    m_deref.bindingBlock = block;
    TU_ASSERT (m_deref.bindingBlock != nullptr);
}

tempo_utils::Status
lyric_compiler::SymbolDerefHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before SymbolDerefHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    auto numChildren = node->numChildren();
    if (numChildren == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "empty deref statement");

    ctx.setSkipChildren(true);

    std::vector<std::string> path;
    for (int i = 0; i < node->numChildren(); i++) {
        auto *child = node->getChild(i);
        std::string identifier;
        TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
        path.push_back(std::move(identifier));
    }
    lyric_common::SymbolUrl symbolUrl;
    TU_ASSIGN_OR_RETURN (symbolUrl, block->resolveDefinition(path));

    auto *symbolCache = driver->getSymbolCache();
    m_deref.symbol = symbolCache->getSymbolOrNull(symbolUrl);
    if (m_deref.symbol == nullptr)
        return CompilerStatus::forCondition(CompilerCondition::kMissingSymbol,
            "missing symbol {}", symbolUrl.toString());

    return {};
}

tempo_utils::Status
lyric_compiler::SymbolDerefHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after SymbolDerefHandler@" << this;

    TU_RETURN_IF_NOT_OK (m_deref.fragment->loadDescriptor(m_deref.symbol));

    if (m_isSideEffect) {
        auto *driver = getDriver();
        auto resultType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
        if (resultType.getType() != lyric_common::TypeDefType::NoReturn) {
            TU_RETURN_IF_NOT_OK (m_deref.fragment->popValue());
        }
    }

    return {};
}