
#include <lyric_compiler/block_node_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::ProcHandler::ProcHandler(
    lyric_assembler::ProcHandle *procHandle,
    bool requiresResult,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_procHandle(procHandle),
      m_requiresResult(requiresResult)
{
    TU_ASSERT (m_procHandle != nullptr);

}

tempo_utils::Status
lyric_compiler::ProcHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    auto *block = m_procHandle->procBlock();
    auto *driver = getDriver();
    auto *codeBuilder = m_procHandle->procCode();
    auto *fragment = codeBuilder->rootFragment();

    auto numChildren = node->numChildren();
    TU_ASSERT (numChildren > 0);

    if (numChildren > 1) {
        for (int i = 0; i < numChildren - 1; i++) {
            auto sideeffect = std::make_unique<FormChoice>(
                FormType::Any, fragment, block, driver);
            ctx.appendChoice(std::move(sideeffect));
        }
    }
    if (m_requiresResult) {
        auto expression = std::make_unique<FormChoice>(
            FormType::Expression, fragment, block, driver);
        ctx.appendChoice(std::move(expression));
    } else {
        auto any = std::make_unique<FormChoice>(
            FormType::Any, fragment, block, driver);
        ctx.appendChoice(std::move(any));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::ProcHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    return {};
}