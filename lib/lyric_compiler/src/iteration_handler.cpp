
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_compiler/block_node_handler.h>
#include <lyric_compiler/iteration_handler.h>
#include "lyric_compiler/compiler_result.h"

lyric_compiler::WhileHandler::WhileHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::WhileHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    TU_ASSIGN_OR_RETURN (m_iteration.beginIterationLabel, m_fragment->appendLabel());

    auto predicate = std::make_unique<FormChoice>(
        FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(predicate));

    auto whileBody = std::make_unique<WhileBody>(&m_iteration, m_fragment, block, driver);
    ctx.appendChoice(std::move(whileBody));

    return {};
}

tempo_utils::Status
lyric_compiler::WhileHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();

    lyric_assembler::JumpTarget nextIterationTarget;
    TU_ASSIGN_OR_RETURN (nextIterationTarget, m_fragment->unconditionalJump());
    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(nextIterationTarget, m_iteration.beginIterationLabel));

    lyric_assembler::JumpLabel loopFinishedLabel;
    TU_ASSIGN_OR_RETURN (loopFinishedLabel, m_fragment->appendLabel());

    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(m_iteration.exitLoopTarget, loopFinishedLabel));

    // if handler is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::WhileBody::WhileBody(
    Iteration *iteration,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_iteration(iteration),
      m_fragment(fragment)
{
    TU_ASSERT (m_iteration != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::WhileBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();
    auto *typeSystem = driver->getTypeSystem();

    auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);

    auto testType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(BoolType, testType));
    if (!isAssignable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "expected test expression to return {}; found {}", BoolType.toString(), testType.toString());

    TU_ASSIGN_OR_RETURN (m_iteration->exitLoopTarget, m_fragment->jumpIfFalse());

    auto whileBlock = std::make_unique<lyric_assembler::BlockHandle>(
        block->blockProc(), block, block->blockState());
    auto whileBody = std::make_unique<BlockNodeHandler>(
        std::move(whileBlock), /* requiresResult= */ false, /* isSideEffect= */ true, m_fragment, driver);
    ctx.setGrouping(std::move(whileBody));

    return {};
}