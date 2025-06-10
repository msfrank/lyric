
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
    TU_LOG_INFO << "before SymbolDerefHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    auto numChildren = node->numChildren();
    if (numChildren == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "empty deref statement");

    auto initial = std::make_unique<SymbolDerefInitial>(&m_deref, block, driver);
    ctx.appendChoice(std::move(initial));

    for (int i = 0; i < node->numChildren() - 1; i++) {
        auto next = std::make_unique<SymbolDerefNext>(&m_deref, block, driver);
        ctx.appendChoice(std::move(next));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::SymbolDerefHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_INFO << "after SymbolDerefHandler@" << this;

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

lyric_compiler::SymbolDerefInitial::SymbolDerefInitial(
    lyric_compiler::SymbolDeref *deref,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

tempo_utils::Status
lyric_compiler::SymbolDerefInitial::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *driver = getDriver();
    auto *fragment = m_deref->fragment;

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Name:
            return deref_descriptor(node, &m_deref->bindingBlock, fragment, driver);
        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid deref target node");
    }
}

lyric_compiler::SymbolDerefNext::SymbolDerefNext(
    lyric_compiler::SymbolDeref *deref,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_deref(deref)
{
    TU_ASSERT (m_deref != nullptr);
}

tempo_utils::Status
lyric_compiler::SymbolDerefNext::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
    auto astId = resource->getId();

    auto *driver = getDriver();
    auto *fragment = m_deref->fragment;

    auto receiverType = driver->peekResult();
    TU_ASSERT (receiverType.isValid());
    TU_RETURN_IF_NOT_OK (driver->popResult());


    switch (astId) {
        case lyric_schema::LyricAstId::Name:
            TU_RETURN_IF_NOT_OK (fragment->popValue());
            return deref_descriptor(node, &m_deref->bindingBlock, fragment, driver);
        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid deref target node");
    }

}