
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/impl_utils.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_typing/impl_selector.h>
#include <lyric_typing/summon_reifier.h>

tempo_utils::Status
lyric_compiler::compile_unary_operator(
    lyric_schema::LyricAstId operationId,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();

    lyric_common::TypeDef operandType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_assembler::ActionSymbol *actionSymbol;
    TU_ASSIGN_OR_RETURN (actionSymbol, resolve_operator_action(operationId, fundamentalCache, symbolCache));

    lyric_typing::SummonReifier summoner(state);

    TU_RETURN_IF_NOT_OK (summoner.initialize(actionSymbol));
    TU_RETURN_IF_NOT_OK (summoner.reifyNextArgument(operandType));
    TU_RETURN_IF_NOT_OK (summoner.finalize());

    lyric_typing::ImplSelector selector(&summoner, block);

    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RETURN_IF_NOT_OK (selector.select(callable));

    lyric_typing::CallsiteReifier reifier(state);
    TU_RETURN_IF_NOT_OK (reifier.initialize(callable.get(), selector.getCallsiteArguments()));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(operandType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, callable->invoke(block, reifier, fragment));

    return driver->pushResult(resultType);
}

tempo_utils::Status
lyric_compiler::compile_binary_operator(
    lyric_schema::LyricAstId operationId,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (driver != nullptr);

    auto *state = block->blockState();
    auto *fundamentalCache = state->fundamentalCache();
    auto *symbolCache = state->symbolCache();

    lyric_common::TypeDef rhsType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_common::TypeDef lhsType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    lyric_assembler::ActionSymbol *actionSymbol;
    TU_ASSIGN_OR_RETURN (actionSymbol, resolve_operator_action(operationId, fundamentalCache, symbolCache));

    lyric_typing::SummonReifier summoner(state);

    TU_RETURN_IF_NOT_OK (summoner.initialize(actionSymbol));
    TU_RETURN_IF_NOT_OK (summoner.reifyNextArgument(lhsType));
    TU_RETURN_IF_NOT_OK (summoner.reifyNextArgument(rhsType));
    TU_RETURN_IF_NOT_OK (summoner.finalize());

    lyric_typing::ImplSelector selector(&summoner, block);

    std::unique_ptr<lyric_assembler::AbstractCallable> callable;
    TU_RETURN_IF_NOT_OK (selector.select(callable));

    lyric_typing::CallsiteReifier reifier(state);
    TU_RETURN_IF_NOT_OK (reifier.initialize(callable.get(), selector.getCallsiteArguments()));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(lhsType));
    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(rhsType));

    lyric_common::TypeDef resultType;
    TU_ASSIGN_OR_RETURN (resultType, callable->invoke(block, reifier, fragment));

    return driver->pushResult(resultType);
}
