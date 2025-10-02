
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/method_handler.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::MethodHandler::MethodHandler(
    Method method,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_method(method)
{
    TU_ASSERT (m_method.callSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::MethodHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before MethodHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    auto pack = std::make_unique<PackHandler>(m_method.callSymbol, block, driver);
    ctx.appendGrouping(std::move(pack));

    auto returnType = m_method.callSymbol->getReturnType();
    TU_ASSERT (returnType.isValid());
    bool requiresResult = returnType != lyric_common::TypeDef::noReturn();

    auto proc = std::make_unique<ProcHandler>(
        m_method.procHandle, requiresResult, getBlock(), getDriver());
    ctx.appendGrouping(std::move(proc));

    return {};
}

tempo_utils::Status
lyric_compiler::MethodHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after MethodHandler@" << this;

    auto *fragment = m_method.procHandle->procFragment();

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // finalize the call
    TU_RETURN_IF_STATUS (m_method.callSymbol->finalizeCall());

    return {};
}

lyric_compiler::AbstractMethodHandler::AbstractMethodHandler(
    Method method,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_method(method)
{
    TU_ASSERT (m_method.callSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::AbstractMethodHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before AbstractMethodHandler@" << this;
    ctx.setSkipChildren(true);
    return {};
}

tempo_utils::Status
lyric_compiler::AbstractMethodHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after AbstractMethodHandler@" << this;
    return {};
}
