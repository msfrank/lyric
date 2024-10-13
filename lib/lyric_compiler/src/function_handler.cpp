
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/function_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::FunctionHandler::FunctionHandler(
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    std::unique_ptr<lyric_assembler::CallableInvoker> &&invoker,
    std::unique_ptr<lyric_typing::CallsiteReifier> &&reifier,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(bindingBlock, invokeBlock, fragment, driver),
      m_invoker(std::move(invoker)),
      m_reifier(std::move(reifier))
{
    TU_ASSERT (m_invoker != nullptr);
    TU_ASSERT (m_reifier != nullptr);
}

tempo_utils::Status
lyric_compiler::FunctionHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *fragment = getFragment();

    TU_RETURN_IF_NOT_OK (placeArguments(m_invoker->getCallable(), *m_reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_invoker->invoke(getInvokeBlock(), *m_reifier, fragment));
    return driver->pushResult(returnType);
}