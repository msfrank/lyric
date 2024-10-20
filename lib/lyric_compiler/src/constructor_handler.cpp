
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constructor_handler.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::ConstructorHandler::ConstructorHandler(
    std::unique_ptr<lyric_assembler::ConstructableInvoker> &&superInvoker,
    lyric_assembler::CallSymbol *callSymbol,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver)
{
    m_constructor.superInvoker = std::move(superInvoker);
    TU_ASSERT (m_constructor.superInvoker != nullptr);
    m_constructor.callSymbol = callSymbol;
    TU_ASSERT (m_constructor.callSymbol != nullptr);
    m_constructor.procHandle = callSymbol->callProc();
    TU_ASSERT (m_constructor.procHandle != nullptr);
}

tempo_utils::Status
lyric_compiler::ConstructorHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_INFO << "before ConstructorHandler@" << this;

    auto *classBlock = getBlock();
    auto *driver = getDriver();
    auto *ctorBlock = m_constructor.procHandle->procBlock();
    auto *procBuilder = m_constructor.procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    auto pack = std::make_unique<PackHandler>(m_constructor.callSymbol, classBlock, driver);
    ctx.appendGrouping(std::move(pack));

    auto super = std::make_unique<InitSuper>(&m_constructor, fragment, ctorBlock, driver);
    ctx.appendGrouping(std::move(super));

    auto proc = std::make_unique<ProcHandler>(
        m_constructor.procHandle, /* requiresResult= */ false, ctorBlock, getDriver());
    ctx.appendGrouping(std::move(proc));

    return {};
}

tempo_utils::Status
lyric_compiler::ConstructorHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_INFO << "after ConstructorHandler@" << this;

    auto *procBuilder = m_constructor.procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // finalize the call
    TU_RETURN_IF_NOT_OK (m_constructor.callSymbol->finalizeCall());

    return {};
}

lyric_compiler::InitSuper::InitSuper(
    Constructor *constructor,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(block, block, fragment, driver),
      m_constructor(constructor)
{
    TU_ASSERT (m_constructor != nullptr);
}

tempo_utils::Status
lyric_compiler::InitSuper::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *fragment = getFragment();

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(*m_constructor->superInvoker));

    // load the uninitialized receiver onto the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->loadThis());

    return BaseInvokableHandler::before(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::InitSuper::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *fragment = getFragment();

    lyric_typing::CallsiteReifier reifier(typeSystem);

    auto *invoker = m_constructor->superInvoker.get();
    TU_RETURN_IF_NOT_OK (reifier.initialize(*invoker));

    auto *constructable = invoker->getConstructable();
    TU_RETURN_IF_NOT_OK (placeArguments(constructable, reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, invoker->invoke(getInvokeBlock(), reifier, fragment, /* flags= */ 0));

    return driver->pushResult(returnType);
}