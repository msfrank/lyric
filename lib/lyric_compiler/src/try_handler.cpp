
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

lyric_compiler::TryHandler::TryHandler(
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
lyric_compiler::TryHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();

    auto numChildren = node->numChildren();
    if (numChildren == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "missing target for try statement");

    auto *procHandle = block->blockProc();

    // create label indicating the beginning of the try block
    lyric_assembler::JumpLabel checkStart;
    TU_ASSIGN_OR_RETURN (checkStart, m_fragment->appendLabel());

    // declare the check
    TU_ASSIGN_OR_RETURN (m_tryCatch.checkHandle, procHandle->declareCheck(checkStart));

    // exception must derive from Status
    auto StatusType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Status);

    // declare a temporary to hold the raised exception
    TU_ASSIGN_OR_RETURN (m_tryCatch.caughtRef, block->declareTemporary(StatusType, /* isVariable= */ false));

    // compile the try block
    auto expression = std::make_unique<FormChoice>(FormType::SideEffect, m_fragment, block, driver);
    ctx.appendChoice(std::move(expression));
    numChildren--;

    // there must be at least one catch
    if (numChildren == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "empty try statement");

    for (int i = 0; i < numChildren; i++) {
        auto exception = std::make_unique<Exception>();
        exception->block = std::make_unique<lyric_assembler::BlockHandle>(
            block->blockProc(), block, block->blockState());
        auto when = std::make_unique<CatchWhen>(
            &m_tryCatch, exception.get(), block, driver);
        m_tryCatch.exceptions.push_back(std::move(exception));
        ctx.appendGrouping(std::move(when));
    }

    if (node->hasAttr(lyric_parser::kLyricAstDefaultOffset)) {
        auto alternative = std::make_unique<CatchAlternative>(&m_tryCatch, m_fragment, block, driver);
        ctx.appendChoice(std::move(alternative));
    }

    if (node->hasAttr(lyric_parser::kLyricAstFinallyOffset)) {
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "finally clause unsupported for try statement");
    }

    return {};
}

tempo_utils::Status
lyric_compiler::TryHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();

    lyric_assembler::JumpLabel exitLabel;
    TU_ASSIGN_OR_RETURN (exitLabel, m_fragment->appendLabel());

    // if try catch is not a side effect then push a NoReturn result
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::CatchWhen::CatchWhen(
    TryCatch *tryCatch,
    Exception *exception,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_tryCatch(tryCatch),
      m_exception(exception)
{
    TU_ASSERT (m_tryCatch != nullptr);
    TU_ASSERT (m_exception != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchWhen::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    auto predicate = std::make_unique<CatchPredicate>(
        m_tryCatch, m_exception, block, driver);
    ctx.appendChoice(std::move(predicate));

    auto body = std::make_unique<CatchBody>(m_exception, block, driver);
    ctx.appendChoice(std::move(body));

    return {};
}

tempo_utils::Status
lyric_compiler::CatchWhen::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_ASSIGN_OR_RETURN (m_exception->exceptionExit, m_exception->fragment->unconditionalJump());
    return {};
}

lyric_compiler::CatchPredicate::CatchPredicate(
    TryCatch *tryCatch,
    Exception *exception,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_tryCatch(tryCatch),
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
            auto unpack = std::make_unique<CatchUnpackPredicate>(m_tryCatch, m_exception, block, driver);
            ctx.setChoice(std::move(unpack));
            return {};
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                "invalid catch predicate");
    }
}

lyric_compiler::CatchUnpackPredicate::CatchUnpackPredicate(
    TryCatch *tryCatch,
    Exception *exception,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_tryCatch(tryCatch),
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
    auto *checkHandle = m_tryCatch->checkHandle;

    // resolve the exception type
    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec exceptionSpec;
    TU_ASSIGN_OR_RETURN (exceptionSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
    TU_ASSIGN_OR_RETURN (m_exception->exceptionType, typeSystem->resolveAssignable(block, exceptionSpec));

    // allocate a fragment for the catch body
    TU_ASSIGN_OR_RETURN (m_exception->fragment, checkHandle->declareException(m_exception->exceptionType));

    auto unpack = std::make_unique<UnpackHandler>(m_exception->exceptionType, m_tryCatch->caughtRef,
        m_exception->fragment, m_exception->block.get(), driver);
    ctx.setGrouping(std::move(unpack));

    return {};
}

lyric_compiler::CatchBody::CatchBody(
    Exception *exception,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_exception(exception)
{
    TU_ASSERT (m_exception != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchBody::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *driver = getDriver();
    auto body = std::make_unique<FormChoice>(
        FormType::SideEffect, m_exception->fragment, m_exception->block.get(), driver);
    ctx.setChoice(std::move(body));
    return {};
}

lyric_compiler::CatchAlternative::CatchAlternative(
    TryCatch *tryCatch,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_tryCatch(tryCatch),
      m_fragment(fragment)
{
    TU_ASSERT (m_tryCatch != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::CatchAlternative::decide(
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
