
#include <lyric_compiler/block_node_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::BlockNodeHandler::BlockNodeHandler(
    std::unique_ptr<lyric_assembler::BlockHandle> &&block,
    bool requiresResult,
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseGrouping(block.get(), driver),
      m_block(std::move(block)),
      m_requiresResult(requiresResult),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_block != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::BlockNodeHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    auto *driver = getDriver();

    auto numChildren = node->numChildren();
    TU_ASSERT (numChildren > 0);

    // append each intermediate form
    if (numChildren > 1) {
        for (int i = 0; i < numChildren - 1; i++) {
            auto intermediate = std::make_unique<FormChoice>(
                FormType::SideEffect, m_fragment, m_block.get(), driver);
            ctx.appendChoice(std::move(intermediate));
        }
    }

    // append the last form in the block
    FormType formType;
    if (m_requiresResult) {
        formType = FormType::Expression;
    } else {
        formType = m_isSideEffect? FormType::SideEffect : FormType::Any;
    }
    auto last = std::make_unique<FormChoice>(
        formType, m_fragment, m_block.get(), driver);
    ctx.appendChoice(std::move(last));

    return {};
}

tempo_utils::Status
lyric_compiler::BlockNodeHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    return {};
}