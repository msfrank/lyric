
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

#include "lyric_assembler/symbol_cache.h"

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
    BeforeContext &ctx)
{
    auto *block = m_procHandle->procBlock();
    auto *driver = getDriver();
    auto *fragment = m_procHandle->procFragment();

    auto numChildren = node->numChildren();

    if (numChildren == 0) {
        if (!m_requiresResult)
            return {};
        return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
            "block requires a result");
    }

    //
    auto *activationCall = m_procHandle->getActivationCall();
    auto *restParameter = activationCall->restPlacement();
    if (restParameter != nullptr && !restParameter->name.empty()) {
        lyric_assembler::DataReference restVar;
        TU_ASSIGN_OR_RETURN (restVar, block->resolveReference(restParameter->name));
        TU_RETURN_IF_NOT_OK (fragment->loadRest());
        TU_RETURN_IF_NOT_OK (fragment->storeRef(restVar, /* initialStore */ true));
    }

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
    AfterContext &ctx)
{
    return {};
}