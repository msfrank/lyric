
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_set.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/try_handler.h>
#include <lyric_compiler/match_utils.h>
#include <lyric_compiler/unpack_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <tempo_utils/file_utilities.h>

lyric_compiler::TryHandler::TryHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
    m_tryCatchFinally.fragment = fragment;
    TU_ASSERT (m_tryCatchFinally.fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::TryHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *procHandle = block->blockProc();
    auto *fragment = m_tryCatchFinally.fragment;

    auto numChildren = node->numChildren();
    if (numChildren < 2 || numChildren > 3)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid Try node");

    // create label indicating the beginning of the try block
    lyric_assembler::JumpLabel checkStart;
    TU_ASSIGN_OR_RETURN (checkStart, fragment->appendLabel(tempo_utils::generate_name("checkStart.XXXXXXXX")));

    // declare the check
    TU_ASSIGN_OR_RETURN (m_tryCatchFinally.checkHandle, procHandle->declareCheck(checkStart));

    // compile the try block
    auto tryBlock = std::make_unique<FormChoice>(FormType::SideEffect, fragment, block, driver);
    ctx.appendChoice(std::move(tryBlock));

    // compile the catch clause
    auto tryCatch = std::make_unique<TryCatch>(&m_tryCatchFinally, block, driver);
    ctx.appendGrouping(std::move(tryCatch));

    // compile the finally clause if present
    if (numChildren == 3) {
        auto tryFinally = std::make_unique<TryFinally>(&m_tryCatchFinally, block, driver);
        ctx.appendGrouping(std::move(tryCatch));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::TryHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *fragment = m_tryCatchFinally.fragment;
    auto *checkHandle = m_tryCatchFinally.checkHandle;
    auto afterCatch = m_tryCatchFinally.afterCatch;

    // patch check exit target
    TU_RETURN_IF_NOT_OK (fragment->patchTarget(m_tryCatchFinally.checkExit, afterCatch));

    // patch catch resume targets
    for (auto it = checkHandle->catchesBegin(); it != checkHandle->catchesEnd(); it++) {
        auto *catchHandle = *it;
        auto resumeTarget = catchHandle->getResumeTarget();
        if (resumeTarget.isValid()) {
            TU_RETURN_IF_NOT_OK (fragment->patchTarget(resumeTarget, afterCatch));
        }
    }

    // if try catch is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        auto *driver = getDriver();
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::TryCatch::TryCatch(
    TryCatchFinally *tryCatchFinally,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_tryCatchFinally(tryCatchFinally)
{
    TU_ASSERT (m_tryCatchFinally != nullptr);
}

tempo_utils::Status
lyric_compiler::TryCatch::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fragment = m_tryCatchFinally->fragment;

    if (!node->isClass(lyric_schema::kLyricAstCatchClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Catch node");

    auto numChildren = node->numChildren();

    // there must be at least one catch
    if (numChildren == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "empty catch clause");

    // jump unconditionally if there was no exception raised
    TU_ASSIGN_OR_RETURN (m_tryCatchFinally->checkExit, fragment->unconditionalJump());

    // create label indicating the end of the try block
    lyric_assembler::JumpLabel checkEnd;
    TU_ASSIGN_OR_RETURN (checkEnd, fragment->appendLabel(tempo_utils::generate_name("checkEnd.XXXXXXXX")));

    // finalize the check
    TU_RETURN_IF_NOT_OK (m_tryCatchFinally->checkHandle->finalizeCheck(checkEnd));

    // append handler for each when clause
    for (int i = 0; i < numChildren; i++) {
        auto when = std::make_unique<CatchWhen>(m_tryCatchFinally, block, driver);
        ctx.appendGrouping(std::move(when));
    }

    // append handler for else clause if present
    if (node->hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        auto alternative = std::make_unique<CatchElse>(m_tryCatchFinally, fragment, block, driver);
        ctx.appendChoice(std::move(alternative));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::TryCatch::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *fragment = m_tryCatchFinally->fragment;
    TU_ASSIGN_OR_RETURN (m_tryCatchFinally->afterCatch, fragment->appendLabel(tempo_utils::generate_name("afterCatch.XXXXXXXX")));
    return {};
}

lyric_compiler::TryFinally::TryFinally(
    TryCatchFinally *tryCatchFinally,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_tryCatchFinally(tryCatchFinally)
{
    TU_ASSERT (m_tryCatchFinally != nullptr);
}

tempo_utils::Status
lyric_compiler::TryFinally::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fragment = m_tryCatchFinally->fragment;
    auto *procHandle = block->blockProc();

    // create label indicating the beginning of the finally clause
    lyric_assembler::JumpLabel cleanupStart;
    TU_ASSIGN_OR_RETURN (cleanupStart, fragment->appendLabel(tempo_utils::generate_name("cleanupStart.XXXXXXXX")));

    // declare the cleanup
    TU_ASSIGN_OR_RETURN (m_tryCatchFinally->cleanupHandle, procHandle->declareCleanup(cleanupStart));

    // compile the finally block
    auto tryBlock = std::make_unique<FormChoice>(FormType::SideEffect, fragment, block, driver);
    ctx.appendChoice(std::move(tryBlock));

    return {};
}

tempo_utils::Status
lyric_compiler::TryFinally::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *fragment = m_tryCatchFinally->fragment;

    // create label indicating the end of the finally clause
    lyric_assembler::JumpLabel cleanupEnd;
    TU_ASSIGN_OR_RETURN (cleanupEnd, fragment->appendLabel(tempo_utils::generate_name("cleanupEnd.XXXXXXXX")));

    // finalize the cleanup
    TU_RETURN_IF_NOT_OK (m_tryCatchFinally->cleanupHandle->finalizeCleanup(cleanupEnd));

    return {};
}

lyric_compiler::CatchWhen::CatchWhen(
    TryCatchFinally *tryCatchFinally,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_tryCatchFinally(tryCatchFinally)
{
    TU_ASSERT (m_tryCatchFinally != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchWhen::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *checkHandle = m_tryCatchFinally->checkHandle;
    auto *fragment = m_tryCatchFinally->fragment;

    lyric_assembler::JumpLabel catchStart;
    TU_ASSIGN_OR_RETURN (catchStart, fragment->appendLabel(tempo_utils::generate_name("catchStart.XXXXXXXX")));

    // declare the catch
    TU_ASSIGN_OR_RETURN (m_exception.catchHandle, checkHandle->declareCatch(catchStart));

    // create a block for the when clause
    m_exception.block = std::make_unique<lyric_assembler::BlockHandle>(
        block->blockProc(), block, block->blockState());

    auto predicate = std::make_unique<CatchPredicate>(
        m_tryCatchFinally, &m_exception, block, driver);
    ctx.appendChoice(std::move(predicate));

    auto body = std::make_unique<CatchBody>(&m_exception, fragment, block, driver);
    ctx.appendChoice(std::move(body));

    return {};
}

tempo_utils::Status
lyric_compiler::CatchWhen::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *catchHandle = m_exception.catchHandle;
    auto *fragment = m_tryCatchFinally->fragment;

    auto numStatements = fragment->numStatements();
    bool unfinished = true;

    if (numStatements > 0) {
        auto lastStatement = fragment->getStatement(numStatements - 1);
        switch (lastStatement.instruction->getType()) {
            case lyric_assembler::InstructionType::Jump:
            case lyric_assembler::InstructionType::Return:
            case lyric_assembler::InstructionType::Interrupt:
            case lyric_assembler::InstructionType::Abort:
            case lyric_assembler::InstructionType::Halt:
                unfinished = false;
                break;
            default:
                break;
        }
    }

    if (unfinished) {
        lyric_assembler::JumpTarget resumeTarget;
        TU_ASSIGN_OR_RETURN (resumeTarget, fragment->unconditionalJump());
        TU_RETURN_IF_NOT_OK (catchHandle->setResumeTarget(resumeTarget));
    }

    lyric_assembler::JumpLabel catchEnd;
    TU_ASSIGN_OR_RETURN (catchEnd, fragment->appendLabel(tempo_utils::generate_name("catchEnd.XXXXXXXX")));

    TU_RETURN_IF_NOT_OK (catchHandle->finalizeCatch(catchEnd));

    return {};
}

lyric_compiler::CatchPredicate::CatchPredicate(
    TryCatchFinally *tryCatchFinally,
    Exception *exception,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_tryCatchFinally(tryCatchFinally),
      m_exception(exception)
{
    TU_ASSERT (m_exception != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchPredicate::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_VV << "decide TryPredicate@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    auto *block = getBlock();
    auto *driver = getDriver();

    switch (astId) {
        case lyric_schema::LyricAstId::Unpack: {
            auto unpack = std::make_unique<CatchUnpackPredicate>(m_tryCatchFinally, m_exception, block, driver);
            ctx.setChoice(std::move(unpack));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "invalid catch predicate");
    }
}

lyric_compiler::CatchUnpackPredicate::CatchUnpackPredicate(
    TryCatchFinally *tryCatchFinally,
    Exception *exception,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_tryCatchFinally(tryCatchFinally),
      m_exception(exception)
{
    TU_ASSERT (m_exception != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchUnpackPredicate::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *checkHandle = m_tryCatchFinally->checkHandle;
    auto *fragment = m_tryCatchFinally->fragment;

    // resolve the exception type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec exceptionSpec;
    TU_ASSIGN_OR_RETURN (exceptionSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
    lyric_common::TypeDef exceptionType;
    TU_ASSIGN_OR_RETURN (exceptionType, typeSystem->resolveAssignable(block, exceptionSpec));
    TU_RETURN_IF_NOT_OK (m_exception->catchHandle->setExceptionType(exceptionType));

    auto unpack = std::make_unique<UnpackHandler>(exceptionType, checkHandle->getCaughtReference(),
        fragment, m_exception->block.get(), driver);
    ctx.setGrouping(std::move(unpack));

    return {};
}

lyric_compiler::CatchBody::CatchBody(
    Exception *exception,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_exception(exception),
      m_fragment(fragment)
{
    TU_ASSERT (m_exception != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *driver = getDriver();
    auto body = std::make_unique<FormChoice>(
        FormType::SideEffect, m_fragment, m_exception->block.get(), driver);
    ctx.setChoice(std::move(body));
    return {};
}

lyric_compiler::CatchElse::CatchElse(
    TryCatchFinally *tryCatchFinally,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_tryCatchFinally(tryCatchFinally),
      m_fragment(fragment)
{
    TU_ASSERT (m_tryCatchFinally != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchElse::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto body = std::make_unique<FormChoice>(FormType::SideEffect, m_fragment, block, driver);
    ctx.setChoice(std::move(body));
    return {};
}
